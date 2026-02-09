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
#include "Actor/WeaponActor.h"
#include "AbilitySystemComponent.h"
#include "AncientKingCharacter.h"
#include "AbilitySystem/AbilityId.h"
#include "AbilitySystem/GameplayAbility/ComboAttackGameplayAbility.h"
#include "EngineUtils.h"
#include "GameData/BeadurincPlayerState.h"

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
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bLockingOnCamera = false;
}

/** On character join a world */
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HealthAttributeSet)
	{
		// Bind the delegate to HandleHealthChange
		HealthAttributeSet->OnHealthChanged.AddDynamic(this, &APlayerCharacter::HandleHealthChange);
	}
	
	// Spawn a weapon actor and attach to the hand
	if (GetMesh() && MainHandWeapon)
	{
		// Instigator != Owner under certain cases
		// e.g. Player shots an arrow using bow: Arrow's owner == bow, but arrow's instigator == player
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = this;
		ActorSpawnParameters.Instigator = GetInstigator();
		
		if (AWeaponActor* WeaponActor = GetWorld()->SpawnActor<AWeaponActor>(MainHandWeapon, ActorSpawnParameters))
		{
			// Initialize main hand weapon actor
			MainHandWeaponActor = WeaponActor;
			
			WeaponActor->SetActorEnableCollision(false);
			
			WeaponActor->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				TEXT("HandGrip_R")
			);
		}
	}
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bLockingOnCamera)
	{
		// When locking on character no longer valid, unlock the camera 
		if (!IsValid(LockingOnCharacter))
		{
			UnlockCamera();
		}
		else
		{
			// Update rotation of the camera to look at the target
			UpdateCameraLock(DeltaSeconds);
		}
	}
}

/** Called when a controller takes control of this character */
void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (TObjectPtr<ABeadurincPlayerState> BeadurincPlayerState = Cast<ABeadurincPlayerState>(GetPlayerState()))
	{
		AbilitySystemComponent = BeadurincPlayerState->GetAbilitySystemComponent();
		
		// Clear old abilities
		AbilitySystemComponent->ClearAllAbilities();
		
		// Reset ACS owner
		AbilitySystemComponent->InitAbilityActorInfo(BeadurincPlayerState, this);
		
		// Initialize Health Attribute
		AbilitySystemComponent->SetNumericAttributeBase(UPlayerAttributeSet::GetHealthAttribute(), InitialHealth);
		
		// Give combo attack ability
		if (IsValid(ComboAttackAbility))
		{
			FGameplayAbilitySpec ComboAttackAbilitySpec(ComboAttackAbility, 1, static_cast<int32>(EAbilityId::Combo_Attack), this);
			ComboAbilitySpecHandler = AbilitySystemComponent->GiveAbility(ComboAttackAbilitySpec);
		}
		
		// Give block ability
		if (IsValid(BlockAbility))
		{
			FGameplayAbilitySpec BlockAbilitySpec(BlockAbility, 1, static_cast<int32>(EAbilityId::Block), this);
			BlockAbilitySpecHandler = AbilitySystemComponent->GiveAbility(BlockAbilitySpec);
		}
		
		// Give roll ability
		if (IsValid(RollAbility))
		{
			FGameplayAbilitySpec RollAbilitySpec(RollAbility, 1, static_cast<int32>(EAbilityId::Roll), this);
			RollAbilitySpecHandler = AbilitySystemComponent->GiveAbility(RollAbilitySpec);
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
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		
		// Camera lock
		EnhancedInputComponent->BindAction(CameraLockAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleCamLock);
		
		// Combo Attack Ability
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::PressAbility, static_cast<int32>(EAbilityId::Combo_Attack));
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::ReleaseAbility, static_cast<int32>(EAbilityId::Combo_Attack));
		
		// Blocking Ability
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started, this, &APlayerCharacter::PressAbility, static_cast<int32>(EAbilityId::Block));
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
		
		if (!PC)
		{
			return;
		}
		
		// Store the viewport size
		int32 ViewX, ViewY;
		PC->GetViewportSize(ViewX, ViewY);
		
		ACharacter* ClosestActorFromCrosshair = nullptr;
		double ClosestDistance = 1000000000.0;
		
		// Iterates all actors
		for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
		{
			ACharacter* Character = *It;
			
			// Skip the character that is invalid or if it's myself
			if (!IsValid(Character) || Character == this)
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
			const double DistanceFromCrosshair = ActorLocationInScreen.SquaredLength();
			
			// Calculate the distance between center of the screen and an actor, if true, memorize the actor and distance
			if (DistanceFromCrosshair < ClosestDistance)
			{
				ClosestActorFromCrosshair = Character;
				ClosestDistance = DistanceFromCrosshair;
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
	if (GetController() != nullptr)
	{
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
}

void APlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw * (bLockingOnCamera ? 0.05F : 1.0F));
		AddControllerPitchInput(Pitch * (bLockingOnCamera ? 0.05F : 1.0F));
	}
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
	if (HasBufferedInput())
	{
		ClearInputBuffer();
	}
	
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

void APlayerCharacter::HandleHealthChange(float Magnitude, float NewHealth)
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		3.0F,
		FColor::Red,
		FString::Printf(
			TEXT("Magnitude: %f, NewHealth: %f"),
			Magnitude, NewHealth
		)
	);
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
	if (IsLocallyControlled() && HasBufferedInput())
	{
		// Try activating the buffered input if it entered within the buffer window
		if (GetWorld()->GetTimeSeconds() - BufferedInput->TimeStamp <= BUFFER_WINDOW_SECONDS)
		{
			const int32 InputID = BufferedInput->InputID;
			
			// On input buffering activation, we do not care about whether it success activating
			AbilitySystemComponent->AbilityLocalInputPressed(InputID);
			
			// Clear the input buffer
			// This results in clearing any buffered input via GameplayAbility#InputPressed, which is
			// the intention of the system, so input buffering won't create another input buffering again.
			// @see UComboAttackGameplayAbility::InputPressed
			ClearInputBuffer();
		}
	}
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
	
	// Set camera rotation
	FVector CameraLoc = PC->PlayerCameraManager->GetCameraLocation();
	
	// Add the half height of the target's collider to aim at the center of the target
	FVector TargetLoc =	LockingOnCharacter->GetActorLocation();
	TargetLoc.Y += LockingOnCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.3;

	// Calculate rotator from vector camera -> target
	FRotator DesiredRot = (TargetLoc - CameraLoc).Rotation();
	FRotator CurrentRot = PC->GetControlRotation();
	
	// Get interpolated rotator by current rotation -> desired rotation
	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, 10.f);
	
	// Set to the calculated rotation
	PC->SetControlRotation(NewRot);
	
	// Set camera rotation with the same principle
	FVector ActorLoc = GetActorLocation();
	FRotator DesiredActorRot = (TargetLoc - ActorLoc).Rotation();
	FRotator CurrentActorRot = GetActorRotation();
	FRotator NewActorRot = FMath::RInterpTo(CurrentActorRot, DesiredActorRot, DeltaTime, 10.f);
	
	// Do not rotate pitch
	NewActorRot.Pitch = CurrentActorRot.Pitch;
	
	SetActorRotation(NewActorRot);
}

void APlayerCharacter::LockCamera(ACharacter* Target)
{
	if (!IsValid(Target))
	{
		return;
	}
	
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bLockingOnCamera = true;
	LockingOnCharacter = Target;
}

void APlayerCharacter::UnlockCamera()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bLockingOnCamera = false;
	LockingOnCharacter = nullptr;
}
