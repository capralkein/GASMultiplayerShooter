// Created by Capralkein


#include "MPShooterGameplayAbility_FromInventoryItem.h"

#include "Misc/DataValidation.h"
#include "MPShooter/Inventory/InventoryItem.h"

UMPShooterGameplayAbility_FromInventoryItem::UMPShooterGameplayAbility_FromInventoryItem(
	const FObjectInitializer& ObjectInitializer)
{
}

AInventoryItem* UMPShooterGameplayAbility_FromInventoryItem::GetAssociatedInventoryItem() const
{
	return Cast<AInventoryItem>(GetSourceObject());
}

#if WITH_EDITOR
EDataValidationResult UMPShooterGameplayAbility_FromInventoryItem::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (InstancingPolicy == EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		Context.AddError(NSLOCTEXT("Inventory", "InventoryItemAbilityMustBeInstanced", "Equipment ability must be instanced"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
