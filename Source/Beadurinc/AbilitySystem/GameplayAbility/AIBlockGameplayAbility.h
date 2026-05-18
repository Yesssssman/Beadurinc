#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AIBlockGameplayAbility.generated.h"

/**
 * Monster-side blocking ability. The Behavior Tree is responsible for deciding
 * whether activation is appropriate (via decorators / score). On activation we
 * simply hold State_Blocking for BlockDuration, then end.
 *
 * Trigger from BT: SendGameplayEventToActor with tag AbilityTags::Ability_AI_Block.
 * The trigger is wired in the derived Blueprint via AbilityTriggers.
 */
UCLASS()
class BEADURINC_API UAIBlockGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	/** Constructor */
	UAIBlockGameplayAbility();

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	/** Callback after BlockDuration elapses */
	UFUNCTION()
	void OnExpired();

	/** Callback when the avatar receives Event.Combat.Hit while parrying. Ends the ability early. */
	UFUNCTION()
	void OnHitReceived(FGameplayEventData Payload);
	
private:

	/** Optional stance montage. May be null; ABP can drive the pose from State_Blocking instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> BlockMontage;
	
	/** Duration persists at least this time */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	float LeastDuration;
};