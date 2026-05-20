#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HeavyAttackGameplayAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
/**
 * Single-shot heavy attack. Triggered by holding the attack key, implemented in
 * APlayerCharacter::SetupPlayerInputComponent via a separate Enhanced Input
 * Action with a UInputTriggerHold modifier.
 *
 * Buffered through the existing input-buffer path (APlayerCharacter::BufferInput /
 * FlushBufferedInput / FireBufferedInputAnimNotify), so 'combo1 -> combo2 -> heavy'
 * works without any cross-ability coupling. When heavy interrupts a running combo
 * montage, the combo ability's OnMontageInterrupted handler cancels it and resets
 * ComboCounter through EndAbility.
 */
UCLASS()
class BEADURINC_API UHeavyAttackGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	
	/// Constructor
	UHeavyAttackGameplayAbility();
	
public:
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/// Called when the heavy montage reaches the last frame.
	UFUNCTION()
	void OnMontageCompleted();

	/// Called when another montage interrupts the heavy montage.
	UFUNCTION()
	void OnMontageInterrupted();
	
private:
	
	/// Kept so we can ExternalCancel if a follow-up attack chains in.
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;
};