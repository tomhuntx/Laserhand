 // Fill out your copyright notice in the Description page of Project Settings.


#include "Target.h"

// Sets default values
ATarget::ATarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Health = 100;

	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup Mesh"));
	//RootComponent = TargetMesh;
}

// Called when the game starts or when spawned
void ATarget::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATarget::DamageTarget(float Damage)
{
	Health -= Damage;

	if (Health <= 0) {
		DestroyTarget();
	}
}

void ATarget::DestroyTarget()
{

	Destroy();
}

