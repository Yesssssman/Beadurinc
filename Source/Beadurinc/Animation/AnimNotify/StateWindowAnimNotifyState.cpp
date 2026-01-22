// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/StateWindowAnimNotifyState.h"

#include "Actor/Character/PlayerCharacter.h"

void UStateWindowAnimNotifyState::NotifyBegin
(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	if (const APlayerCharacter* BCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		if (!BCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateTag))
		{
			BCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateTag);
		}
	}
}

void UStateWindowAnimNotifyState::NotifyEnd
(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	if (const APlayerCharacter* BCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
	{
		if (BCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateTag))
		{
			BCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateTag);
		}
	}
}