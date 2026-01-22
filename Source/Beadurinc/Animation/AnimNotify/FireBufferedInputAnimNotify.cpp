// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/FireBufferedInputAnimNotify.h"

#include "Actor/Character/PlayerCharacter.h"

void UFireBufferedInputAnimNotify::Notify
(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	if (!MeshComp) return;
	
	AActor* Owner = MeshComp->GetOwner();
	
	if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(Owner))
	{
		// Triggers the notify only in local client
		if (OwnerCharacter->IsLocallyControlled())
		{
			OwnerCharacter->TriggerBufferedInput();
		}
	}
}
