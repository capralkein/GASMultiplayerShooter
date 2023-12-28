// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityCost.generated.h"

class UMPShooterGameplayAbility;

/**
 * UAbilityCost
 *
 * Base class for costs that a GameplayAbility has (e.g., ammo or charges)
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class MPSHOOTER_API UAbilityCost : public UObject
{
	GENERATED_BODY()
public:
	UAbilityCost() {}

	/**
	 * Checks if we can afford this cost.
	 *
	 * A failure reason tag can be added to OptionalRelevantTags (if non-null), which can be queried
	 * elsewhere to determine how to provide user feedback (e.g., a clicking noise if a weapon is out of ammo)
	 * 
	 * Ability and ActorInfo are guaranteed to be non-null on entry, but OptionalRelevantTags can be nullptr.
	 * 
	 * @return true if we can pay for the ability, false otherwise.
	 */
	virtual bool CheckCost(const UMPShooterGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
	{
		return true;
	}

	/**
	 * Applies the ability's cost to the target
	 *
	 * Notes:
	 * - Your implementation don't need to check ShouldOnlyApplyCostOnHit(), the caller does that for you.
	  * - Ability and ActorInfo are guaranteed to be non-null on entry.
	 */
	virtual void ApplyCost(const UMPShooterGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
	{
	}
};
