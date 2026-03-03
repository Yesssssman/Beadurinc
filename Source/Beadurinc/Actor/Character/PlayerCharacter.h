// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FighterCharacter.h"
#include "AbilitySystem/AttributeSet/LivingAttributeSet.h"
#include "Logging/LogMacros.h"
#include "PlayerCharacter.generated.h"

// Allowed input buffering's lifetime. this means
// only buffered inputs entered before 0.5 seconds are
// fired when the player becomes available state.
#define BUFFER_WINDOW_SECONDS 0.25

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FGameplayAbilitySpecHandle;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

typedef struct FBufferedInput
{
	int32 InputID;
	double TimeStamp;
} FBufferedInput;

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class APlayerCharacter : public AFighterCharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;
	
	/** Camera Lock Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CameraLockAction;
	
	/** Combo Attack Ability Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ComboAttackAction;
	
	/** Sword Blocking Ability Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* BlockAction;
	
	/** Rolling Ability Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* RollAction;

protected:
	
	/** Called when a new controller takes control of this character */
	virtual void PossessedBy(AController* NewController) override;
	
	/** On player state replicated in client side */
	virtual void OnRep_PlayerState() override;
	
private:
	
	/** Used by input buffering system */
	TOptional<FBufferedInput> BufferedInput;
	
	/** A character being locked on by the player */
	TObjectPtr<ACharacter> LockingOnCharacter;
	
	/** Used by camera lock on system */
	bool bLockingOnCamera;
	
public:

	/** Constructor */
	APlayerCharacter();	

protected:

	/** On every tick in a world */
	virtual void Tick(float DeltaSeconds) override;
	
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	/** Called for camera lock input */
	void ToggleCamLock(const FInputActionValue& Value);

	/** On pressed GAS ability input key */
	void PressAbility(int32 AbilityID);

	/** On released GAS ability input key */
	void ReleaseAbility(int32 InputId);

public:
	
	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	
	/** Buffer an ability input by InputID */
	virtual void BufferInput(int32 InputID);
	
	/** Tryna activate buffered input and flush the buffer */
	virtual void FlushBufferedInput();
	
	/** Clear Input buffer */
	virtual void ClearInputBuffer();
	
	/** Checks if any buffered input exist */
	virtual bool HasBufferedInput();
	
public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	/** Updates camera rotation to align a target to crosshair */
	void UpdateCameraLock(float DeltaTime);
	
	/** Locks the camera to given target */
	void LockCamera(ACharacter* Target);
	
	/** Unlocks the camera if locked */
	void UnlockCamera();
	
};

