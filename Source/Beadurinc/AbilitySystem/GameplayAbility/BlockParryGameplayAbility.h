// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BlockParryGameplayAbility.generated.h"

/**
 * Note: the playing hit animation is done by `HitReactGameplayAbility.` This ability only in charge of activating
 * blocking stance and calculating parrying window since the last time the ability is activated.
 */
UCLASS()
class BEADURINC_API UBlockParryGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
private:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> BlockLocomotion;
	
	/**
	 * A parry window applied to the next blocking input. Gradually decreases when player press continuously.
	 * On any type of hit react activates, hit, block, parrying will reset this value to maximum.
	 */
	float NextParryWindowTime;
	
	/**
	 * Initial value of parry window
	 */
	float MaxParryWindowTime;
	
	/**
	 * Initial value of parry window
	 */
	float ParryWindowPenaltyMultiplier;
	
public:
	
	UBlockParryGameplayAbility();
	
protected:
	
	/**
	 * Checks if the player can do combo attacks
	 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	/**
	 * Takes blocking stance
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	/**
	 * Called on the ability finished both by force and normal finish
	 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
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
