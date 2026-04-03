// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"	
#include "AbilitySystemComponent.h"
#include "AncientKingCharacter.h"
#include "AbilitySystem/AbilityId.h"
#include "MotionWarpingComponent.h"
#include "GameData/BeadurincPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

/** Constructor */
APlayerCharacter::APlayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 240.0F;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	LockOnDistance = 2000.0;
	
	bLockingOnCamera = false;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Remove actor rotation, to get a local space location.
	FVector CameraLocal = GetActorRotation().UnrotateVector(FollowCamera->GetComponentLocation() - GetActorLocation());
	
	// Save the initial camera local location
	InitLocalCameraLocation = CameraLocal;
	
	// Remove the backward translation
	// The calculation of the rotation for camera lock-on should be based on the center of the camera orbit, not the
	// final location of relative camera position in world space coord.
	//
	// By removing the spring arm length, we can get the center of the circle that the camera draws. This point will
	// calculate the correct rotation to locate the lock-on destination point to the center of the screen applied to
	// the spring arm.
	InitLocalCameraLocation.X = 0;
}

// Handle server side respawn
void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (ABeadurincPlayerState* PS = GetPlayerState<ABeadurincPlayerState>())
	{
		// Init ASC object holder from PlayerState to minimize nested access for it
		AbilitySystemComponent = PS->GetAbilitySystemComponent();
        
		// Set the Owner to the PlayerState, and the Avatar to this Character
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
		
		// Reset stat values to their maximum. Only needs to be called in authorized side
		// so they automatically be handled by GAS networking.
		PS->ResetStats();
	}
}

// Handle client side respawn
void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (ABeadurincPlayerState* PS = GetPlayerState<ABeadurincPlayerState>())
	{
		// Extract ASC and AttributeSet from PlayerState to minimize nested accessing
		AbilitySystemComponent = PS->GetAbilitySystemComponent();
		AttributeSet = PS->GetAttributeSet();
		
		// Set owner as Player State since PlayerCharacter is transient object. (Destroyed on death)
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}
}

/** Returns whether current locking target is valid */
bool APlayerCharacter::IsValidLockOnTarget(const ACharacter* Target) const
{
	// Invalid (object destroyed or pending kill)
	if (!IsValid(Target))
	{
		return false;
	}
	
	double DistanceSqr = (GetActorLocation() - Target->GetActorLocation()).SquaredLength();
	
	// Distance too far
	if (DistanceSqr > LockOnDistance * LockOnDistance)
	{
		return false;
	}
	
	return true;
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bLockingOnCamera)
	{
		// When locking on character no longer valid, unlock the camera
		// Since check for object pointer is conducted in each tick we can ignore
		// the error message, "Object member 'LockingOnCharacter' can be destroyed
		// during garbage collection, resulting in a stale pointer"
		if (!IsValidLockOnTarget(LockingOnCharacter))
		{
			UnlockCamera();
		}
		else
		{
			// Update rotation of the camera to look at the target
			UpdateCameraLock(DeltaSeconds);
		}
	}
	
	if (bRunning)
	{
		FVector Velocity = GetVelocity();
		
		// Reset running state and max walk speed if current velocity is under movement threshold
		if (Velocity.SquaredLength() < 0.025F)
		{
			GetCharacterMovement()->MaxWalkSpeed = 200.0F;
			bRunning = false;
		}
		
		// Normalize the velocity before calculating dot product
		Velocity.Normalize(0.05);
		
		// Prevents the player running backward
		if (FVector::DotProduct(GetActorForwardVector(), Velocity) <= -0.5F)
		{
			GetCharacterMovement()->MaxWalkSpeed = 200.0F;
			bRunning = false;
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &APlayerCharacter::Run);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		
		// Camera lock
		EnhancedInputComponent->BindAction(CameraLockAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleCamLock);
		
		// Combo Attack Ability
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::PressAbility, static_cast<int32>(EAbilityId::Combo_Attack));
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::ReleaseAbility, static_cast<int32>(EAbilityId::Combo_Attack));
		
		// Blocking Ability
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered, this, &APlayerCharacter::PressAbility, static_cast<int32>(EAbilityId::Block));
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &APlayerCharacter::ReleaseAbility, static_cast<int32>(EAbilityId::Block));
		
		// Rolling Ability
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &APlayerCharacter::PressAbility, static_cast<int32>(EAbilityId::Roll));
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void APlayerCharacter::Run(const FInputActionValue& Value)
{
	// assign new max walk speed based on running state
	GetCharacterMovement()->MaxWalkSpeed = bRunning ? 200.0F : 500.0F;
	
	// toggle running state
	bRunning = !bRunning;
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void APlayerCharacter::ToggleCamLock(const FInputActionValue& Value)
{
	if (!bLockingOnCamera)
	{
		// To get a local player's controller, pass 0 to PlayerIndex
		const APlayerController* PC = Cast<APlayerController>(GetController());
		
		if (!PC) return;
		
		// Store the viewport size
		int32 ViewX, ViewY;
		PC->GetViewportSize(ViewX, ViewY);
		
		ACharacter* ClosestActorFromCrosshair = nullptr;
		double ClosestDistanceOnScreenSqr = 1000000000.0;
		
		// Define the types to look for
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
		
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);
		TArray<AActor*> ActorsWithinBound;
		
		// Only gets actors within preset distance
		UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(), 
			GetActorLocation(), 
			LockOnDistance,
			ObjectTypes, 
			ACharacter::StaticClass(), 
			IgnoreActors, 
			ActorsWithinBound
		);
		
		// Iterates all actors
		for (AActor* Actor : ActorsWithinBound)
		{
			ACharacter* Character = Cast<ACharacter>(Actor);
			
			// Skip the character that is invalid or if it's myself
			if (!IsValidLockOnTarget(Character) || Character == this)
			{
				continue;
			}
			
			FVector2D ActorLocationInScreen;
			
			// Get a x, y coordinate of an actor in the screen 
			const bool ConvertSucceeded = PC->ProjectWorldLocationToScreen(
				Character->GetActorLocation(),
				ActorLocationInScreen
			);
			
			// A case that the target is not on the screen
			if (!ConvertSucceeded)
			{
				continue;
			}
			
			// Transform the left-top coord system into center-center
			ActorLocationInScreen.X -= ViewX / 2;
			ActorLocationInScreen.Y -= ViewY / 2;
			
			// Calculates distance
			const double DistanceFromCrosshairSqr = ActorLocationInScreen.SquaredLength();
			
			// Calculate the distance between center of the screen and an actor, if true, memorize the actor and distance
			if (DistanceFromCrosshairSqr < ClosestDistanceOnScreenSqr)
			{
				ClosestActorFromCrosshair = Character;
				ClosestDistanceOnScreenSqr = DistanceFromCrosshairSqr;
			}
		}
		
		// If found any nearest target, set locking true
		if (ClosestActorFromCrosshair != nullptr)
		{
			LockCamera(ClosestActorFromCrosshair);
			return;
		}
	}
	
	UnlockCamera();
}

void APlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController() == nullptr) return;
	
	// find out which way is forward
	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	AddMovementInput(ForwardDirection, Forward);
	AddMovementInput(RightDirection, Right);
}

void APlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() == nullptr) return;
	
	// add yaw and pitch input to controller
	AddControllerYawInput(Yaw * (bLockingOnCamera ? 0.05F : 1.0F));
	AddControllerPitchInput(Pitch * (bLockingOnCamera ? 0.05F : 1.0F));
}

void APlayerCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void APlayerCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void APlayerCharacter::PressAbility(int32 InputId)
{
	// Clear existing Buffered Input to prevent double execution
	if (HasBufferedInput()) ClearInputBuffer();
	
	if (FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromInputID(InputId))
	{
		if (Spec->IsActive())
		{
			// Trigger InputPressed in GA
			AbilitySystemComponent->AbilitySpecInputPressed(*Spec);
		}
		else
		{
			if (!AbilitySystemComponent->TryActivateAbility(Spec->Handle))
			{
				// Buffer the latest input if it failed in activating ability
				BufferInput(InputId);
			}
		}
	}
}

void APlayerCharacter::ReleaseAbility(int32 InputId)
{
	AbilitySystemComponent->AbilityLocalInputReleased(InputId);
}

/** Buffer an ability input by InputID */
void APlayerCharacter::BufferInput(int32 InputID)
{
	BufferedInput = {InputID, GetWorld()->GetTimeSeconds()};
}

/** Tryna activate buffered input and flush the buffer */
void APlayerCharacter::FlushBufferedInput()
{
	// Checks buffered inputs in local client (to avoid unnecessary call)
	if (!IsLocallyControlled() || !HasBufferedInput()) return;
	
	// Try activating the buffered input if it entered within the buffer window
	if (GetWorld()->GetTimeSeconds() - BufferedInput->TimeStamp > BUFFER_WINDOW_SECONDS) return;
	
	const int32 InputID = BufferedInput->InputID;
	
	// On input buffering activation, we do not care about whether it succeed in activating
	AbilitySystemComponent->AbilityLocalInputPressed(InputID);
	
	// Clear the input buffer
	// This results in clearing any buffered input via GameplayAbility#InputPressed, which is
	// the intention of the system, so input buffering won't create another input buffering again.
	// @see UComboAttackGameplayAbility::InputPressed
	ClearInputBuffer();
}

/** Clear Input buffer */
void APlayerCharacter::ClearInputBuffer()
{
	// Clear the buffer
	BufferedInput.Reset();
}

/** Checks if any buffered input exist */
bool APlayerCharacter::HasBufferedInput()
{
	return BufferedInput.IsSet();
}

void APlayerCharacter::UpdateCameraLock(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// Sets actor rotation
	// Decoupled from camera rotation since actor eye position is different from camera position
	// Calculates in the same way, using (target - eye).Rotation()
	FRotator DesiredActorRot = (LockingOnCharacter->GetActorLocation() - GetActorLocation()).Rotation();
	FRotator CurrentActorRot = GetActorRotation();
	FRotator NewActorRot = FMath::RInterpTo(CurrentActorRot, DesiredActorRot, DeltaTime, 8.f);
	
	// Won't rotate in pitch
	NewActorRot.Pitch = CurrentActorRot.Pitch;
	SetActorRotation(NewActorRot);
	
	// Calculate world location of camera initial point
	// Reused 'DesiredActorRot' instead of 'GetActorRot()' so it's not affected by ongoing interpolation
	FVector WorldLocalCamera = DesiredActorRot.RotateVector(InitLocalCameraLocation);
	
	// Get the camera world location where to start calculating lock-on
	FVector LockOnStart = GetActorLocation() + WorldLocalCamera;
	
	// Find a "middle point" of player and target actor 
	FVector TargetLoc =	GetActorLocation() + (LockingOnCharacter->GetActorLocation() - GetActorLocation()) / 2;
	
	// Calculate rotator from vector camera -> target
	FRotator DesiredRot = (TargetLoc - LockOnStart).Rotation();
	FRotator CurrentRot = PC->GetControlRotation();
	
	// Get interpolated rotator by current rotation -> desired rotation
	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, 8.f);
	
	// Set to the calculated rotation
	PC->SetControlRotation(NewRot);
}

void APlayerCharacter::LockCamera(ACharacter* Target)
{
	if (!IsValid(Target)) return;
	
	GetCharacterMovement()->bOrientRotationToMovement = false;
	
	bLockingOnCamera = true;
	LockingOnCharacter = Target;
}

void APlayerCharacter::UnlockCamera()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bLockingOnCamera = false;
	LockingOnCharacter = nullptr;
	MotionWarpingComponent->RemoveWarpTarget(TEXT("AttackTarget"));
}
