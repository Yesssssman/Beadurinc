#include "AbilitySystem/GameplayAbility/AIParryGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/GameplayTag/GameplayEventTags.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Actor/Character/FighterCharacter.h"

UAIParryGameplayAbility::UAIParryGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAIParryGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	if (!Fighter || !Fighter->GetAbilitySystemComponent())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	UAbilitySystemComponent* ASC = Fighter->GetAbilitySystemComponent();

	// UHitReactGameplayAbility requires BOTH tags to take the parry branch.
	ASC->AddLooseGameplayTag(StateGameplayTags::State_Blocking);
	ASC->AddLooseGameplayTag(StateGameplayTags::State_Parry);

	if (IsValid(ParryMontage)) Fighter->PlayAnimMontage(ParryMontage);

	// Early-out: end as soon as a hit is consumed during the window so the BT isn't blocked waiting.
	// OnlyTriggerOnce=true so the task auto-cancels itself after a single event.
	UAbilityTask_WaitGameplayEvent* AT_WaitHit = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		GameplayEventTags::Event_Combat_Hit,
		/*OptionalExternalTarget*/ nullptr,
		/*OnlyTriggerOnce*/ true,
		/*OnlyMatchExact*/ false
	);
	AT_WaitHit->EventReceived.AddDynamic(this, &UAIParryGameplayAbility::OnHitReceived);
	AT_WaitHit->ReadyForActivation();
	
	// Timeout: ability persists at least this amount of time to prevent block spamming
	UAbilityTask_WaitDelay* AT_WaitDelay = UAbilityTask_WaitDelay::WaitDelay(this, LeastDuration);
	AT_WaitDelay->OnFinish.AddDynamic(this, &UAIParryGameplayAbility::OnExpired);
	AT_WaitDelay->ReadyForActivation();
}

void UAIParryGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	if (!Fighter) return;

	UAbilitySystemComponent* ASC = Fighter->GetAbilitySystemComponent();
	if (!ASC) return;

	if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Parry))
	{
		ASC->RemoveLooseGameplayTag(StateGameplayTags::State_Parry);
	}
	
	if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Blocking))
	{
		ASC->RemoveLooseGameplayTag(StateGameplayTags::State_Blocking);
	}
}

void UAIParryGameplayAbility::OnExpired()
{
	if (!IsActive()) return;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAIParryGameplayAbility::OnHitReceived(FGameplayEventData Payload)
{
	if (!IsActive()) return;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}