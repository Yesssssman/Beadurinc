#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HitReactGameplayAbility.generated.h"

UCLASS()
class BEADURINC_API UHitReactGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:

	/**
	 * Checks whether the actor blocks the attack or gets hurt
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	
	/** A montage to play on blocking attacks */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> OnBlock;
	
	/** A montage that played when offended on stamina 0 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> Stagger;
	
	/** Montages to play on parrying attacks. Identified by AttackTypeTags */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = true))
	TMap<FGameplayTag, UAnimMontage*> OnParry;

	/** Montage to play on damage applied to health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> OnHurt;

	/** Attacker pause time when hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = true))
	float HitStop;
	
	/** Whether to activate no stamina stagger on health damage. False for players, True for monsters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = true))
	bool StaggerOnHealthDamage = true;
	
	/** Stamina damage attenuation when hurt */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (AllowPrivateAccess = true))
	float StaminaDamageAttenuation = 1.0F;
	
};