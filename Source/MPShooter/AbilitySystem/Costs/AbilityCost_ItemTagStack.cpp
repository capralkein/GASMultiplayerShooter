#include "AbilityCost_ItemTagStack.h"
#include "NativeGameplayTags.h"
#include "MPShooter/AbilitySystem/Abilities/MPShooterGameplayAbility_FromInventoryItem.h"
#include "MPShooter/Inventory/InventoryItem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityCost_ItemTagStack)

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_FAIL_COST, "Ability.ActivateFail.Cost");

UAbilityCost_ItemTagStack::UAbilityCost_ItemTagStack()
{
	Quantity.SetValue(1.0f);
	FailureTag = TAG_ABILITY_FAIL_COST;
}

bool UAbilityCost_ItemTagStack::CheckCost(const UMPShooterGameplayAbility* InAbility,	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (const UMPShooterGameplayAbility_FromInventoryItem* Ability = Cast<const UMPShooterGameplayAbility_FromInventoryItem>(InAbility))
	{
		if (const AInventoryItem* Item = Ability->GetAssociatedInventoryItem())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);
			const bool bCanApplyCost = Item->GetStatTagStackCount(Tag) >= NumStacks;

			// Inform other abilities why this cost cannot be applied
			if (!bCanApplyCost && OptionalRelevantTags && FailureTag.IsValid())
			{
				OptionalRelevantTags->AddTag(FailureTag);				
			}
			
			return bCanApplyCost;
		}
	}
	return false;
}

void UAbilityCost_ItemTagStack::ApplyCost(const UMPShooterGameplayAbility* InAbility,	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (const UMPShooterGameplayAbility_FromInventoryItem* Ability = Cast<const UMPShooterGameplayAbility_FromInventoryItem>(InAbility))
		{
			if (AInventoryItem* Item = Ability->GetAssociatedInventoryItem())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				Item->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}
