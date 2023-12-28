// Created by Capralkein


#include "QuickBarComponent.h"
#include "InventoryItem.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"


UQuickBarComponent::UQuickBarComponent(const FObjectInitializer& ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UQuickBarComponent::BeginPlay()
{
	InitSlots();
	Super::BeginPlay();
}

void UQuickBarComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Slots);
	DOREPLIFETIME(ThisClass, ActiveSlotIndex);
}

void UQuickBarComponent::InitSlots()
{
	if (Slots.Num() < NumSlots)
	{
		Slots.AddDefaulted(NumSlots - Slots.Num());
	}
}

void UQuickBarComponent::CycleActiveSlotForward()
{
	if (Slots.Num() < 1) return;

	const int32 OldIndex = ActiveSlotIndex < 0 ? Slots.Num()-1 : ActiveSlotIndex;
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex + 1) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex_Server(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

void UQuickBarComponent::CycleActiveSlotBackward()
{
	if (Slots.Num() < 1) return;

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num()-1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex_Server(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

void UQuickBarComponent::SetActiveSlotIndex_Server_Implementation(int32 NewIndex)
{
	if (NewIndex > NumSlots - 1) return;
	
	if (ActiveSlotIndex == NewIndex && EquippedItem == nullptr)
	{
		EquipItemInSlot();
	}

	if (ActiveSlotIndex != NewIndex)
	{
		UnequipItemInSlot();
		
		ActiveSlotIndex = NewIndex;
		
		EquipItemInSlot();

		OnRep_ActiveSlotIndex();
	}
}

void UQuickBarComponent::AddInventoryItem(const TSubclassOf<AInventoryItem> InventoryItemClass)
{
	if (!InventoryItemClass)
		return;
	
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor) || !OwnerActor->HasAuthority())
		return;
	
	const int32 SlotIndex = GetNextFreeItemSlot();
	if (SlotIndex == INDEX_NONE)
		return;

	AInventoryItem* NewItem = GetWorld()->SpawnActorDeferred<AInventoryItem>(InventoryItemClass, FTransform::Identity, OwnerActor);
	if (!NewItem)
		return;

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!IsValid(Character))
		return;
	
	if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket")))
	{
		HandSocket->AttachActor(NewItem, Character->GetMesh());
	}

	NewItem->SetActorHiddenInGame(true);
	NewItem->FinishSpawning(FTransform::Identity, true);

	Slots[SlotIndex] = NewItem;
	OnRep_Slots();
}

int32 UQuickBarComponent::GetNextFreeItemSlot() const
{
	int32 SlotIndex = 0;
	for (TObjectPtr<AInventoryItem> ItemPtr : Slots)
	{
		if (ItemPtr == nullptr)
		{
			return SlotIndex;
		}
		++SlotIndex;
	}
	return INDEX_NONE;
}

AInventoryItem* UQuickBarComponent::GetActiveSlotItem() const
{
	return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

AInventoryItem* UQuickBarComponent::GetSlotItem(int32 SlotIndex) const
{
	if (Slots.Num() <= SlotIndex)
		return nullptr;

	return Slots[SlotIndex];
}

void UQuickBarComponent::RemoveItems()
{
	for (TObjectPtr<AInventoryItem> ItemPtr : Slots)
	{
		if (ItemPtr != nullptr)
		{
			ItemPtr->SetLifeSpan(0.01);
			ItemPtr == nullptr;
		}
	}
}

void UQuickBarComponent::EquipItemInSlot()
{
	if (!Slots.IsValidIndex(ActiveSlotIndex))
		return;

	if (AInventoryItem* SlotItem = Slots[ActiveSlotIndex])
	{
		EquippedItem = SlotItem;

		SlotItem->OnEquipped();

	}
}

void UQuickBarComponent::UnequipItemInSlot()
{
	if (!EquippedItem)
		return;

	EquippedItem->OnUnequipped();
	
	EquippedItem = nullptr;
}

void UQuickBarComponent::OnRep_Slots()
{
	if (OnSlotsUpdated.IsBound())
	{
		OnSlotsUpdated.Broadcast();
	}
}

void UQuickBarComponent::OnRep_ActiveSlotIndex()
{
	if (OnActiveSlotChanged.IsBound())
	{
		OnActiveSlotChanged.Broadcast();
	}
}

UQuickBarComponent* UQuickBarComponent::FindQuickBarComponent(const AActor* Owner)
{
	return IsValid(Owner) ? Cast<UQuickBarComponent>(Owner->GetComponentByClass(ThisClass::StaticClass())) : nullptr;
}
