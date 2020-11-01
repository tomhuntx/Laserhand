// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectLaserhandGameMode.h"
#include "ProjectLaserhandHUD.h"
#include "ProjectLaserhandCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectLaserhandGameMode::AProjectLaserhandGameMode() : Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AProjectLaserhandHUD::StaticClass();
}
