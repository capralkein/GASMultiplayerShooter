// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilitySet.generated.h"

class UMPShooterAbilitySystemComponent;
class UAttributeSet;
class UMPShooterAttributeSet;
class UGameplayEffect;
class UMPShooterGameplayAbility;
struct FAttributeSetInitializer;

// Data used by the ability set to grant gameplay abilities.
USTRUCT(BlueprintType)
struct FAbilitySet_GameplayAbility
{
	GENERATED_BODY()
public:
	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMPShooterGameplayAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

// Data used by the ability set to grant gameplay effects.
USTRUCT(BlueprintType)
struct FAbilitySet_GameplayEffect
{
	GENERATED_BODY()
public:
	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// Level of gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

// Data used by the ability set to grant attribute sets.
USTRUCT(BlueprintType)
struct FAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMPShooterAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly)
	FInt32Interval RandLevelRange = FInt32Interval(0, 0);
	
	UPROPERTY(EditDefaultsOnly)
	TArray<FAttributeSetInitializer> Initializers;
};

//	Data used to store handles to what has been granted by the ability set.
USTRUCT(BlueprintType)
struct FAbilitySet_GrantedHandles
{
	GENERATED_BODY()
public:
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	void TakeFromAbilitySystem(UMPShooterAbilitySystemComponent* ASC);

protected:

	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// Pointers to the granted attribute sets
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

UCLASS(BlueprintType, Const, Meta = (DisplayName = "Ability Set"))
class MPSHOOTER_API UAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.
	void GiveToAbilitySystem(UMPShooterAbilitySystemComponent* ASC, FAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

	TArray<FAbilitySet_GameplayAbility> GetGrantedGameplayAbilities() const { return GrantedGameplayAbilities; } 

protected:
	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// Attribute sets to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
	TArray<FAbilitySet_AttributeSet> GrantedAttributes;
};
