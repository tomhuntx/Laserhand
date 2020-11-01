// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "ProjectLaserhandCharacter.h"
#include "Laserhand_GameMode.h"
#include <Runtime\Engine\Classes\Kismet\GameplayStatics.h>

// Sets default values
AEnemy::AEnemy()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup Mesh"));
	//RootComponent = TargetMesh;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::DamageTarget(float Damage, FTransform ProjTransform)
{
	Health -= Damage;

	// Gain a few points with each hit
	// Get player character
	GivePlayerPoints(20);

	OnDamage(ProjTransform);

	if (Health <= 0) {
		DestroyTarget();
	}
}

void AEnemy::DestroyTarget()
{
	GivePlayerPoints(KillPoints);

	OnDeath();

	Destroy();
}

void AEnemy::GivePlayerPoints(float points) {
	AProjectLaserhandCharacter* player = Cast<AProjectLaserhandCharacter>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	ALaserhand_GameMode* gm = Cast<ALaserhand_GameMode>(
		UGameplayStatics::GetGameMode(GetWorld()));
	
	// Half points if game mode is half mode
	if (gm != NULL && gm->HalfMode) {
		points /= 2.0f;
	}

	// Give points to player
	if (player != NULL) {
		player->Points += points;
	}
}
