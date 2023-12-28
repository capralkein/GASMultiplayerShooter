// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MPShooter/AbilitySystem/AbilitySet.h"
#include "MPShooter/AbilitySystem/GameplayTagStack.h"
#include "InventoryItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryItemStatTagChanged);

class USphereComponent;
class UAbilitySet;

UCLASS(Abstract, NotPlaceable)
class MPSHOOTER_API AInventoryItem : public AActor
{
	GENERATED_BODY()
public:
	AInventoryItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnEquipped();
	virtual void OnUnequipped();

protected:
	// AActor
	virtual void BeginPlay() override;
	//~AActor
public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category=Inventory, meta=(DisplayName="On Equipped"))
	void K2_OnEquipped();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category=Inventory, meta=(DisplayName="On Unequipped"))
	void K2_OnUnequipped();

protected:
	UPROPERTY(EditDefaultsOnly, Category=Settings)
	TArray<TObjectPtr<const UAbilitySet>> AbilitySetsToGrant;

	UPROPERTY(EditDefaultsOnly, Category=Settings)
	TMap<FGameplayTag, int32> InitialItemStats;

	UPROPERTY(EditDefaultsOnly, Category=Settings)
	FText ItemName;

public:
	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Inventory)
	bool HasStatTag(FGameplayTag Tag) const;

	UPROPERTY(BlueprintAssignable)
	FInventoryItemStatTagChanged OnInventoryItemStatTagChanged;
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_StatTags)
	FGameplayTagStackContainer StatTags;

	UFUNCTION()
	virtual void OnRep_StatTags();

	void BroadcastOnStatTagsChanged() const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Inventory Item Properties")
	USkeletalMeshComponent* SkeletalMesh;
	
	UPROPERTY()
	FAbilitySet_GrantedHandles GrantedHandles;
	
	bool bIsEquipped = false;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsEquipped() const { return bIsEquipped; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FText GetItemName() const { return ItemName; }

};
