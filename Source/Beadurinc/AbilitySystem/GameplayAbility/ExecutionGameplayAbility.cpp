#include "AbilitySystem/GameplayAbility/ExecutionGameplayAbility.h"
#include "Actor/Character/FighterCharacter.h"
#include "ContextualAnimTypes.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "ContextualAnimSceneActorComponent.h"

bool UExecutionGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	AFighterCharacter* OwnerCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	GEngine->AddOnScreenDebugMessage(-1, 2.0F, FColor::Green, FString::Printf(TEXT("CanActivate two conditions: %d %d"), !OwnerCharacter, !OwnerCharacter->GetAttackTarget()));
	
	if (!OwnerCharacter) return false;
	
	AFighterCharacter* TargetCharacter = Cast<AFighterCharacter>(OwnerCharacter->GetAttackTarget());
	if (!TargetCharacter) return false;
	
	// Check GAS tag
	return TargetCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateGameplayTags::State_VulnerableToExecution);
};

void UExecutionGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AFighterCharacter* OwnerCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	GEngine->AddOnScreenDebugMessage(-1, 2.0F, FColor::Green, FString::Printf(TEXT("Activate two conditions: %d %d"), !OwnerCharacter, !ExecutionAnimation));
	GEngine->AddOnScreenDebugMessage(-1, 2.0F, FColor::Green, FString::Printf(TEXT("What the hek %s"), *ExecutionAnimation.GetName()));
	
	if (OwnerCharacter && ExecutionAnimation)
	{
		FContextualAnimSceneBindings ContextualAnimBindings(*ExecutionAnimation, 0, 0);
		
		// Bind each actor with role names
		ContextualAnimBindings.BindActorToRole(*OwnerCharacter, FName(TEXT("Executioner")));
		ContextualAnimBindings.BindActorToRole(*OwnerCharacter->GetAttackTarget(), FName(TEXT("Executed")));
	
		GEngine->AddOnScreenDebugMessage(-1, 2.0F, FColor::Green, TEXT("Start CAS?"));
	
		// Start CAS with binding info
		OwnerCharacter->GetContextualAnimSceneActorComponent()->StartContextualAnimScene(ContextualAnimBindings);
	};
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}