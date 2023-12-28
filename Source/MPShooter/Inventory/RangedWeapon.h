// Created by Capralkein

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "RangedWeapon.generated.h"

UCLASS()
class MPSHOOTER_API ARangedWeapon : public AInventoryItem
{
	GENERATED_BODY()

public:
	ARangedWeapon();

protected:
	// The maximum distance at which this weapon can deal damage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config", meta=(ForceUnits=cm))
	float MaxDamageRange = 25000.0f;

	// Number of bullets to fire in a single cartridge (typically 1, but may be more for shotguns)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config")
	int32 BulletsPerCartridge = 1;

	// The spread angle (in degrees, diametrical)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config")
	float SpreadAngle = 0.0f;

	// Spread exponent, affects how tightly shots will cluster around the center line
	// when the weapon has spread (non-perfect accuracy). Higher values will cause shots
	// to be closer to the center (default is 1.0 which means uniformly within the spread range)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.1), Category="Spread|Fire Params")
	float SpreadExponent = 1.0f;

	// The radius for bullet traces sweep spheres (0.0 will result in a line trace)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config", meta=(ForceUnits=cm))
	float BulletTraceSweepRadius = 0.0f;

	// Damage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.1), Category="Damage")
	float DamagePerBullet = 1.0f;

public:
	float GetMaxDamageRange() const
	{
		return MaxDamageRange;
	}

	int32 GetBulletsPerCartridge() const
	{
		return BulletsPerCartridge;
	}

	float GetSpreadAngle() const
	{
		return SpreadAngle;
	}

	float GetSpreadExponent() const
	{
		return SpreadExponent;
	}

	float GetBulletTraceSweepRadius() const
	{
		return BulletTraceSweepRadius;
	}

};
