// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MPShooterAttributeSet.generated.h"

class UMPShooterAbilitySystemComponent;
struct FGameplayEffectSpec;

/**
 * This macro defines a set of helper functions for accessing and initializing attributes.
 *
 * The following example of the macro:
 *		ATTRIBUTE_ACCESSORS(UHealthSet, Health)
 *		will create the following functions:
 *		static FGameplayAttribute GetHealthAttribute();
 *		float GetHealth() const;
 *		void SetHealth(float NewVal);
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_MULTICAST_DELEGATE_SixParams(FMPShooterAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);

USTRUCT(BlueprintType)
struct MPSHOOTER_API FAttributeSetInitializer
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	FGameplayAttribute Attribute;
	
	UPROPERTY(EditDefaultsOnly)
	FRuntimeFloatCurve Value;
};

UCLASS()
class MPSHOOTER_API UMPShooterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UMPShooterAttributeSet();

	// UAttributeSet
	virtual UWorld* GetWorld() const override;
	//~UAttributeSet
	
public:
	void InitFromInitializers(const TArray<FAttributeSetInitializer>& Initializers, int32 Level = 0);
	
	UMPShooterAbilitySystemComponent* GetMPShooterAbilitySystemComponent() const;
};
