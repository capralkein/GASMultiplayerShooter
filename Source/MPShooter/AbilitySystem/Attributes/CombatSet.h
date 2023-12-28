#pragma once

#include "CoreMinimal.h"
#include "MPShooterAttributeSet.h"
#include "CombatSet.generated.h"


UCLASS(BlueprintType)
class MPSHOOTER_API UCombatSet : public UMPShooterAttributeSet
{
	GENERATED_BODY()
public:

	UCombatSet();

	ATTRIBUTE_ACCESSORS(UCombatSet, BaseDamage);
	ATTRIBUTE_ACCESSORS(UCombatSet, BaseHeal);

protected:

	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseHeal(const FGameplayAttributeData& OldValue);

private:

	// The base amount of damage to apply in the damage execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "Attribute|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage;

	// The base amount of healing to apply in the heal execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHeal, Category = "Attribute|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHeal;
};
