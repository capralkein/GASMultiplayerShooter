// Created by Capralkein


#include "MPShooterInputComponent.h"

UMPShooterInputConfig::UMPShooterInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* UMPShooterInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag,
	bool bLogNotFound) const
{
	for (const FMPShooterInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UMPShooterInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag,
	bool bLogNotFound) const
{
	for (const FMPShooterInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

UMPShooterInputComponent::UMPShooterInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UMPShooterInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (const uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
