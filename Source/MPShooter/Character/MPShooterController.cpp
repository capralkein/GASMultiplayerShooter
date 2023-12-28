// Created by Capralkein


#include "MPShooterController.h"

#include "GameFramework/PlayerState.h"
#include "MPShooter/AbilitySystem/MPShooterAbilitySystemComponent.h"
#include "MPShooter/AbilitySystem/MPShooterAbilitySystemGlobals.h"

UAbilitySystemComponent* AMPShooterController::GetAbilitySystemComponent() const
{
	return PlayerState ? UMPShooterAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState) : nullptr;
}

void AMPShooterController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UMPShooterAbilitySystemComponent* AbilitySystemComponent = UMPShooterAbilitySystemGlobals::GetMPShooterAbilitySystemComponentFromActor(this))
	{
		AbilitySystemComponent->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}
