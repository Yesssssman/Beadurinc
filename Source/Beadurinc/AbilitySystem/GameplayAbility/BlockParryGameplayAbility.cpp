#include "BlockParryGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Actor/Character/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBlockParryGameplayAbility::UBlockParryGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UBlockParryGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!IsValid(BlockLocomotion)) return false;
	
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (PlayerCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateGameplayTags::State_BlockingLocked))
		{
			return false;
		}
		
		// Unable to block while in the air
		if (PlayerCharacter->GetCharacterMovement()->IsFalling())
		{
			return false;
		}
	}
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UBlockParryGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	if (!Fighter) return;
	
	// Add state tags
	Fighter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateGameplayTags::State_Blocking);
	Fighter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateGameplayTags::State_Parry);
	
	// Play blocking anim montage
	if (IsValid(BlockLocomotion)) Fighter->PlayAnimMontage(BlockLocomotion);
	
	// Allows parrying within time window 0.5 seconds
	UAbilityTask_WaitDelay* AT_WaitDelay = UAbilityTask_WaitDelay::WaitDelay(
		this,
		0.25F
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

/**
 * Called on the ability finished both by force and normal finish
 */
void UBlockParryGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter) return;
	
	PlayerCharacter->StopAnimMontage(BlockLocomotion);
	
	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
	if (!ASC) return;
	
	// Remove state tags
	if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Blocking))
	{
		PlayerCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateGameplayTags::State_Blocking);
	}
	
	if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Parry))
	{
		PlayerCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateGameplayTags::State_Parry);
	}
}

void UBlockParryGameplayAbility::OnParryWindowFinished()
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!PlayerCharacter) return;
	
	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
	if (!ASC) return;
	
	if (!ASC->HasMatchingGameplayTag(StateGameplayTags::State_Parry)) return;
	
	PlayerCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateGameplayTags::State_Parry);
}

void UBlockParryGameplayAbility::OnInputReleased(float TimeHeld)
{
	if (!IsActive()) return;
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

