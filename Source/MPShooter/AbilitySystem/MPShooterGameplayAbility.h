// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MPShooterGameplayAbility.generated.h"

class AMPShooterCharacter;
class UAbilityCost;

USTRUCT(BlueprintType)
struct FAbilitySimpleFailureMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	FText UserFacingReason;
};

/** Failure reason that can be used to play an animation montage when a failure occurs */
USTRUCT(BlueprintType)
struct FAbilityMontageFailureMessage
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// All the reasons why this ability has failed
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};

UENUM(BlueprintType)
enum class EAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered.
	OnInputTriggered,

	// Continually try to activate the ability while the input is active.
	WhileInputActive,
	
	// Try to activate the ability when an avatar is assigned.
	OnSpawn
};

//	Defines how an ability is meant to deactivate.
UENUM(BlueprintType)
enum class EAbilityDeactivationPolicy : uint8
{
	None,

	// Automatic end ability when input released
	OnInputRelease,

	// Automatic end ability when input triggered
	OnInputTriggered
};

UCLASS()
class MPSHOOTER_API UMPShooterGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class UMPShooterAbilitySystemComponent;

public:
	UMPShooterGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
	//~End of UGameplayAbility interface
	
protected:
	virtual void OnPawnAvatarSet() { K2_OnPawnAvatarSet(); }
	virtual void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const { K2_OnAbilityFailedToActivate(FailedReason); }
	
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

	virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const UObject*& OutAbilitySource, AActor*& OutEffectCauser) const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	void K2_OnAbilityRemoved();

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	void K2_OnPawnAvatarSet();

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityFailedToActivate")
	void K2_OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	UFUNCTION(BlueprintPure, Category="Ability")
	FActiveGameplayEffectHandle GetGrantedByEffect();

	UFUNCTION(BlueprintCallable, Category="Ability")
	UObject* GetSourceObject() const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	AMPShooterCharacter* GetMPShooterCharacterFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Ability|EffectContext", BlueprintPure)
	FGameplayEffectContextHandle MakeEffectContextFromTargetData(const FGameplayAbilityTargetDataHandle& TargetData);

	UFUNCTION(BlueprintCallable, Category = "Ability|EffectContext", BlueprintPure)
	FGameplayEffectContextHandle MakeEffectContextFromHitResult(const FHitResult& HitResult);

protected:
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Ability Activation")
	EAbilityActivationPolicy ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;

	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Ability Deactivation")
	EAbilityDeactivationPolicy DeactivationPolicy = EAbilityDeactivationPolicy::None;
	
	UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
	TArray<TObjectPtr<UAbilityCost>> AdditionalCosts;
	
public:
	FORCEINLINE const TArray<FAbilityTriggerData>& GetAbilityTriggers() const { return AbilityTriggers; }
	FORCEINLINE EAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	FORCEINLINE EAbilityDeactivationPolicy GetDeactivationPolicy() const { return DeactivationPolicy; }
};
