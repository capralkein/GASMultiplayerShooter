// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MPShooterCharacter.generated.h"

class UPawnData;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UQuickBarComponent;
struct FInputActionValue;
struct FGameplayTag;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilitySystemReady);

UCLASS(config=Game)
class AMPShooterCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AMPShooterCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ACharacter
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual auto SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) -> void override;
	//~ACharacter

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~IAbilitySystemInterface

	// Input
	virtual void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	virtual void Input_AbilityInputTagHeld(FGameplayTag InputTag);
	virtual void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	//~Input
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnAbilitySystemReady OnAbilitySystemReady;
	
	UFUNCTION(BlueprintImplementableEvent, Category=ASC, meta=(DisplayName="On Ability System Ready"))
	void K2_OnAbilitySystemReady();

public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Settings, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const UPawnData> PawnData = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UQuickBarComponent> QuickBarComponent = nullptr;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);


public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

