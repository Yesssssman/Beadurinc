#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FighterCharacter.h"
#include "Logging/LogMacros.h"
#include "PlayerCharacter.generated.h"

// Allowed input buffering's lifetime. this means
// only buffered inputs entered before 0.5 seconds are
// fired when the player becomes available state.
#define BUFFER_WINDOW_SECONDS 0.25

class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputComponent;
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

public:

	/** Constructor */
	APlayerCharacter();

protected:

	/** Called on join a level */
	virtual void BeginPlay() override;

	/** On every tick in a world */
	virtual void Tick(float DeltaSeconds) override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Called when a new controller takes control of this character. A sweet spot for handling player respawn in server. */
	virtual void PossessedBy(AController* NewController) override;

	/** On player state replicated in a client side. Sweet spot for handling player respawn in client. */
	virtual void OnRep_PlayerState() override;

	/** Returns whether current locking target is valid */
	bool IsValidLockOnTarget(const ACharacter* Target) const;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for runnig input */
	void Run(const FInputActionValue& Value);

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

	/** Handle notify from nearby attacker's danger attacks (unblockable, like pierce and low attacks) */
	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void AlertDangerAttacks(const FGameplayTag& AttackType);

	/** Buffer an ability input by InputID */
	virtual void BufferInput(int32 InputID);

	/** Tryna activate buffered input and flush the buffer */
	virtual void FlushBufferedInput();

	/** Clear Input buffer */
	virtual void ClearInputBuffer();

	/** Checks if any buffered input exist */
	virtual bool HasBufferedInput();

	/** Get a current attack target (Locked-on target by camera) */
	FORCEINLINE virtual TObjectPtr<ACharacter> GetAttackTarget() override { return LockingOnCharacter; }
	
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

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** A widget component to indicate incoming danager attacks */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UWidgetComponent* WidgetComp;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Run Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* RunAction;

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

	/** Maximum Distance of Lockable Target */
	UPROPERTY(EditAnywhere, Category="Camera")
	double LockOnDistance;

	/** A character being locked on by the player */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<ACharacter> LockingOnCharacter;

private:

	/** Used by input buffering system */
	TOptional<FBufferedInput> BufferedInput;

	/** Initial camera location from player coord system. Used in calculating camera lock angle */
	FVector InitLocalCameraLocation;

	/** Used by camera lock on system */
	bool bLockingOnCamera;

	/** Running state */
	bool bRunning;

};