// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "MPShooterAbilitySystemComponent.h"
#include "MPShooterAbilitySystemGlobals.generated.h"

UCLASS(Config=Game)
class MPSHOOTER_API UMPShooterAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_UCLASS_BODY()

public:
	// UAbilitySystemGlobals
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
	//~UAbilitySystemGlobals
	
	static UMPShooterAbilitySystemComponent* GetMPShooterAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent=true);
};
