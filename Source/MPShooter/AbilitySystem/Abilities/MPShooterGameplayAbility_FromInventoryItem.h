// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "MPShooter/AbilitySystem/MPShooterGameplayAbility.h"
#include "MPShooterGameplayAbility_FromInventoryItem.generated.h"


class AInventoryItem;

UCLASS()
class MPSHOOTER_API UMPShooterGameplayAbility_FromInventoryItem : public UMPShooterGameplayAbility
{
	GENERATED_BODY()
public:
	UMPShooterGameplayAbility_FromInventoryItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="Ability")
	AInventoryItem* GetAssociatedInventoryItem() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
