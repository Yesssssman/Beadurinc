// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/FireBufferedInputAnimNotify.h"

#include "Actor/Character/BeadurincCharacter.h"

void UFireBufferedInputAnimNotify::Notify
(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	if (!MeshComp) return;
	
	AActor* Owner = MeshComp->GetOwner();
	
	if (ABeadurincCharacter* OwnerCharacter = Cast<ABeadurincCharacter>(Owner))
	{
		// Triggers the notify only in local client
		if (OwnerCharacter->IsLocallyControlled())
		{
			OwnerCharacter->TriggerBufferedInput();
		}
	}
}
