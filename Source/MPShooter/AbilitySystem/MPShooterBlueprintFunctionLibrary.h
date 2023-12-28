// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "Effects/GameplayEffectContext.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MPShooterBlueprintFunctionLibrary.generated.h"


UCLASS()
class MPSHOOTER_API UMPShooterBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// Returns TargetData
	UFUNCTION(BlueprintPure, Category = "Ability|EffectContext", Meta = (DisplayName = "GetTargetData"))
	static FGameplayAbilityTargetDataHandle EffectContextGetTargetData(FGameplayEffectContextHandle EffectContext);
};
