#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AIParryGameplayAbility.generated.h"

/**
 * Monster-side parrying ability. Holds State_Blocking + State_Parry for a short
 * window, then ends. The BT decides when to use this vs the plain block ability
 * (via behavior score).
 *
 * Also ends early if the avatar is hit during the window. UHitReactGameplayAbility
 * is triggered first (via AbilityTriggers), reads the State_Blocking + State_Parry
 * tags, and applies the parry branch. The UAbilityTask_WaitGameplayEvent listener
 * fires after the triggered ability dispatch — by then HitReact is done consuming
 * the tags, so it is safe to EndAbility here.
 *
 * Trigger from BT: SendGameplayEventToActor with tag AbilityTags::Ability_AI_Parry.
 * The trigger is wired in the derived Blueprint via AbilityTriggers.
 */
UCLASS()
class BEADURINC_API UAIParryGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	/** Constructor */
	UAIParryGameplayAbility();

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	/** Callback after ParryWindow elapses */
	UFUNCTION()
	void OnParryWindowFinished();

	/** Callback when the avatar receives Event.Combat.Hit while parrying. Ends the ability early. */
	UFUNCTION()
	void OnHitReceived(FGameplayEventData Payload);

private:

	/** Optional parry stance montage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ParryMontage;

	/** Duration the avatar holds both State_Blocking and State_Parry. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Behavior, meta = (AllowPrivateAccess = "true", ClampMin = "0.05"))
	float ParryWindow = 0.4F;
};