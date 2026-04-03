// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/CheckAndPlayParriedAnimNotify.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Actor/Character/FighterCharacter.h"

void UCheckAndPlayParriedAnimNotify::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	// No parried animation set
	if (!IsValid(Parried)) return;
	
	AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner());
	
	// Not a FighterCharacter
	if (!FighterCharacter) return;
	
	// Check the attack ever parried, then play the parry stun animation.
	if (!FighterCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateGameplayTags::State_Parried)) return;
	
	// Play parried montage
	FighterCharacter->PlayAnimMontage(Parried);
	// Remove state tag
	FighterCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateGameplayTags::State_Parried);
}
