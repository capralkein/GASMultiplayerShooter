// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "MPShooterPlayerState.generated.h"

class UMPShooterAbilitySystemComponent;

UCLASS(BlueprintType)
class MPSHOOTER_API AMPShooterPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AMPShooterPlayerState(const FObjectInitializer& Initializer = FObjectInitializer::Get());

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~IAbilitySystemInterface

public:
	UFUNCTION(BlueprintCallable, Category = "MPShooter|PlayerState")
	UMPShooterAbilitySystemComponent* GetMPShooterAbilitySystemComponent() const { return AbilitySystemComponent; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMPShooterAbilitySystemComponent> AbilitySystemComponent = nullptr;
};
