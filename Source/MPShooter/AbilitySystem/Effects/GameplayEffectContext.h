// Created by Capralkein

#pragma once

#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayEffectContext.generated.h"

class AActor;
class FArchive;
class UObject;

USTRUCT()
struct FMPShooterGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FMPShooterGameplayEffectContext()
		: FGameplayEffectContext()
	{
	}

	FMPShooterGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser)
	{
	}

	/** Returns the wrapped FMPShooterGameplayEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
	static FMPShooterGameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

	/** Sets the object used as the ability source */
	void SetAbilitySource(const UObject* InObject);

	/** Returns the ability source interface associated with the source object. Only valid on the authority. */
	const UObject* GetAbilitySource() const;

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FMPShooterGameplayEffectContext* NewContext = new FMPShooterGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
			NewContext->TargetData.Append(TargetData);
		}
		return NewContext;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FMPShooterGameplayEffectContext::StaticStruct();
	}

	/** Overridden to serialize new fields */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

protected:
	UPROPERTY()
	TWeakObjectPtr<const UObject> AbilitySourceObject;

public:
	virtual FGameplayAbilityTargetDataHandle GetTargetData()
	{
		return TargetData;
	}

	virtual void AddTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
	{
		TargetData.Append(TargetDataHandle);
	}
	
protected:
	FGameplayAbilityTargetDataHandle TargetData;
};

template<>
struct TStructOpsTypeTraits<FMPShooterGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FMPShooterGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

