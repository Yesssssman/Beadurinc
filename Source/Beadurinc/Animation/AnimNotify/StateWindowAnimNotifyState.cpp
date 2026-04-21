// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystemComponent.h"
#include "Actor/Character/FighterCharacter.h"
#include "Animation/AnimNotify/StateWindowAnimNotifyState.h"

void UStateWindowAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	const AFighterCharacter* BCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner());
	if (!BCharacter) return;
	
	for (FGameplayTag StateTag : StateTags)
	{
		if (BCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateTag)) continue;
		BCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateTag);
	}
}

void UStateWindowAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	const AFighterCharacter* BCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner());
	if (!BCharacter) return;
	
	for (FGameplayTag StateTag : StateTags)
	{
		if (!BCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateTag)) continue;
		BCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateTag);
	}
}
