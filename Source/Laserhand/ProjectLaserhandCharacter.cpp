// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectLaserhandCharacter.h"
#include "ProjectLaserhandProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "DrawDebugHelpers.h"
#include "Target.h"
#include "Enemy.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AProjectLaserhandCharacter

AProjectLaserhandCharacter::AProjectLaserhandCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	//FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.0f, 20.0f, 10.0f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Heal variables
	HealTimer = 0.0f;
	HealTime = 0.1f;

	CanShoot = true;

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AProjectLaserhandCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AProjectLaserhandCharacter::TimerDelaySwitch,
		LaserDelay, false);
}

void AProjectLaserhandCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Heal player by regen speed every 0.2 seconds
	HealTimer += FApp::GetDeltaTime();
	if (HealTimer > HealTime) {
		HealTimer = 0;

		if (CurrentHealth < MaxHealth) {
			CurrentHealth += RegenSpeed;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProjectLaserhandCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AProjectLaserhandCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AProjectLaserhandCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AProjectLaserhandCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProjectLaserhandCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProjectLaserhandCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProjectLaserhandCharacter::LookUpAtRate);
}

void AProjectLaserhandCharacter::OnFire()
{
	if (CanShoot) {

		// try and fire a projectile
		if (ProjectileClass != NULL)
		{
			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				/* Raycast Shooting (DEPRECATED)
				FHitResult* HitResult = new FHitResult();
				FVector StartTrace = FP_MuzzleLocation->GetComponentLocation();
				FVector Forward = FirstPersonCameraComponent->GetForwardVector();
				float randomX = FMath::RandRange(-0.001f * Spread, 0.001f * Spread);
				float randomY = FMath::RandRange(-0.001f * Spread, 0.001f * Spread);
				Forward += FirstPersonCameraComponent->GetRightVector() * randomX;
				Forward += FirstPersonCameraComponent->GetUpVector() * randomY;
				FVector EndTrace = ((Forward) * 3000) + StartTrace;
				FCollisionQueryParams* TraceParams = new FCollisionQueryParams();
				*/

				FRotator SpawnRotation = GetControlRotation() + GunRotOffset;

				float axisSpread = 0.001f * Spread;

				FRotator SpreadOffset = FRotator(FMath::FRandRange(-axisSpread, axisSpread), FMath::FRandRange(-axisSpread, axisSpread), 
					FMath::FRandRange(-axisSpread, axisSpread));

				SpawnRotation += SpreadOffset;

				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				//ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AProjectLaserhandProjectile* proj = World->SpawnActor<AProjectLaserhandProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

				// Set the damage of the projectile
				proj->ProjDamage = CurrentDamage;

				/* Kept for potential future VR purposes
				if (bUsingMotionControllers)
				{
					const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
					const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
					World->SpawnActor<AProjectLaserhandProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
				}
				else
				{

					const FRotator SpawnRotation = GetControlRotation();
					// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
					const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AProjectLaserhandProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
				}
				*/
			}
		}

		// try and play the sound if specified
		if (FireSound != NULL)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// try and play a firing animation if specified
		if (FireAnimation != NULL)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
			if (AnimInstance != NULL)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
			}
		}

		ShootEffect();

		CanShoot = false;
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AProjectLaserhandCharacter::TimerDelaySwitch,
			LaserDelay, false);
	}
}

void AProjectLaserhandCharacter::TimerDelaySwitch()
{
	CanShoot = true;
}

void AProjectLaserhandCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AProjectLaserhandCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AProjectLaserhandCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}


//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AProjectLaserhandCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AProjectLaserhandCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AProjectLaserhandCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AProjectLaserhandCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProjectLaserhandCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AProjectLaserhandCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AProjectLaserhandCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AProjectLaserhandCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AProjectLaserhandCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

void AProjectLaserhandCharacter::DamagePlayer(float Damage, FVector ProjLocation)
{
	CurrentHealth -= Damage;

	TookDamage();

	SpawnIndicator(ProjLocation);

	if (CurrentHealth <= 0) {
		GameOver();
	}
}

void AProjectLaserhandCharacter::GameOver()
{
}