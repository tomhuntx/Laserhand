// Fill out your copyright notice in the Description page of Project Settings.


#include "Laserhand_GameMode.h"
#include "ProjectLaserhandHUD.h"
#include "ProjectLaserhandCharacter.h"
#include "Enemy.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

ALaserhand_GameMode::ALaserhand_GameMode() : Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AProjectLaserhandHUD::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	Spawn_Time_Threshold = 1.0f;
	SpawnPause = 0;
	PowerupPause = 0;
}

void ALaserhand_GameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ALaserhand_GameMode::SpawnEnemies,
		FMath::RandRange(Spawn_Time_Min, Spawn_Time_Max), false);

	GetWorldTimerManager().SetTimer(PowerupTimerHandle, this, &ALaserhand_GameMode::SpawnPowerups,
		FMath::RandRange(Spawn_Time_Min, Spawn_Time_Max), false);
}

void ALaserhand_GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Spawn_Time_Threshold > 0.2f) {
		// Decreases time threshold by 0.005 every second
		Spawn_Time_Threshold -= DeltaTime * SpawnSpeedRate;
	}
	else {
		Spawn_Time_Threshold = 0.2f;
	}
}

void ALaserhand_GameMode::SpawnEnemies()
{
	float x = FMath::RandRange(-1, 1) * SpawnRange;
	if (HalfMode) {
		x = FMath::RandRange(-1, 0) * SpawnRange;
	}
	float y = FMath::RandRange(-1, 1) * SpawnRange;;
	float z = FMath::RandRange(-1, 1) * SpawnRange;;
	FVector SpawnPosition = FVector(x,y,z);

	float RandZRotation = FMath::RandRange(0.0f, 360.0f);
	FRotator SpawnRotation = FRotator(0.0f, RandZRotation, 0.0f);

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ALaserhand_GameMode::SpawnEnemies,
		FMath::RandRange(Spawn_Time_Min * Spawn_Time_Threshold, Spawn_Time_Max * Spawn_Time_Threshold), false);

	// Reset spawning
	SpawnBoss = false;
	SpawnGroup = false;

	// Check if should spawn boss
	if (Spawn_Time_Threshold < GroupLimit) {
		// 1 in 5 chance 
		if (FMath::RandRange(0.0f, 1.0f) < GroupChance) {
			SpawnGroup = true;
		}
	}

	// Check if should spawn boss
	if (Spawn_Time_Threshold < BossLimit) {
		// 1 in 5 chance 
		if (FMath::RandRange(0.0f, 1.0f) < BossChance) {
			SpawnBoss = true;
		}
	}

	// Alternate spawning in half mode
	if (HalfMode && SpawnPosition.X > 250.0f) {
		SpawnEnemies();

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Stopped half spawning"));
	}
	else if (SpawnPause > 0) {
		SpawnPause--;
	}
	//Prevent enemies spawning directly below or directly above the player
	else if (SpawnPosition.Equals(FVector(0, 0, -SpawnRange), 2.f)) {
		// Try again if spawn is directly below
		SpawnEnemies();
	}
	else if (SpawnPosition.Equals(FVector(0, 0, SpawnRange), 2.f)) {
		// Try again if directly above
		SpawnEnemies();
	}
	else if (SpawnGroup) {
		// Spawn 3 Bots
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AActor* Bot = GetWorld()->SpawnActor(EnemyGroup.Get(), &SpawnPosition, &SpawnRotation, ActorSpawnParams);

		// Start spawn delay
		SpawnPause = GroupSpawnPause;
	}
	// Spawn boss type enemy
	else if (SpawnBoss) {
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AActor* Bot = GetWorld()->SpawnActor(EnemyBoss.Get(), &SpawnPosition, &SpawnRotation, ActorSpawnParams);

		// Spawn AI Controller
		if (Bot)
		{
			APawn* BotPawn = Cast<APawn>(Bot);
			if (BotPawn)
			{
				BotPawn->SpawnDefaultController();
			}
		}

		// Start spawn delay
		SpawnPause = BossSpawnPause;
	}
	// Otherwise spawn normally
	else {
		// Spawn Bot
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AActor* Bot = GetWorld()->SpawnActor(Enemy.Get(), &SpawnPosition, &SpawnRotation, ActorSpawnParams);

		// Spawn AI Controller
		if (Bot)
		{
			APawn* BotPawn = Cast<APawn>(Bot);
			if (BotPawn)
			{
				BotPawn->SpawnDefaultController();
			}
		}
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Spawned regular"));
}

void ALaserhand_GameMode::SpawnPowerups()
{
	GetWorldTimerManager().SetTimer(PowerupTimerHandle, this, &ALaserhand_GameMode::SpawnPowerups,
		FMath::RandRange(Spawn_Time_Min * Spawn_Time_Threshold, Spawn_Time_Max * Spawn_Time_Threshold), false);

	// Check if should spawn powerup
	if (Spawn_Time_Threshold < PowerupLimit)
	{
		float x = FMath::RandRange(-1.0f, 1.0f) * SpawnRange;
		if (HalfMode) {
			x = FMath::RandRange(-1.0f, 0.0f) * SpawnRange;
		}
		float y = FMath::RandRange(-1.0f, 1.0f) * SpawnRange;;
		float z = FMath::RandRange(-1.0f, 1.0f) * SpawnRange;;
		FVector SpawnPosition = FVector(x, y, z);

		float RandZRotation = FMath::RandRange(0.0f, 360.0f);
		FRotator SpawnRotation = FRotator(0.0f, RandZRotation, 0.0f);

		//Prevent enemies spawning directly below or directly above the player
		if (SpawnPosition.Equals(FVector(0, 0, -SpawnRange), 2500.0f)) {
			// Try again if spawn is directly below
			SpawnPowerups();
		}
		else if (SpawnPosition.Equals(FVector(0, 0, SpawnRange), 1000.0f)) {
			// Try again if directly above
			SpawnPowerups();
		}

		float spawnChance = FMath::RandRange(0.0f, 1.0f);

		// Spawn Powerup
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Spawn heal Powerup
		if (PowerupPause > 0) {
			PowerupPause--;
		}
		else if (FMath::RandRange(0.0f, 1.0f) < PowerupChance) {
			if (spawnChance <= 0.3f) {
				AActor* heal = GetWorld()->SpawnActor(HealPowerup.Get(), &SpawnPosition, &SpawnRotation, ActorSpawnParams);

				// Spawn AI Controller
				if (heal)
				{
					APawn* pawn = Cast<APawn>(heal);
					if (pawn)
					{
						pawn->SpawnDefaultController();
					}
				}
			}
			else if (spawnChance <= 0.7f) {
				AActor* rapid = GetWorld()->SpawnActor(RapidPowerup.Get(), &SpawnPosition, &SpawnRotation, ActorSpawnParams);

				// Spawn AI Controller
				if (rapid)
				{
					APawn* pawn = Cast<APawn>(rapid);
					if (pawn)
					{
						pawn->SpawnDefaultController();
					}
				}
			}
			// Spawn time powerup
			else {
				AActor* time = GetWorld()->SpawnActor(TimePowerup.Get(), &SpawnPosition, &SpawnRotation, ActorSpawnParams);

				// Spawn AI Controller
				if (time)
				{
					APawn* pawn = Cast<APawn>(time);
					if (pawn)
					{
						pawn->SpawnDefaultController();
					}
				}
			}

			// Start spawn delay
			PowerupPause = PowerupSpawnPause;
		}
	}
}