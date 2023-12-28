// Created by Capralkein


#include "MPShooterGameplayAbility.h"

#include "MPShooterAbilitySystemComponent.h"
#include "Costs/AbilityCost.h"
#include "Effects/GameplayEffectContext.h"
#include "MPShooter/Character/MPShooterCharacter.h"

UMPShooterGameplayAbility::UMPShooterGameplayAbility(const FObjectInitializer& ObjectInitializer)
{
}

bool UMPShooterGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

bool UMPShooterGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}

	// Verify we can afford any additional costs
	for (TObjectPtr<UAbilityCost> AdditionalCost : AdditionalCosts)
	{
		if (!AdditionalCost)
			continue;
		
		if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, /*inout*/ OptionalRelevantTags))
			return false;
	}

	return true;
}

void UMPShooterGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	check(ActorInfo);
	
	for (TObjectPtr<UAbilityCost> AdditionalCost : AdditionalCosts)
	{
		if (!AdditionalCost)
			continue;
		
		AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
	}
}

void UMPShooterGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();
	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UMPShooterGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (TriggerEventData)
	{
		CurrentEventData = *TriggerEventData;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMPShooterGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UMPShooterGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (DeactivationPolicy == EAbilityDeactivationPolicy::OnInputTriggered)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UMPShooterGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (DeactivationPolicy == EAbilityDeactivationPolicy::OnInputRelease)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

FGameplayEffectContextHandle UMPShooterGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	FGameplayEffectContextHandle ContextHandle = Super::MakeEffectContext(Handle, ActorInfo);

	FMPShooterGameplayEffectContext* EffectContext = FMPShooterGameplayEffectContext::ExtractEffectContext(ContextHandle);
	check(EffectContext);

	check(ActorInfo);

	AActor* EffectCauser = nullptr;
	const UObject* AbilitySource = nullptr;
	GetAbilitySource(Handle, ActorInfo, AbilitySource, EffectCauser);

	const UObject* SourceObject = UGameplayAbility::GetSourceObject(Handle, ActorInfo);

	AActor* Instigator = ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr;

	EffectContext->SetAbilitySource(AbilitySource);
	EffectContext->AddInstigator(Instigator, EffectCauser);
	EffectContext->AddSourceObject(SourceObject);

	return ContextHandle;
}

void UMPShooterGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec) const
{
	const bool bIsPredicting = (Spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);

	// Try to activate if activation policy is on spawn.
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && (ActivationPolicy == EAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

void UMPShooterGameplayAbility::GetAbilitySource(FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const UObject*& OutAbilitySource, AActor*& OutEffectCauser) const
{
	OutEffectCauser = ActorInfo->AvatarActor.Get();
	OutAbilitySource = UGameplayAbility::GetSourceObject(Handle, ActorInfo);
}

FActiveGameplayEffectHandle UMPShooterGameplayAbility::GetGrantedByEffect()
{
	if (!IsInstantiated())
		return FActiveGameplayEffectHandle();
	
	check(CurrentActorInfo);
	
	if (!CurrentActorInfo)
		return FActiveGameplayEffectHandle();
	
	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo_Checked();
	
	return AbilitySystemComponent->FindActiveGameplayEffectHandle(GetCurrentAbilitySpecHandle());
}

UObject* UMPShooterGameplayAbility::GetSourceObject() const
{
	if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Spec->SourceObject.Get();
	}

	return nullptr;
}

AMPShooterCharacter* UMPShooterGameplayAbility::GetMPShooterCharacterFromActorInfo() const
{
	return CurrentActorInfo ? Cast<AMPShooterCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

AController* UMPShooterGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// Look for a player controller or pawn in the owner chain.
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* PC = Cast<AController>(TestActor))
			{
				return PC;
			}

			if (const APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

FGameplayEffectContextHandle UMPShooterGameplayAbility::MakeEffectContextFromTargetData(
	const FGameplayAbilityTargetDataHandle& TargetData)
{
	FGameplayEffectContextHandle ContextHandle = MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);
	FMPShooterGameplayEffectContext* EffectContext = FMPShooterGameplayEffectContext::ExtractEffectContext(ContextHandle);
	check(EffectContext);

	EffectContext->AddTargetData(TargetData);

	return ContextHandle;	
}

FGameplayEffectContextHandle UMPShooterGameplayAbility::MakeEffectContextFromHitResult(const FHitResult& HitResult)
{
	FGameplayEffectContextHandle ContextHandle = MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);
	FMPShooterGameplayEffectContext* EffectContext = FMPShooterGameplayEffectContext::ExtractEffectContext(ContextHandle);
	check(EffectContext);

	EffectContext->AddHitResult(HitResult);
	
	return ContextHandle;
}
