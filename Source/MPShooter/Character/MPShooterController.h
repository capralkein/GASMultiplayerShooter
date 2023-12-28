// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerController.h"
#include "MPShooterController.generated.h"


UCLASS()
class MPSHOOTER_API AMPShooterController : public APlayerController, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~IAbilitySystemInterface

	//~APlayerController interface
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~APlayerController interface
};
