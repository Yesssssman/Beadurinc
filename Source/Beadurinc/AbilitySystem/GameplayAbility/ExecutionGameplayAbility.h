#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ExecutionGameplayAbility.generated.h"

class UContextualAnimSceneAsset;

UCLASS()
class BEADURINC_API UExecutionGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	TObjectPtr<UContextualAnimSceneAsset> ExecutionAnimation;
};
