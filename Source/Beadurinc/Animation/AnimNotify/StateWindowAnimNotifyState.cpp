#include "Actor/Character/FighterCharacter.h"
#include "Animation/AnimNotify/StateWindowAnimNotifyState.h"

void UStateWindowAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AFighterCharacter* BCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner());
	if (!BCharacter) return;
	
	for (FGameplayTag StateTag : StateTags)
	{
		BCharacter->AddState(-1, StateTag);
	}
}

void UStateWindowAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AFighterCharacter* BCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner());
	if (!BCharacter) return;
	
	for (FGameplayTag StateTag : StateTags)
	{
		BCharacter->RemoveState(-1, StateTag);
	}
}

void UStateWindowAnimNotifyState::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	AFighterCharacter* BCharacter = Cast<AFighterCharacter>(BranchingPointPayload.SkelMeshComponent == nullptr ? nullptr : BranchingPointPayload.SkelMeshComponent->GetOwner());
	if (!BCharacter) return;
	
	for (FGameplayTag StateTag : StateTags)
	{
		BCharacter->AddState(BranchingPointPayload.MontageInstanceID, StateTag);
	}
}

void UStateWindowAnimNotifyState::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	AFighterCharacter* BCharacter = Cast<AFighterCharacter>(BranchingPointPayload.SkelMeshComponent == nullptr ? nullptr : BranchingPointPayload.SkelMeshComponent->GetOwner());
	if (!BCharacter) return;
	
	for (FGameplayTag StateTag : StateTags)
	{
		BCharacter->RemoveState(BranchingPointPayload.MontageInstanceID, StateTag);
	}
}
