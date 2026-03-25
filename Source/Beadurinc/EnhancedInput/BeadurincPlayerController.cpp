// Copyright Epic Games, Inc. All Rights Reserved.


#include "BeadurincPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Beadurinc.h"
#include "Actor/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/BeadurincHUD.h"
#include "Widgets/Input/SVirtualJoystick.h"

/** Plays a UI sound via RPC (Should be suffixed by `_Implementation` to follow RPC's convention) */
void ABeadurincPlayerController::PlayLocalSound_Implementation(USoundBase* SoundToPlay, const float VolumeModifier)
{
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySound2D(this, SoundToPlay, VolumeModifier);
	}
}

/**
 * Adds a boss stat bar (Should be suffixed by `_Implementation` to follow RPC's convention)
 * @param DisplayedBoss A boss character that will bound to widget
 */
void ABeadurincPlayerController::DisplayBossBar_Implementation(const AFighterCharacter* DisplayedBoss)
{
	if (ABeadurincHUD* BeadurincHUD = Cast<ABeadurincHUD>(GetHUD()))
	{
		BeadurincHUD->AddBossEntry(DisplayedBoss);
	}
}

/**
 * Removes a boss stat bar (Should be suffixed by `_Implementation` to follow RPC's convention)
 * @param DisconnectedBoss A boss character who will be removed from boss bars
 */
void ABeadurincPlayerController::CloseBossBar_Implementation(const AFighterCharacter* DisconnectedBoss)
{
	if (ABeadurincHUD* BeadurincHUD = Cast<ABeadurincHUD>(GetHUD()))
	{
		BeadurincHUD->RemoveBossEntry(DisconnectedBoss);
	}
}

void ABeadurincPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogBeadurinc, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void ABeadurincPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool ABeadurincPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

/** Called on the controller starts possessing player character */
void ABeadurincPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (ABeadurincHUD* BeadurincHUD = Cast<ABeadurincHUD>(GetHUD()))
	{
		if (const TObjectPtr<APlayerCharacter> PlayerCharacter = Cast<APlayerCharacter>(InPawn))
		{
			BeadurincHUD->InitPlayer(PlayerCharacter);
		}
	}
}

/** Called on the controller starts possessing player character (Rep in client) */
void ABeadurincPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	
	if (ABeadurincHUD* BeadurincHUD = Cast<ABeadurincHUD>(GetHUD()))
	{
		if (const TObjectPtr<APlayerCharacter> PlayerCharacter = Cast<APlayerCharacter>(GetOwner()))
		{
			BeadurincHUD->InitPlayer(PlayerCharacter);
		}
	}
}
