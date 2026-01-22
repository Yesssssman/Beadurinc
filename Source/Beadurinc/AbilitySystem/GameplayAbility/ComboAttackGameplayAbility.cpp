// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbility/ComboAttackGameplayAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Actor/Character/PlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/AbilityId.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"

UComboAttackGameplayAbility::UComboAttackGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ComboCounter = 0;
}

/** Plays next combo montage by ability task */
void UComboAttackGameplayAbility::PlayNextComboAttack()
{
	APlayerCharacter* BCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	
	// Checks weapon in hand
	if (BCharacter && BCharacter->IsHoldingWeapon())
	{
		// Add combo lock state
		BCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateGameplayTags::State_ComboLocked);
		
		if (LastComboMontagePlayTask)
		{
			// Force cancel of the old task to prevent calling `EndAbility` abnormally
			LastComboMontagePlayTask->ExternalCancel();
		}
		
		// Triggers by play montage ability task (activates until montage ends)
		UAbilityTask_PlayMontageAndWait* AT = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("ComboAttack"),
			BCharacter->GetMainHandWeaponActor()->GetComboAttackAt(ComboCounter)
		);
		
		AT->OnCompleted.AddDynamic(this, &UComboAttackGameplayAbility::OnMontageCompleted);
		AT->OnInterrupted.AddDynamic(this, &UComboAttackGameplayAbility::OnMontageInterrupted);
		AT->ReadyForActivation();
		LastComboMontagePlayTask = AT;
		GEngine->AddOnScreenDebugMessage(-1, 3.0F, FColor::Green,  FString::Printf(TEXT("Combo %d"), ComboCounter));
		// Clamp combo counter to combo montage array length
		ComboCounter = (ComboCounter + 1) % BCharacter->GetMainHandWeaponActor()->GetComboSequenceLength();
		BCharacter->ClearInputBuffer();
	}
}

bool UComboAttackGameplayAbility::CanActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags
) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	if (APlayerCharacter* AbilityOwner = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (const UAbilitySystemComponent* ASC = AbilityOwner->GetAbilitySystemComponent())
		{
			// If State_ComboLocked exists, ability is not available
			return !ASC->HasMatchingGameplayTag(StateGameplayTags::State_ComboLocked);
		}
	}
	
	return false;
}

/// Called on the ability being activated
void UComboAttackGameplayAbility::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	PlayNextComboAttack();
}

/**
 * Called when the player press the ability key *while ability is activated*
 */
void UComboAttackGameplayAbility::InputPressed
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
)
{
	if (APlayerCharacter* AbilityOwner = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		// Check if the ability is available again
		if (CanActivateAbility(Handle, ActorInfo))
		{
			PlayNextComboAttack();
		}
		else
		{
			AbilityOwner->BufferInput(EAbilityId::Combo_Attack);
		}
	}
}

/**
 * Called when the player release the ability key *while ability is activated*
 */
void UComboAttackGameplayAbility::InputReleased
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

/**
 * Called when the combo combo attack montage is done by force
 */
void UComboAttackGameplayAbility::CancelAbility
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
 * Called when the combo attack montage is done by reaching the last frame
 */
void UComboAttackGameplayAbility::EndAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	// "bResetNextComboCounter == false" means the next montage is combo sequence attack montage
	ResetComboCounter();
	
	LastComboMontagePlayTask = nullptr;
}

/**
 * Montage interrupt callback by ability task
 */
void UComboAttackGameplayAbility::OnMontageInterrupted()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false);
}

/**
 * Montage complete callback by ability task
 */
void UComboAttackGameplayAbility::OnMontageCompleted()
{
	// If montage reaches the last frame reset the combo counter
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

/**
 * Reset combo counter to start combo from the first sequence
 */
void UComboAttackGameplayAbility::ResetComboCounter()
{
	ComboCounter = 0;
}