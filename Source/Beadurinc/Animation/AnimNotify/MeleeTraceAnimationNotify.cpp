#include "MeleeTraceAnimationNotify.h"

#include "Actor/WeaponActor.h"
#include "Actor/Character/FighterCharacter.h"

void UMeleeTraceAnimationNotify::NotifyBegin
(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	// Check if the owner is a weapon holdable character
	if (AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner()))
	{
		// Activate collision check when contacting phase starts
		FighterCharacter->ResetMeleeSwing();
		FighterCharacter->GetWeaponActor()->SetActorEnableCollision(true);
	}
}

void UMeleeTraceAnimationNotify::NotifyEnd
(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	// Check if the owner is a weapon holdable character
	if (AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner()))
	{
		// Deactivate collision check when contacting phase ends
		FighterCharacter->ResetMeleeSwing();
		FighterCharacter->GetWeaponActor()->SetActorEnableCollision(false);
	}
}
