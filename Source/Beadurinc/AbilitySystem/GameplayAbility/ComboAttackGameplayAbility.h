// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "ComboAttackGameplayAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
/**
 * 
 */
UCLASS()
class BEADURINC_API UComboAttackGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	/// Constructor
	UComboAttackGameplayAbility();
	
	/** Plays next combo montage by ability task */
	void PlayNextComboAttack();
	
protected:
	/**
	 * Checks if the player can do combo attacks
	 */
	virtual bool CanActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;

	/**
	 * Plays a combo animation and increases combo counter
	 */
	virtual void ActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	/**
	 * Called when the player press the ability key *while ability is activated*
	 */
	virtual void InputPressed
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	/**
	 * Called when the player release the ability key *while ability is activated*
	 */
	virtual void InputReleased
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	/**
	 * Called on the ability being canceled
	 * Effectively call EndAbility
 	 */
	virtual void CancelAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateCancelAbility
	) override;
	
	/**
	 * Called on the ability finished both by force and normal finish
	 */
	virtual void EndAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
	
	/** Reset combo counter to 0 */
	UFUNCTION()
	void ResetComboCounter();
	
	/** Called on montage interrupted by another montage being played */
	UFUNCTION()
	void OnMontageInterrupted();
	
	/** Called on ability task completed */
	UFUNCTION()
	void OnMontageCompleted();
	
private:
	
	/** Combo counter */
	int ComboCounter;
	
	/** Marker for next combo activation, prevents OnInterrupted callback cancels the combo counter */
	bool bResetNextComboCounter;
	
	/** Checks if any buffered input exists */
	//bool bInputBuffered;
	
	/** A timer handle to reset input buffer */
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* LastComboMontagePlayTask;
	
	/** Ability Task waiting for notify */
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* ComboOpenTask;

};
