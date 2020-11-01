// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjectLaserhandCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AProjectLaserhandCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

public:
	AProjectLaserhandCharacter();

protected:
	virtual void BeginPlay();

public:
	// Maximum health of player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game)
		float MaxHealth;

	// Current health of player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Game)
		float CurrentHealth;

	// Current health of player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Game)
		float RegenSpeed;

	// Delay between laser shots
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game)
		float LaserDelay;

	// If player is dead
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game)
		bool isDead;

	// Player points
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game)
		float Points;

	// Player damage (default = 50)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Game)
		float CurrentDamage;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Gun muzzle's rotation offset from current firing point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FRotator GunRotOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Projectile)
	TSubclassOf<class AProjectLaserhandProjectile> ProjectileClass;

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		TSubclassOf<class AProjectLaserhandProjectile> ProjectileClassRegular;

	/** Projectile class to spawn with powerup */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		TSubclassOf<class AProjectLaserhandProjectile> RapidProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

	/** Gun Damage */
	UPROPERTY(EditAnywhere, Category = "Gun")
		float BurstDamage;

	/** Gun Spread */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
		int Spread;

	/** Fires a projectile. */
	UFUNCTION(BlueprintCallable)
		void OnFire();

	// Damages the target by damage
	void DamagePlayer(float Damage, FVector ProjLocation);

	// Destroy the target when its health is 0
	void GameOver();

	// Damage Indicator function implemented in BP
	UFUNCTION(BlueprintImplementableEvent)
		void SpawnIndicator(FVector HitLocation);

	// Move cursor when shooting
	UFUNCTION(BlueprintImplementableEvent)
		void ShootEffect();

	// Reaction to getting hit
	UFUNCTION(BlueprintImplementableEvent)
		void TookDamage();

	// Shoot delay timer
	FTimerHandle SpawnTimerHandle;

	void TimerDelaySwitch();

	float CanShoot;

protected:

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

	float HealTimer;
	float HealTime;

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};

