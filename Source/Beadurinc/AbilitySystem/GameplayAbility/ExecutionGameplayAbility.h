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
	
	/** Contextual animation to play when execution triggered */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	TObjectPtr<UContextualAnimSceneAsset> ExecutionAnimation;
	
	/** Min distance between the actors to trigger execution */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Criteria", meta = (AllowPrivateAccess = true))
	float Distance;
	
	/** A starting angle (-180 ~ 180) of allowed area. If the value is less than `AngleTo`, the corn are is calculated by counter-clockwise direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Criteria", meta = (AllowPrivateAccess = true))
	float AngleFrom;
	
	/** An ending angle (-180 ~ 180) of allowed area. If the value is bigger than `AngleFrom`, the corn are is calculated by counter-clockwise direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Criteria", meta = (AllowPrivateAccess = true))
	float AngleTo;
};
