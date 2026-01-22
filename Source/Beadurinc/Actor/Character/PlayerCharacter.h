// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AttributeSet/PlayerAttributeSet.h"
#include "Actor/WeaponActor.h"
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
class APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** Gameplay Ability class for Combo Attacks */
	UPROPERTY(EditAnywhere, Category = "Gameplay Abilities")
	TSubclassOf<UGameplayAbility> ComboAttackAbility;
	
	/** Gameplay Abilities */
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	/** Holding weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equipments", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeaponActor> MainHandWeapon;
	
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
	
	/** Combo Attack Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ComboAttackAction;
	
	UFUNCTION()
	void HandleHealthChange(float Magnitude, float NewHealth);

protected:
	
	UPROPERTY()
	TObjectPtr<UPlayerAttributeSet> HealthAttributeSet;
	
	UPROPERTY()
	TObjectPtr<AWeaponActor> MainHandWeaponActor;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle ComboAbilitySpecHandler;
	
	/** Used by input buffering system */
	TOptional<FBufferedInput> BufferedInput;
	
	UPROPERTY(EditAnywhere, Category="Attribute")
	float InitialHealth;
	
public:

	/** Constructor */
	APlayerCharacter();	

protected:

	/** On character first join the world **/
	virtual void BeginPlay() override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called when a new controller takes control of this character */
	virtual void PossessedBy(AController* NewController) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

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
	
	/** Returns whether the character is holding weapon in main hand **/
	FORCEINLINE bool IsHoldingWeapon() const { return IsValid(MainHandWeapon); }
	
	/** Returns Equipping Weapon Actor in main hand **/
	FORCEINLINE TObjectPtr<AWeaponActor> GetMainHandWeaponActor() const { return MainHandWeaponActor; }
	
	/** Returns Ability Component object **/
	FORCEINLINE UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; };
	
};

