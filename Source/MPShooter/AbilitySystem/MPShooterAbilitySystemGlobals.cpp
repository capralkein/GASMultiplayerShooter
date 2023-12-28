// Created by Capralkein


#include "MPShooterAbilitySystemGlobals.h"

#include "AbilitySystemInterface.h"
#include "Effects/GameplayEffectContext.h"

UMPShooterAbilitySystemGlobals::UMPShooterAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayEffectContext* UMPShooterAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FMPShooterGameplayEffectContext();
}

UMPShooterAbilitySystemComponent* UMPShooterAbilitySystemGlobals::GetMPShooterAbilitySystemComponentFromActor(
	const AActor* Actor, bool LookForComponent)
{
	if (!IsValid(Actor))
		return nullptr;

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
		return Cast<UMPShooterAbilitySystemComponent>(ASI->GetAbilitySystemComponent());

	if (LookForComponent)
		return Actor->FindComponentByClass<UMPShooterAbilitySystemComponent>();

	return nullptr;
}
