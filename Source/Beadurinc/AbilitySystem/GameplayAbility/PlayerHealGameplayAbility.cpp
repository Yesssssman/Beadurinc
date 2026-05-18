#include "AbilitySystem/GameplayAbility/PlayerHealGameplayAbility.h"

#include "RollGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "GameData/BeadurincPlayerState.h"

UPlayerHealGameplayAbility::UPlayerHealGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UPlayerHealGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	ABeadurincPlayerState* PlayerState = Cast<ABeadurincPlayerState>(ActorInfo->OwnerActor.Get());
	if (!PlayerState) return false;
	
	// Can't activate ability when remaining healing potion is 0
	if (PlayerState->GetRemainingHealingPotions() < 1) return false;
	
	// Check state
	if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(StateGameplayTags::State_ComboLocked)) return false;
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UPlayerHealGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!HealAnimMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	}
	
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	ASC->AddLooseGameplayTag(StateGameplayTags::State_ComboLocked);
	
	UAbilityTask_PlayMontageAndWait* AT = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("Healing"),
		HealAnimMontage
	);
	
	AT->OnCompleted.AddDynamic(this, &UPlayerHealGameplayAbility::MontageEnds);
	AT->OnInterrupted.AddDynamic(this, &UPlayerHealGameplayAbility::MontageEnds);
	AT->OnCancelled.AddDynamic(this, &UPlayerHealGameplayAbility::MontageEnds);
	AT->OnBlendOut.AddDynamic(this, &UPlayerHealGameplayAbility::MontageEnds);
	
	AT->ReadyForActivation();
}

/** On rolling animation ends */
void UPlayerHealGameplayAbility::MontageEnds()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}