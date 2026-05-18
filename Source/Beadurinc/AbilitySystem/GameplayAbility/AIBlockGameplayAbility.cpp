#include "AbilitySystem/GameplayAbility/AIBlockGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/GameplayTag/GameplayEventTags.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Actor/Character/FighterCharacter.h"

UAIBlockGameplayAbility::UAIBlockGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAIBlockGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	if (!Fighter || !Fighter->GetAbilitySystemComponent())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	// Enter blocking state — consumed by UHitReactGameplayAbility on incoming hit.
	Fighter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateGameplayTags::State_Blocking);

	if (IsValid(BlockMontage)) Fighter->PlayAnimMontage(BlockMontage);

	// Early-out: end as soon as a hit is consumed during the window so the BT isn't blocked waiting.
	// OnlyTriggerOnce=true so the task auto-cancels itself after a single event.
	UAbilityTask_WaitGameplayEvent* AT_WaitHit = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		GameplayEventTags::Event_Combat_Hit,
		/*OptionalExternalTarget*/ nullptr,
		/*OnlyTriggerOnce*/ true,
		/*OnlyMatchExact*/ false
	);
	AT_WaitHit->EventReceived.AddDynamic(this, &UAIBlockGameplayAbility::OnHitReceived);
	AT_WaitHit->ReadyForActivation();
	
	// Timeout: ability persists at least this amount of time to prevent block spamming
	UAbilityTask_WaitDelay* AT_WaitDelay = UAbilityTask_WaitDelay::WaitDelay(this, LeastDuration);
	AT_WaitDelay->OnFinish.AddDynamic(this, &UAIBlockGameplayAbility::OnExpired);
	AT_WaitDelay->ReadyForActivation();
}

void UAIBlockGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	if (!Fighter) return;

	//if (IsValid(BlockMontage)) Fighter->StopAnimMontage(BlockMontage);

	UAbilitySystemComponent* ASC = Fighter->GetAbilitySystemComponent();
	if (!ASC) return;

	if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Blocking))
	{
		ASC->RemoveLooseGameplayTag(StateGameplayTags::State_Blocking);
	}
}

void UAIBlockGameplayAbility::OnExpired()
{
	if (!IsActive()) return;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAIBlockGameplayAbility::OnHitReceived(FGameplayEventData Payload)
{
	if (!IsActive()) return;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}