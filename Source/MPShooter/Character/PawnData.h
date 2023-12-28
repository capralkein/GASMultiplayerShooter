// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PawnData.generated.h"


class AInventoryItem;
class UMPShooterInputConfig;
class UAbilitySet;
class UInputMappingContext;

UCLASS(BlueprintType, Const, Meta = (DisplayName = "Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class MPSHOOTER_API UPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPawnData(const FObjectInitializer& ObjectInitializer);
public:
	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UAbilitySet>> AbilitySets;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UMPShooterInputConfig> InputConfig = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> MappingContexts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TArray<TSubclassOf<AInventoryItem>> InventoryItems;
};
