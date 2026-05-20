#include "AbilitySystem/GameplayAbility/ComboAttackGameplayAbility.h"
#include "Actor/Character/PlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/AbilityId.h"
#include "AbilitySystem/GameplayTag/AbilityTags.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"

UComboAttackGameplayAbility::UComboAttackGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ComboCounter = 0;
}

/** Plays next combo or dash montage by ability task */
void UComboAttackGameplayAbility::PlayNextComboAttack()
{
	APlayerCharacter* BCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());

	if (!BCharacter || !BCharacter->IsHoldingWeapon()) return;

	FRotator ActorRotation = BCharacter->GetActorRotation();
	FRotator ControlRotation = BCharacter->GetController()->GetControlRotation();

	BCharacter->SetActorRotation(FRotator(ActorRotation.Pitch, ControlRotation.Yaw, ActorRotation.Roll));

	if (LastComboMontagePlayTask)
	{
		// Force cancel of the old task to prevent calling `EndAbility` abnormally
		LastComboMontagePlayTask->ExternalCancel();
	}

	// Checks low attack immunity that is granted by jumping ability
	const bool bJumpBranch = CurrentActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(StateGameplayTags::State_LowAttackDodgeable) && IsValid(BCharacter->GetWeaponActor()->GetJumpAttack());

	// Sprint check uses actual planar velocity, not MaxWalkSpeed, so a "sprint toggled on but standing still" state doesn't fire dash.
	const bool bDashBranch = BCharacter->GetVelocity().Size2D() > 300.0F && IsValid(BCharacter->GetWeaponActor()->GetDashAttack());

	TObjectPtr<UAnimMontage> NextMontage;
	
	if (bJumpBranch)
	{
		NextMontage = BCharacter->GetWeaponActor()->GetJumpAttack();
		// Discards any in-progress combo chain.
		ResetComboCounter();
	}
	else if (bDashBranch)
	{
		NextMontage = BCharacter->GetWeaponActor()->GetDashAttack();
		// Discards any in-progress combo chain.
		ResetComboCounter();
	}
	else
	{
		NextMontage = BCharacter->GetWeaponActor()->GetComboAttackAt(ComboCounter);
	}

	if (!IsValid(NextMontage)) return;

	// Triggers by play montage ability task (activates until montage ends).
	UAbilityTask_PlayMontageAndWait* AbilityTaskPlayMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("ComboAttack"),
		NextMontage
	);

	AbilityTaskPlayMontage->OnCompleted.AddDynamic(this, &UComboAttackGameplayAbility::OnMontageCompleted);
	AbilityTaskPlayMontage->OnInterrupted.AddDynamic(this, &UComboAttackGameplayAbility::OnMontageInterrupted);
	AbilityTaskPlayMontage->ReadyForActivation();
	LastComboMontagePlayTask = AbilityTaskPlayMontage;

	if (!bJumpBranch && !bDashBranch)
	{
		ComboCounter = (ComboCounter + 1) % BCharacter->GetWeaponActor()->GetComboSequenceLength();
	}

	BCharacter->ClearInputBuffer();
}

bool UComboAttackGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	
	APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!OwnerCharacter) return false;
	
	// The case that the current attack target is executable. Instead of playing combo, it plays CAS finisher animation by `ExecutionGameplayAbility`. 
	if (OwnerCharacter->GetAttackTarget())
	{
		AFighterCharacter* TargetCharacter = Cast<AFighterCharacter>(OwnerCharacter->GetAttackTarget());
		if (!TargetCharacter) return false;
		
		// Check GAS tag
		if (TargetCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateGameplayTags::State_VulnerableToExecution))
		{
			return false;
		}
	}
	
	// If State_ComboLocked exists, ability is not available
	return !ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(StateGameplayTags::State_ComboLocked);
}

/// Called on the ability being activated
void UComboAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilitySystemComponent* OwnerACS = ActorInfo->AbilitySystemComponent.Get();
	if (!OwnerACS) return;
	
	PlayNextComboAttack();

	// Cancel blocking ability if enabled
	FGameplayTagContainer TargetTags;
	TargetTags.AddTag(AbilityTags::Ability_BlockParry);
	OwnerACS->CancelAbilities(&TargetTags);
	
	// Apply stamina gen cooldown
	if (AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get()))
	{
		FighterCharacter->ApplyStaminaRegenCooldown();
	}
}

/**
 * Called when the player press the ability key *while ability is activated*
 */
void UComboAttackGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
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
			AbilityOwner->BufferInput(static_cast<int32>(EAbilityId::Combo_Attack));
		}
	}
}

/**
 * Called when the combo attack montage is done by reaching the last frame
 */
void UComboAttackGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
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