// Created by Capralkein

#include "GameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayEffectContext)

FMPShooterGameplayEffectContext* FMPShooterGameplayEffectContext::ExtractEffectContext(
	FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if (BaseEffectContext && BaseEffectContext->GetScriptStruct()->IsChildOf(FMPShooterGameplayEffectContext::StaticStruct()))
	{
		return static_cast<FMPShooterGameplayEffectContext*>(BaseEffectContext);
	}

	return nullptr;
}

void FMPShooterGameplayEffectContext::SetAbilitySource(const UObject* InObject)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
}

const UObject* FMPShooterGameplayEffectContext::GetAbilitySource() const
{
	return AbilitySourceObject.Get();
}

bool FMPShooterGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	return FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess) && TargetData.NetSerialize(Ar, Map, bOutSuccess);
}
