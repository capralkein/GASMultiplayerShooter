// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuickBarComponent.generated.h"

class AInventoryItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuickBarEvent);

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class MPSHOOTER_API UQuickBarComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// UActorComponent
	virtual void BeginPlay() override;
	//~UActorComponent

	void InitSlots();
	int32 GetNextFreeItemSlot() const;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddInventoryItem(TSubclassOf<AInventoryItem> InventoryItemClass);

	UFUNCTION(BlueprintCallable, Category="QuickBar")
	void CycleActiveSlotForward();

	UFUNCTION(BlueprintCallable, Category="QuickBar")
	void CycleActiveSlotBackward();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category="QuickBar")
	void SetActiveSlotIndex_Server(int32 NewIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	TArray<AInventoryItem*> GetSlots() const { return Slots; }

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	AInventoryItem* GetActiveSlotItem() const;

	UFUNCTION(BlueprintPure)
	AInventoryItem* GetSlotItem(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RemoveItems();

	UPROPERTY(BlueprintAssignable)
    FQuickBarEvent OnSlotsUpdated;

	UPROPERTY(BlueprintAssignable)
	FQuickBarEvent OnActiveSlotChanged;

private:
	void EquipItemInSlot();
	void UnequipItemInSlot();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="QuickBar")
	int32 NumSlots = 3;

	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION()
	void OnRep_ActiveSlotIndex();

private:
	UPROPERTY(ReplicatedUsing=OnRep_Slots)
	TArray<TObjectPtr<AInventoryItem>> Slots;

	UPROPERTY(ReplicatedUsing=OnRep_ActiveSlotIndex)
	int32 ActiveSlotIndex = -1;

	UPROPERTY()
	TObjectPtr<AInventoryItem> EquippedItem;

public:
	UFUNCTION(BlueprintPure)
	static UQuickBarComponent* FindQuickBarComponent(const AActor* Owner);
};
