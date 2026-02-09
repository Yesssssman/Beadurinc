// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BlockParryGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class BEADURINC_API UBlockParryGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> BlockingMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> OnBlockedMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimMontage>> OnParriedMontage;
	
public:
	UBlockParryGameplayAbility();
	
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
	 * Takes blocking stance
	 */
	virtual void ActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
	/**
	 * Called on the ability being canceled
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
	
protected:
	/**
	 * Callback on parry window finished
	 */
	UFUNCTION()
	void OnParryWindowFinished();
	
	/**
	 * Callback on block key released
	 */
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
};
