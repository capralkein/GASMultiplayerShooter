// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MPShooterGameplayAbility.h"
#include "MPShooterAbilitySystemComponent.generated.h"

class UMPShooterAttributeSet;

UCLASS()
class MPSHOOTER_API UMPShooterAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	// UAbilitySystemComponent
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	//~UAbilitySystemComponent
	
	// Input
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();
	//~Input

	UMPShooterAttributeSet* GetOrCreateAttributeSet(TSubclassOf<UMPShooterAttributeSet> AttributeClass);

protected:
	// UAbilitySystemComponent
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
	//~UAbilitySystemComponent

	bool TryActivateAbility_Internal(FGameplayAbilitySpecHandle AbilityToActivate, bool bAllowRemoteActivation = true);
	void TryActivateAbilitiesOnSpawn();

	UFUNCTION(Client, Unreliable)
	void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);
	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);
	
	typedef TFunctionRef<bool(const UMPShooterGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	/** Allow events to be registered for specific gameplay tags being added or removed */
	FOnGameplayEffectTagCountChanged& RegisterBlockedGameplayTagEvent(FGameplayTag Tag, EGameplayTagEventType::Type EventType=EGameplayTagEventType::NewOrRemoved);

	/** Unregister previously added events */
	void UnregisterBlockedGameplayTagEvent(FDelegateHandle DelegateHandle, FGameplayTag Tag, EGameplayTagEventType::Type EventType=EGameplayTagEventType::NewOrRemoved);

	/** Gets the ability target data associated with the given ability handle and activation info */
	void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

protected:
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
};
