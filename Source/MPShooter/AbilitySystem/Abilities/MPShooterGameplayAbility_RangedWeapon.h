// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "MPShooterGameplayAbility_FromInventoryItem.h"
#include "MPShooterGameplayAbility_RangedWeapon.generated.h"

class ARangedWeapon;

UCLASS()
class MPSHOOTER_API UMPShooterGameplayAbility_RangedWeapon : public UMPShooterGameplayAbility_FromInventoryItem
{
	GENERATED_BODY()
public:
	UMPShooterGameplayAbility_RangedWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="Ability")
	ARangedWeapon* GetAssociatedRangedWeapon() const;

	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

protected:
	struct FRangedWeaponFiringInput
	{
		FVector StartTrace;
		FVector EndAim;
		FVector AimDir;
		ARangedWeapon* Weapon = nullptr;
		bool bCanPlayBulletFX = false;

		FRangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};
protected:
	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);

protected:
	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits) const;
	
	// Does a single weapon trace, either sweeping or ray depending on if SweepRadius is above zero
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const;
	
	// Wrapper around WeaponTrace to handle trying to do a ray trace before falling back to a sweep trace if there were no hits and SweepRadius is above zero 
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const;
	
	// Traces all of the bullets in a single cartridge
	void TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits) const;

	virtual void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;

	virtual ECollisionChannel DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const;
	
	FTransform GetTargetingTransform(const APawn* SourcePawn) const;
	FVector GetWeaponTargetingSourceLocation() const;
	
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

protected:
	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();

	UFUNCTION(BlueprintImplementableEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
};
