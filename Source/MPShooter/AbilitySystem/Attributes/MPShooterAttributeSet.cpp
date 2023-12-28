// Created by Capralkein


#include "MPShooterAttributeSet.h"

#include "AbilitySystemLog.h"
#include "MPShooter/AbilitySystem/MPShooterAbilitySystemComponent.h"

UMPShooterAttributeSet::UMPShooterAttributeSet()
{
}

UWorld* UMPShooterAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);
	return Outer->GetWorld();
}

void UMPShooterAttributeSet::InitFromInitializers(const TArray<FAttributeSetInitializer>& Initializers, int32 Level)
{
	for ( TFieldIterator<FProperty> It(GetClass(), EFieldIteratorFlags::IncludeSuper) ; It ; ++It )
	{
		FProperty* Property = *It;

		for (const auto& Initializer : Initializers)
		{
			if (Property->GetNameCPP() == Initializer.Attribute.GetUProperty()->GetNameCPP())
			{
				float BaseValue = 0.f;

				if (Initializer.Value.GetRichCurveConst())
				{
					BaseValue = Initializer.Value.GetRichCurveConst()->Eval(Level);
				}
				else if (Initializer.Value.ExternalCurve)
				{
					BaseValue = Initializer.Value.ExternalCurve->GetFloatValue(Level);
				}
				else
				{
					ABILITY_LOG(Error, TEXT("[MPShooterAttributeSet] <%s> Can't init from initializer - invalid curve data!"), *GetName());
				}
				
				if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
				{
					void *Data = NumericProperty->ContainerPtrToValuePtr<void>(this);
					NumericProperty->SetFloatingPointPropertyValue(Data, BaseValue);
				}
				else if (FGameplayAttribute::IsGameplayAttributeDataProperty(Property))
				{
					const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
					check(StructProperty);
					FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(this);
					check(DataPtr);
					DataPtr->SetBaseValue(BaseValue);
					DataPtr->SetCurrentValue(BaseValue);
				}

				break;
			}
		}
	}
}

UMPShooterAbilitySystemComponent* UMPShooterAttributeSet::GetMPShooterAbilitySystemComponent() const
{
	return Cast<UMPShooterAbilitySystemComponent>(GetOwningAbilitySystemComponent());

}
