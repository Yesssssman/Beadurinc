#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ParryReactGameplayAbility.generated.h"

UCLASS()
class BEADURINC_API UParryReactGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:
	
	/** A montage that played when offended on stamina 0 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> Stagger;
};
