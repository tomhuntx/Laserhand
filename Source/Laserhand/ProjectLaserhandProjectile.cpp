// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectLaserhandProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Enemy.h"
#include "ProjectLaserhandCharacter.h"
#include "Laserhand_GameMode.h"

AProjectLaserhandProjectile::AProjectLaserhandProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectLaserhandProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	CollisionCompRef = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AProjectLaserhandProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		// Player projectiles (shooting enemies)
		if (targetTag == "Enemy") {
			if (OtherActor->ActorHasTag(targetTag))
			{
				AEnemy* enemy = Cast<AEnemy>(OtherActor);

				if (!enemy->IsPendingKill()) {
					enemy->DamageTarget(ProjDamage, GetActorTransform());
				}
			}

			if (OtherComp->IsSimulatingPhysics()) {
				OtherComp->AddImpulseAtLocation(GetVelocity() * ProjKnockback, GetActorLocation());
			}
		}
		// Enemy projectiles (shooting the player)
		else if (targetTag == "Player") {
			if (OtherActor->ActorHasTag(targetTag))
			{
				AProjectLaserhandCharacter* player = Cast<AProjectLaserhandCharacter>(OtherActor);

				if (!player->IsPendingKill()) {
					player->DamagePlayer(ProjDamage, GetActorLocation());
				}
			}
		}
	}

	// Cue hit event
	GenericHit();
}