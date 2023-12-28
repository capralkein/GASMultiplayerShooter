// Created by Capralkein


#include "MPShooterGameplayAbility_RangedWeapon.h"

#include "AbilitySystemComponent.h"
#include "MPShooter/Inventory/RangedWeapon.h"

namespace MPShooterConsoleVariables
{
	static float DrawBulletTracesDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletTraceDuraton(
		TEXT("MPShooter.Weapon.DrawBulletTraceDuration"),
		DrawBulletTracesDuration,
		TEXT("Should we do debug drawing for bullet traces (if above zero, sets how long (in seconds))"),
		ECVF_Default);

	static float DrawBulletHitDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHits(
		TEXT("MPShooter.Weapon.DrawBulletHitDuration"),
		DrawBulletHitDuration,
		TEXT("Should we do debug drawing for bullet impacts (if above zero, sets how long (in seconds))"),
		ECVF_Default);

	static float DrawBulletHitRadius = 3.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHitRadius(
		TEXT("MPShooter.Weapon.DrawBulletHitRadius"),
		DrawBulletHitRadius,
		TEXT("When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in uu)"),
		ECVF_Default);
}

FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
{
	if (ConeHalfAngleRad > 0.f)
	{
		const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);

		// consider the cone a concatenation of two rotations. one "away" from the center line, and another "around" the circle
		// apply the exponent to the away-from-center rotation. a larger exponent will cluster points more tightly around the center
		const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
		const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
		const float AngleAround = FMath::FRand() * 360.0f;

		FRotator Rot = Dir.Rotation();
		FQuat DirQuat(Rot);
		FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
		FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
		FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
		FinalDirectionQuat.Normalize();

		return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
	}
	else
	{
		return Dir.GetSafeNormal();
	}
}

UMPShooterGameplayAbility_RangedWeapon::UMPShooterGameplayAbility_RangedWeapon(
	const FObjectInitializer& ObjectInitializer)
{
}

ARangedWeapon* UMPShooterGameplayAbility_RangedWeapon::GetAssociatedRangedWeapon() const
{
	return Cast<ARangedWeapon>(GetAssociatedInventoryItem());
}

void UMPShooterGameplayAbility_RangedWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	OnTargetDataReadyCallbackDelegateHandle = ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMPShooterGameplayAbility_RangedWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
		check(ASC);

		ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

int32 UMPShooterGameplayAbility_RangedWeapon::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
	{
		const FHitResult& CurHitResult = HitResults[Idx];
		if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
		{
			return Idx;
		}

		const AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
		if (HitActor && HitActor->GetAttachParentActor() && Cast<APawn>(HitActor->GetAttachParentActor()))
		{
			return Idx;
		}
	}

	return INDEX_NONE;
}

void UMPShooterGameplayAbility_RangedWeapon::PerformLocalTargeting(TArray<FHitResult>& OutHits) const
{
	const APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());

	ARangedWeapon* Weapon = GetAssociatedRangedWeapon();
	if (AvatarPawn && AvatarPawn->IsLocallyControlled() && Weapon)
	{
		FRangedWeaponFiringInput InputData;
		InputData.Weapon = Weapon;
		InputData.bCanPlayBulletFX = (AvatarPawn->GetNetMode() != NM_DedicatedServer);

		const FTransform TargetTransform = GetTargetingTransform(AvatarPawn);
		InputData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
		InputData.StartTrace = TargetTransform.GetTranslation();

		InputData.EndAim = InputData.StartTrace + InputData.AimDir * Weapon->GetMaxDamageRange();

#if ENABLE_DRAW_DEBUG
		if (MPShooterConsoleVariables::DrawBulletTracesDuration > 0.0f)
		{
			static float DebugThickness = 2.0f;
			DrawDebugLine(GetWorld(), InputData.StartTrace, InputData.StartTrace + (InputData.AimDir * 100.0f), FColor::Yellow, false, MPShooterConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
		}
#endif

		TraceBulletsInCartridge(InputData, /*out*/ OutHits);
	}
}

FHitResult UMPShooterGameplayAbility_RangedWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace,
	float SweepRadius, bool bIsSimulated, TArray<FHitResult>& OutHitResults) const
{
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/ true, /*IgnoreActor=*/ GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	AddAdditionalTraceIgnoreActors(TraceParams);

	const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, bIsSimulated);

	if (SweepRadius > 0.0f)
	{
		GetWorld()->SweepMultiByChannel(HitResults, StartTrace, EndTrace, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(SweepRadius), TraceParams);
	}
	else
	{
		GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, TraceChannel, TraceParams);
	}

	FHitResult Hit(ForceInit);
	if (HitResults.Num() > 0)
	{
		// Filter the output list to prevent multiple hits on the same actor;
		// this is to prevent a single bullet dealing damage multiple times to
		// a single actor if using an overlap trace
		for (FHitResult& CurHitResult : HitResults)
		{
			auto Pred = [&CurHitResult](const FHitResult& Other)
			{
				return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
			};

			if (!OutHitResults.ContainsByPredicate(Pred))
			{
				OutHitResults.Add(CurHitResult);
			}
		}

		Hit = OutHitResults.Last();
	}
	else
	{
		Hit.TraceStart = StartTrace;
		Hit.TraceEnd = EndTrace;
	}

	return Hit;
}

FHitResult UMPShooterGameplayAbility_RangedWeapon::DoSingleBulletTrace(const FVector& StartTrace,
	const FVector& EndTrace, float SweepRadius, bool bIsSimulated, TArray<FHitResult>& OutHits) const
{
#if ENABLE_DRAW_DEBUG
	if (MPShooterConsoleVariables::DrawBulletTracesDuration > 0.0f)
	{
		static float DebugThickness = 1.0f;
		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, MPShooterConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
	}
#endif // ENABLE_DRAW_DEBUG

	FHitResult Impact;

	// Trace and process instant hit if something was hit
	// First trace without using sweep radius
	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		Impact = WeaponTrace(StartTrace, EndTrace, /*SweepRadius=*/ 0.0f, bIsSimulated, /*out*/ OutHits);
	}

	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		// If this weapon didn't hit anything with a line trace and supports a sweep radius, try that
		if (SweepRadius > 0.0f)
		{
			TArray<FHitResult> SweepHits;
			Impact = WeaponTrace(StartTrace, EndTrace, SweepRadius, bIsSimulated, /*out*/ SweepHits);

			// If the trace with sweep radius enabled hit a pawn, check if we should use its hit results
			const int32 FirstPawnIdx = FindFirstPawnHitResult(SweepHits);
			if (SweepHits.IsValidIndex(FirstPawnIdx))
			{
				// If we had a blocking hit in our line trace that occurs in SweepHits before our
				// hit pawn, we should just use our initial hit results since the Pawn hit should be blocked
				bool bUseSweepHits = true;
				for (int32 Idx = 0; Idx < FirstPawnIdx; ++Idx)
				{
					const FHitResult& CurHitResult = SweepHits[Idx];

					auto Pred = [&CurHitResult](const FHitResult& Other)
					{
						return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
					};
					if (CurHitResult.bBlockingHit && OutHits.ContainsByPredicate(Pred))
					{
						bUseSweepHits = false;
						break;
					}
				}

				if (bUseSweepHits)
				{
					OutHits = SweepHits;
				}
			}
		}
	}

	return Impact;
}

void UMPShooterGameplayAbility_RangedWeapon::TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData,
	TArray<FHitResult>& OutHits) const
{
	ARangedWeapon* Weapon = InputData.Weapon;
	check(Weapon);

	const int32 BulletsPerCartridge = Weapon->GetBulletsPerCartridge();

	for (int32 BulletIndex = 0; BulletIndex < BulletsPerCartridge; ++BulletIndex)
	{
		const float ActualSpreadAngle = Weapon->GetSpreadAngle();
		const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

		const FVector BulletDir = VRandConeNormalDistribution(InputData.AimDir, HalfSpreadAngleInRadians, Weapon->GetSpreadExponent());

		const FVector EndTrace = InputData.StartTrace + (BulletDir * Weapon->GetMaxDamageRange());

		TArray<FHitResult> AllImpacts;

		FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, Weapon->GetBulletTraceSweepRadius(), /*bIsSimulated=*/ false, /*out*/ AllImpacts);

		if (Impact.GetActor())
		{
#if ENABLE_DRAW_DEBUG
			if (MPShooterConsoleVariables::DrawBulletHitDuration > 0.0f)
			{
				DrawDebugPoint(GetWorld(), Impact.ImpactPoint, MPShooterConsoleVariables::DrawBulletHitRadius, FColor::Red, false, MPShooterConsoleVariables::DrawBulletHitRadius);
			}
#endif

			if (AllImpacts.Num() > 0)
			{
				OutHits.Append(AllImpacts);
			}
		}
		else if (!Impact.bBlockingHit)
		{
			Impact.Location = EndTrace;
			Impact.ImpactPoint = EndTrace;
			OutHits.Add(Impact);
		}
		
		if (OutHits.Num() == 0)
		{
			if (!Impact.bBlockingHit)
			{
				Impact.Location = EndTrace;
				Impact.ImpactPoint = EndTrace;
			}

			OutHits.Add(Impact);
		}
	}
}

void UMPShooterGameplayAbility_RangedWeapon::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
	if (const AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		// Ignore any actors attached to the avatar doing the shooting
		TArray<AActor*> AttachedActors;
		Avatar->GetAttachedActors(/*out*/ AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
}

ECollisionChannel UMPShooterGameplayAbility_RangedWeapon::DetermineTraceChannel(FCollisionQueryParams& TraceParams,
	bool bIsSimulated) const
{
	return ECC_Visibility;
}

FTransform UMPShooterGameplayAbility_RangedWeapon::GetTargetingTransform(const APawn* SourcePawn) const
{
	check(SourcePawn);
	const double FocalDistance = 1024.0f;

	const APlayerController* PlayerController = Cast<APlayerController>(SourcePawn->Controller);
	if (!IsValid(PlayerController))
		return FTransform();

	FVector CamLoc;
	FRotator CamRot;
	
	PlayerController->GetPlayerViewPoint(/*out*/ CamLoc, /*out*/ CamRot);

	// Determine initial focal point to 
	const FVector AimDir = CamRot.Vector().GetSafeNormal();
	FVector FocalLoc = CamLoc + (AimDir * FocalDistance);

	// Move the start and focal point up in front of pawn
	const FVector WeaponLoc = GetWeaponTargetingSourceLocation();
	CamLoc = FocalLoc + (((WeaponLoc - FocalLoc) | AimDir) * AimDir);
	FocalLoc = CamLoc + (AimDir * FocalDistance);

	return FTransform(CamRot, CamLoc);
}

FVector UMPShooterGameplayAbility_RangedWeapon::GetWeaponTargetingSourceLocation() const
{
	const APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	check(AvatarPawn);
	
	if (const ARangedWeapon* WeaponInstance = GetAssociatedRangedWeapon())
	{
		if (const USkeletalMeshComponent* SkeletalMeshComponent = WeaponInstance->FindComponentByClass<USkeletalMeshComponent>())
		{
			return SkeletalMeshComponent->GetSocketLocation(TEXT("Muzzle"));
		}
		
		return WeaponInstance->GetActorLocation();
	}

	return AvatarPawn->GetActorLocation();
}

void UMPShooterGameplayAbility_RangedWeapon::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData,
																		FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	if (const FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow	ScopedPrediction(ASC);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		const FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		if (CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority())
		{
			ASC->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, ASC->ScopedPredictionKey);
		}

		// See if we still have ammo
		if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			// Let the blueprint do stuff like apply effects to the targets
			OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Weapon ability %s failed to commit"), *GetPathName());
			K2_EndAbility();
		}
	}

	// We've processed the data
	ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UMPShooterGameplayAbility_RangedWeapon::StartRangedWeaponTargeting()
{
	check(CurrentActorInfo);

	const AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);
    
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	const AController* Controller = GetControllerFromActorInfo();
	check(Controller);
    
	FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());
    
	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(/*out*/ FoundHits);

	FGameplayAbilityTargetDataHandle TargetData;
	if (FoundHits.Num() > 0)
	{
		const int32 CartridgeID = FMath::Rand();
    
		for (const FHitResult& FoundHit : FoundHits)
		{
			FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
			NewTargetData->HitResult = FoundHit;
    
			TargetData.Add(NewTargetData);
		}
	}
    
	// Process the target data immediately
	OnTargetDataReadyCallback(TargetData, FGameplayTag());
}


