// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockParryGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Actor/Character/PlayerCharacter.h"

UBlockParryGameplayAbility::UBlockParryGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

/**
 * Checks if the player can do combo attacks
 */
bool UBlockParryGameplayAbility::CanActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags
) const
{
	if (!IsValid(BlockingMontage))
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
void UBlockParryGameplayAbility::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		// Add state tags
		PlayerCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateGameplayTags::State_Blocking);
		PlayerCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateGameplayTags::State_Parry);
		
		// Play blocking anim montage
		if (IsValid(BlockingMontage))
			PlayerCharacter->PlayAnimMontage(BlockingMontage);
		
		// Allows parrying within time window 0.5 seconds
		UAbilityTask_WaitDelay* AT_WaitDelay = UAbilityTask_WaitDelay::WaitDelay(
			this,
			0.5F
		);
		
		AT_WaitDelay->OnFinish.AddDynamic(this, &UBlockParryGameplayAbility::OnParryWindowFinished);
		AT_WaitDelay->ReadyForActivation();
		
		// Stop blocking on input unpressed
		UAbilityTask_WaitInputRelease* AT_WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(
			this,
			false
		);
		
		AT_WaitInputRelease->OnRelease.AddDynamic(this, &UBlockParryGameplayAbility::OnInputReleased);
		AT_WaitInputRelease->ReadyForActivation();
	}
}

/**
 * Called on the ability being canceled
  */
void UBlockParryGameplayAbility::CancelAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility
)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

/**
 * Called on the ability finished both by force and normal finish
 */
void UBlockParryGameplayAbility::EndAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		PlayerCharacter->StopAnimMontage(BlockingMontage);
		
		UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
		if (!ASC) return;
		
		// Remove state tags
		if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Blocking))
			PlayerCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateGameplayTags::State_Blocking);
		
		if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Parry))
			PlayerCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateGameplayTags::State_Parry);
	}
}

void UBlockParryGameplayAbility::OnParryWindowFinished()
{
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
		
		if (!ASC) return;
		
		if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Parry))
			PlayerCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateGameplayTags::State_Parry);
	}
}

void UBlockParryGameplayAbility::OnInputReleased(float TimeHeld)
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}