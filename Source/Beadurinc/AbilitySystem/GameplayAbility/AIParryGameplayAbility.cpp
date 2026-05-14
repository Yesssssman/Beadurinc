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

	// Hard time-out: end after the parry window.
	UAbilityTask_WaitDelay* AT_WaitDelay = UAbilityTask_WaitDelay::WaitDelay(this, ParryWindow);
	AT_WaitDelay->OnFinish.AddDynamic(this, &UAIParryGameplayAbility::OnParryWindowFinished);
	AT_WaitDelay->ReadyForActivation();

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
}

void UAIParryGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	if (!Fighter) return;

	if (IsValid(ParryMontage)) Fighter->StopAnimMontage(ParryMontage);

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

void UAIParryGameplayAbility::OnParryWindowFinished()
{
	if (!IsActive()) return;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAIParryGameplayAbility::OnHitReceived(FGameplayEventData Payload)
{
	// HitReact (triggered via AbilityTriggers) has already run by the time this generic
	// listener fires, so the State_Blocking + State_Parry tags have been consumed for
	// routing. Safe to tear down here and free the BT.
	if (!IsActive()) return;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}