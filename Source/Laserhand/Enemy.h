// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class PROJECTLASERHAND_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* EnemyMesh;

	// health of target
	UPROPERTY(EditAnywhere, Category = Stats)
		float Health;

	// Player points on kill
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game)
		float KillPoints;

	// Death FX function implemented in BP
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void OnDeath();

	// Damage FX function implemented in BP
	UFUNCTION(BlueprintImplementableEvent)
		void OnDamage(FTransform ProjTransform);

	// damages the cursor target by damage
	void DamageTarget(float Damage, FTransform ProjTransform);

	// destroy the target when its health is 0
	UFUNCTION(BlueprintCallable)
		void DestroyTarget();

	// Give the player some points
	void GivePlayerPoints(float points);
};
