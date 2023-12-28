// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPShooterGameMode.h"
#include "UObject/ConstructorHelpers.h"

AMPShooterGameMode::AMPShooterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/Player/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
