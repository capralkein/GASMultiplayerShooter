// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "MPShooterInputComponent.generated.h"

/**
 * FMPShooterInputAction
 *
 *	Struct used to map a input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FMPShooterInputAction
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
	const UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * UMPShooterlInputConfig
 *
 *	Non-mutable data asset that contains input configuration properties.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "MPShooter Input Config"))
class MPSHOOTER_API UMPShooterInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	UMPShooterInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "MPShooter|Input")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;
	
	UFUNCTION(BlueprintCallable, Category = "MPShooter|Input")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;
public:
	// List of input actions used by the owner.  These input actions are mapped to a gameplay tag and must be manually bound.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FMPShooterInputAction> NativeInputActions;

	// List of input actions used by the owner.  These input actions are mapped to a gameplay tag and are automatically bound to abilities with matching input tags.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FMPShooterInputAction> AbilityInputActions;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MPSHOOTER_API UMPShooterInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:
	UMPShooterInputComponent(const FObjectInitializer& ObjectInitializer);

	template<class UserClass, typename FuncType>
	void BindNativeAction(const UMPShooterInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound, TArray<uint32>& BindHandles);

	template<class UserClass, typename PressedFuncType, typename HeldFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UMPShooterInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, HeldFuncType HeldFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);
};

template <class UserClass, typename FuncType>
void UMPShooterInputComponent::BindNativeAction(const UMPShooterInputConfig* InputConfig,
	const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound,
	TArray<uint32>& BindHandles)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindHandles.Add(BindAction(IA, TriggerEvent, Object, Func).GetHandle());
	}
}

template <class UserClass, typename PressedFuncType, typename HeldFuncType, typename ReleasedFuncType>
void UMPShooterInputComponent::BindAbilityActions(const UMPShooterInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, HeldFuncType HeldFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FMPShooterInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}
			if (HeldFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Ongoing, Object, HeldFunc, Action.InputTag).GetHandle());
			}
			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Canceled, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}
