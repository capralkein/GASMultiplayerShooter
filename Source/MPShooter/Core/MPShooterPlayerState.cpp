// Created by Capralkein

#include "MPShooterPlayerState.h"

#include "MPShooter/AbilitySystem/MPShooterAbilitySystemComponent.h"

AMPShooterPlayerState::AMPShooterPlayerState(const FObjectInitializer& Initializer)	: Super(Initializer)
{
	AbilitySystemComponent = Initializer.CreateDefaultSubobject<UMPShooterAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	NetUpdateFrequency = 100.0f;
}

UAbilitySystemComponent* AMPShooterPlayerState::GetAbilitySystemComponent() const
{
	return GetMPShooterAbilitySystemComponent();
}
