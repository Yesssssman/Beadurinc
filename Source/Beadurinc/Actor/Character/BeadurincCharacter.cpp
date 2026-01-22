// Copyright Epic Games, Inc. All Rights Reserved.

#include "BeadurincCharacter.h"
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
#include "Beadurinc.h"
#include "AbilitySystem/AbilityId.h"
#include "AbilitySystem/GameplayAbility/ComboAttackGameplayAbility.h"
#include "GameData/BeadurincPlayerState.h"

/** Constructor */
ABeadurincCharacter::ABeadurincCharacter()
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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

/** On character join a world */
void ABeadurincCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HealthAttributeSet)
	{
		// Bind the delegate to HandleHealthChange
		HealthAttributeSet->OnHealthChanged.AddDynamic(this, &ABeadurincCharacter::HandleHealthChange);
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

/** Called when a controller takes control of this character */
void ABeadurincCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (TObjectPtr<ABeadurincPlayerState> BeadurincPlayerState = Cast<ABeadurincPlayerState>(GetPlayerState()))
	{
		AbilitySystemComponent = BeadurincPlayerState->GetAbilitySystemComponent();
		
		// Reset ACS owner
		AbilitySystemComponent->InitAbilityActorInfo(BeadurincPlayerState, this);
		
		// Initialize Health Attribute
		AbilitySystemComponent->SetNumericAttributeBase(UPlayerAttributeSet::GetHealthAttribute(), InitialHealth);
		
		// Give Combo Attack Ability
		FGameplayAbilitySpec AbilitySpec(ComboAttackAbility, 1, EAbilityId::Combo_Attack, this);
		
		// Return Spec Handler to use outside
		ComboAbilitySpecHandler = AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

void ABeadurincCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABeadurincCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ABeadurincCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABeadurincCharacter::Look);
		
		// triggering & releasing GAS
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &ABeadurincCharacter::PressAbility, static_cast<int32>(EAbilityId::Combo_Attack));
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Completed, this, &ABeadurincCharacter::ReleaseAbility, static_cast<int32>(EAbilityId::Combo_Attack));
	}
	else
	{
		UE_LOG(LogBeadurinc, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ABeadurincCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ABeadurincCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ABeadurincCharacter::DoMove(float Right, float Forward)
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

void ABeadurincCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ABeadurincCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ABeadurincCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ABeadurincCharacter::PressAbility(int32 InputId)
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

void ABeadurincCharacter::ReleaseAbility(int32 InputId)
{
	AbilitySystemComponent->AbilityLocalInputReleased(InputId);
}

void ABeadurincCharacter::HandleHealthChange(float Magnitude, float NewHealth)
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
void ABeadurincCharacter::BufferInput(int32 InputID)
{
	BufferedInput = {InputID, GetWorld()->GetTimeSeconds()};
}

/** Tryna activate buffered input and flush the buffer */
void ABeadurincCharacter::TriggerBufferedInput()
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
void ABeadurincCharacter::ClearInputBuffer()
{
	// Clear the buffer
	BufferedInput.Reset();
}

/** Checks if any buffered input exist */
bool ABeadurincCharacter::HasBufferedInput()
{
	return BufferedInput.IsSet();
}