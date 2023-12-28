﻿#include "CombatSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CombatSet)

class FLifetimeProperty;


UCombatSet::UCombatSet()
	: BaseDamage(0.0f)
	, BaseHeal(0.0f)
{
}

void UCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCombatSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
}

void UCombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatSet, BaseDamage, OldValue);
}

void UCombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatSet, BaseHeal, OldValue);
}