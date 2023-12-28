#include "DamageExecution.h"

#include "GameplayEffectContext.h"
#include "MPShooter/AbilitySystem/Attributes/CombatSet.h"
#include "MPShooter/AbilitySystem/Attributes/HealthSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DamageExecution)

struct FDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;

	FDamageStatics()
	{
		BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(UCombatSet::GetBaseDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}

UDamageExecution::UDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef);
}

void UDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FMPShooterGameplayEffectContext* TypedContext = FMPShooterGameplayEffectContext::ExtractEffectContext(Spec.GetContext());
	check(TypedContext);

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float BaseDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef, EvaluateParameters, BaseDamage);

	// Clamping is done when damage is converted to -health
	const float DamageDone = FMath::Max(BaseDamage, 0.0f);

	if (DamageDone > 0.0f)
	{
		// Apply a damage modifier, this gets turned into - health on the target
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageDone));
	}
#endif // #if WITH_SERVER_CODE
}
