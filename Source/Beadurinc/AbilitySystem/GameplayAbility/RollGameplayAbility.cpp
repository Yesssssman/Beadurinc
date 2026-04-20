// Fill out your copyright notice in the Description page of Project Settings.

#include "RollGameplayAbility.h"
#include "Actor/Character/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

URollGameplayAbility::URollGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

/**
 * Checks if the player can do combo attacks
 */
bool URollGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!IsValid(RollingMontage))
	{
		return false;
	}
	
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		return !PlayerCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateGameplayTags::State_RollingLocked);
	}
	
	return false;
}

/**
 * Takes blocking stance
 */
void URollGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		return;
	}

	// Suppress lock-on actor rotation so the roll direction is controlled by WASD input
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	ASC->AddLooseGameplayTag(StateGameplayTags::State_LockOnRotationSuppressed);

	// Orient actor toward WASD input direction before playing the roll montage
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		FVector MoveInput = PlayerCharacter->GetLastMovementInputVector();
		
		if (!MoveInput.IsNearlyZero())
		{
			FRotator RollRotation = MoveInput.Rotation();
			RollRotation.Pitch = 0.f;
			RollRotation.Roll  = 0.f;
			PlayerCharacter->SetActorRotation(RollRotation);
		}
	}

	UAbilityTask_PlayMontageAndWait* AT = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("ComboAttack"),
		RollingMontage
	);

	AT->OnCompleted.AddDynamic(this, &URollGameplayAbility::MontageEnds);
	AT->OnInterrupted.AddDynamic(this, &URollGameplayAbility::MontageEnds);
	AT->ReadyForActivation();

	// Apply stamina gen cooldown
	if (AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get()))
	{
		FighterCharacter->ApplyStaminaRegenCooldown();
	}
}

/** On rolling animation ends */
void URollGameplayAbility::MontageEnds()
{
	// Restore lock-on actor rotation
	if (UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(StateGameplayTags::State_LockOnRotationSuppressed);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
