// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectLaserhandProjectile.generated.h"

UCLASS(config=Game)
class AProjectLaserhandProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

public:
	AProjectLaserhandProjectile();

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile)
		class USphereComponent* CollisionCompRef;

	// damage of the projectile
	UPROPERTY(EditAnywhere, Category = Damage)
		float ProjDamage;

	// knockback force of player projectile (default=300)
	UPROPERTY(EditAnywhere, Category = Damage)
		float ProjKnockback;

	// target's collider tag 
	UPROPERTY(EditAnywhere, Category = Damage)
		FName targetTag;

	// Generic hit effect
	UFUNCTION(BlueprintImplementableEvent)
		void GenericHit();

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};

