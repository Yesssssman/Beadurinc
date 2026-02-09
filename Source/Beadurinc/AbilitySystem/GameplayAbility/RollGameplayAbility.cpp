// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Character/PlayerCharacter.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"

#include "RollGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

URollGameplayAbility::URollGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

/**
 * Checks if the player can do combo attacks
 */
bool URollGameplayAbility::CanActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags
) const
{
	if (!IsValid(RollingMontage))
	{
		return false;
	}
	
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		return !PlayerCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateGameplayTags::State_BlockingLocked);
	}
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

/**
 * Takes blocking stance
 */
void URollGameplayAbility::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		UAbilityTask_PlayMontageAndWait* AT = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("ComboAttack"),
			RollingMontage
		);
		
		AT->OnCompleted.AddDynamic(this, &URollGameplayAbility::End);
		AT->OnInterrupted.AddDynamic(this, &URollGameplayAbility::End);
		AT->ReadyForActivation();
	}
}

/** On rolling animation ends */
void URollGameplayAbility::End()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}