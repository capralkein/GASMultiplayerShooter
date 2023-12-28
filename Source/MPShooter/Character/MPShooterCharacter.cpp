// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPShooterCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MPShooterInputComponent.h"
#include "PawnData.h"
#include "GameFramework/PlayerState.h"
#include "MPShooter/AbilitySystem/AbilitySet.h"
#include "MPShooter/AbilitySystem/MPShooterAbilitySystemComponent.h"
#include "MPShooter/Core/MPShooterPlayerState.h"
#include "MPShooter/Inventory/QuickBarComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMPShooterCharacter

AMPShooterCharacter::AMPShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	QuickBarComponent = CreateDefaultSubobject<UQuickBarComponent>(TEXT("QuickBarComponent"));
}

void AMPShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PawnData && !PawnData->MappingContexts.IsEmpty())
	{
		if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				for (const auto& Context : PawnData->MappingContexts)
				{
					if (!Context)
						continue;
					
					Subsystem->AddMappingContext(Context, 0);
				}
			}
		}
	}
}

void AMPShooterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->ClearActorInfo();
	}

	if (IsValid(QuickBarComponent))
	{			
		QuickBarComponent->RemoveItems();
	}
}

void AMPShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (!HasAuthority())
		return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
		return;

	if (PawnData)
	{
		for (const UAbilitySet* AbilitySet : PawnData->AbilitySets)
		{
			if (!IsValid(AbilitySet))
				continue;

			AbilitySet->GiveToAbilitySystem(Cast<UMPShooterAbilitySystemComponent>(ASC), nullptr);
		}

		if (IsValid(QuickBarComponent))
		{
			QuickBarComponent->InitSlots();
			
			for (const auto& Item : PawnData->InventoryItems)
			{
				QuickBarComponent->AddInventoryItem(Item);
			}
			
			QuickBarComponent->CycleActiveSlotForward();
		}
	}

	ASC->InitAbilityActorInfo(GetPlayerState(), this);
	
	K2_OnAbilitySystemReady();
	
	if (OnAbilitySystemReady.IsBound())
	{
		OnAbilitySystemReady.Broadcast();
	}
	
	//InitTagPropertyMap(ASC); // TODO
}

void AMPShooterCharacter::OnRep_PlayerState()
{
	if (HasAuthority())
		return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
		return;
	
	ASC->InitAbilityActorInfo(GetPlayerState(), this);
	
	K2_OnAbilitySystemReady();
	
	if (OnAbilitySystemReady.IsBound())
	{
		OnAbilitySystemReady.Broadcast();
	}
	
	//InitTagPropertyMap(ASC); // TODO
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMPShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UMPShooterInputComponent* MPShooterInputComponent = Cast<UMPShooterInputComponent>(PlayerInputComponent))
	{
		if (PawnData && PawnData->InputConfig)
		{
			TArray<uint32> BindHandles;
			MPShooterInputComponent->BindAbilityActions(PawnData->InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagHeld ,&ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
		}

		MPShooterInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMPShooterCharacter::Move);
		MPShooterInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMPShooterCharacter::Look);
	}
}

UAbilitySystemComponent* AMPShooterCharacter::GetAbilitySystemComponent() const
{
	const AMPShooterPlayerState* MPPlayerState = GetPlayerState<AMPShooterPlayerState>();
	return MPPlayerState ? MPPlayerState->GetAbilitySystemComponent() : nullptr;
}

void AMPShooterCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UMPShooterAbilitySystemComponent* ASC = Cast<UMPShooterAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->AbilityInputTagPressed(InputTag);
	}
}

void AMPShooterCharacter::Input_AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (UMPShooterAbilitySystemComponent* ASC = Cast<UMPShooterAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->AbilityInputTagHeld(InputTag);
	}
}

void AMPShooterCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UMPShooterAbilitySystemComponent* ASC = Cast<UMPShooterAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->AbilityInputTagReleased(InputTag);
	}
}

void AMPShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMPShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}