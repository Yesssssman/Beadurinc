#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PlayerHealGameplayAbility.generated.h"

class UAbilityTask_PlayMontageAndTriggerZeroNotifies;

UCLASS()
class BEADURINC_API UPlayerHealGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UPlayerHealGameplayAbility();
	
public:
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:

	/** Ability Task callback */
	UFUNCTION()
	void MontageEnds();
	
private:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> HealAnimMontage;
};
