// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HitReactGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class BEADURINC_API UHitReactGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
	/** Montage to play on blocking attacks */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> OnBlock;
	
	/** Montage to play on damage applied to health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> OnHurt;
	
	/** Whether the owner actor should look at attacker on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Control", meta = (AllowPrivateAccess = "true"))
	bool LookAttacker;
	
protected:
	/**
	 * Checks whether the actor blocks the attack or gets hurt
	 */
	virtual void ActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};
