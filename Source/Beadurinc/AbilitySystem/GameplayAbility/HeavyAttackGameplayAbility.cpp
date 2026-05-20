#include "AbilitySystem/GameplayAbility/HeavyAttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Actor/Character/PlayerCharacter.h"
#include "Actor/WeaponActor.h"
#include "AbilitySystem/GameplayTag/AbilityTags.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"

UHeavyAttackGameplayAbility::UHeavyAttackGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UHeavyAttackGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	
	APlayerCharacter* Owner = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!Owner || !Owner->IsHoldingWeapon() || !Owner->GetWeaponActor()->GetHeavyAttack()) return false;
	
	// Reuse the global attack lock used by Combo / Execution.
	return !ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(StateGameplayTags::State_ComboLocked);
}

void UHeavyAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	APlayerCharacter* Owner = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	
	if (!Owner)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
	
	// Snap yaw to camera so the heavy swing tracks the player's aim.
	const FRotator ActorRot = Owner->GetActorRotation();
	const FRotator ControlRot = Owner->GetController()->GetControlRotation();
	Owner->SetActorRotation(FRotator(ActorRot.Pitch, ControlRot.Yaw, ActorRot.Roll));
	
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	if (MontageTask)
	{
		// Force cancel of the old task to prevent calling `EndAbility` abnormally
		MontageTask->ExternalCancel();
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, TEXT("HeavyAttack"), Owner->GetWeaponActor()->GetHeavyAttack()
	);

	MontageTask->OnCompleted.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageCompleted);

	MontageTask->OnInterrupted.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageInterrupted);

	MontageTask->ReadyForActivation();

	Owner->ClearInputBuffer();

	// Same side-effects as Combo: cancel block, eat stamina regen.
	FGameplayTagContainer BlockTags;
	BlockTags.AddTag(AbilityTags::Ability_BlockParry);
	ASC->CancelAbilities(&BlockTags);
	
	if (AFighterCharacter* Fighter = Cast<AFighterCharacter>(Owner))
	{
		Fighter->ApplyStaminaRegenCooldown();
	}
}

void UHeavyAttackGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	MontageTask = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHeavyAttackGameplayAbility::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UHeavyAttackGameplayAbility::OnMontageInterrupted()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false);
}