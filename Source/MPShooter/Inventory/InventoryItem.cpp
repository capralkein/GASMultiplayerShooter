// Created by Capralkein


#include "InventoryItem.h"

#include "Components/SphereComponent.h"
#include "MPShooter/AbilitySystem/AbilitySet.h"
#include "MPShooter/AbilitySystem/MPShooterAbilitySystemGlobals.h"
#include "Net/UnrealNetwork.h"


AInventoryItem::AInventoryItem(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SetRootComponent(SkeletalMesh);

	SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AInventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
}

void AInventoryItem::OnEquipped()
{
	bIsEquipped = true;
	SetActorHiddenInGame(false);

	if (!Owner)	return;
	
	if (UMPShooterAbilitySystemComponent* ASC = UMPShooterAbilitySystemGlobals::GetMPShooterAbilitySystemComponentFromActor(Owner))
	{
		for (const auto& AbilitySet : AbilitySetsToGrant)
		{
			AbilitySet->GiveToAbilitySystem(ASC, &GrantedHandles, this);
		}
	}
	
	K2_OnEquipped();
}

void AInventoryItem::OnUnequipped()
{
	bIsEquipped = false;
	SetActorHiddenInGame(true);

	if (!Owner)	return;

	if (UMPShooterAbilitySystemComponent* ASC = UMPShooterAbilitySystemGlobals::GetMPShooterAbilitySystemComponentFromActor(Owner))
	{
		GrantedHandles.TakeFromAbilitySystem(ASC);
	}
	
	K2_OnUnequipped();
}


void AInventoryItem::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
		return;
	
	for (const auto& KVP : InitialItemStats)
	{
		AddStatTagStack(KVP.Key, KVP.Value);
	}
}

void AInventoryItem::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
	BroadcastOnStatTagsChanged();
}

void AInventoryItem::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
	BroadcastOnStatTagsChanged();
}

int32 AInventoryItem::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool AInventoryItem::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void AInventoryItem::OnRep_StatTags()
{
	BroadcastOnStatTagsChanged();
}

void AInventoryItem::BroadcastOnStatTagsChanged() const
{
	if (OnInventoryItemStatTagChanged.IsBound())
	{
		OnInventoryItemStatTagChanged.Broadcast();
	}
}

