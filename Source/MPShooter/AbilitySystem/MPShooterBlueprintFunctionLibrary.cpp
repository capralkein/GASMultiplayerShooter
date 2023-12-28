// Created by Capralkein


#include "MPShooterBlueprintFunctionLibrary.h"

FGameplayAbilityTargetDataHandle UMPShooterBlueprintFunctionLibrary::EffectContextGetTargetData(FGameplayEffectContextHandle EffectContextHandle)
{
	if (FMPShooterGameplayEffectContext* EffectContext = static_cast<FMPShooterGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->GetTargetData();
	}

	return FGameplayAbilityTargetDataHandle();
}
