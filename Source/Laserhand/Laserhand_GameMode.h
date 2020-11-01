// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Laserhand_GameMode.generated.h"

/**
 * Game Mode Base for Main Game
 * Spawns enemies and tracks score 
 */
UCLASS()
class PROJECTLASERHAND_API ALaserhand_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALaserhand_GameMode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay();


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SpawnEnemies();

	void SpawnPowerups();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameModeType")
		bool HalfMode;

	UPROPERTY(EditAnywhere, Category = "Spawning")
		float SpawnRange;

	// 3 by default
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
		float Spawn_Time_Min;

	// 5 by default
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
		float Spawn_Time_Max;

	// Rate of spawn speed increase 0.005 by default
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
		float SpawnSpeedRate;

	// When boss should start spawning: 0.75 by default
	UPROPERTY(EditAnywhere, Category = "Spawning")
		float GroupLimit;

	// Chance of boss spawning (0.2 = 20%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
		float GroupChance;

	// Amount of spawns that are prevented after boss is spawned
	UPROPERTY(EditAnywhere, Category = "Spawning")
		int GroupSpawnPause;

	// When boss should start spawning: 0.75 by default
	UPROPERTY(EditAnywhere, Category = "Spawning")
		float BossLimit;

	// Chance of boss spawning (0.2 = 20%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
		float BossChance;

	// Amount of spawns that are prevented after boss is spawned
	UPROPERTY(EditAnywhere, Category = "Spawning")
		int BossSpawnPause;

	// Chance of powerup spawning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
		float PowerupChance;

	// When powerup should start spawning: 0.8 by default
	UPROPERTY(EditAnywhere, Category = "Spawning")
		float PowerupLimit;

	// Amount of spawns that are prevented after powerup is spawned
	UPROPERTY(EditAnywhere, Category = "Spawning")
		int PowerupSpawnPause;

	FTimerHandle SpawnTimerHandle;
	FTimerHandle PowerupTimerHandle;

	UPROPERTY(EditAnywhere)
		TSubclassOf<APawn> Enemy;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> EnemyGroup;

	UPROPERTY(EditAnywhere)
		TSubclassOf<APawn> EnemyBoss;

	UPROPERTY(EditAnywhere)
		TSubclassOf<APawn> HealPowerup;

	UPROPERTY(EditAnywhere)
		TSubclassOf<APawn> TimePowerup;

	UPROPERTY(EditAnywhere)
		TSubclassOf<APawn> RapidPowerup;

	// Current spawn speed
	float Spawn_Time_Threshold;

	// Spawns stopped after boss spawns
	int SpawnPause;

	// Spawns stopped after powerup is spawned
	int PowerupPause;

	// Whether or not to spawn a boss this spawn
	bool SpawnBoss;

	// Whether or not to spawn a group this spawn
	bool SpawnGroup;
};
