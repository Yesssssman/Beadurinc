#include "MeleeTraceAnimationNotify.h"
#include "Actor/WeaponActor.h"
#include "Actor/Character/FighterCharacter.h"

void UMeleeTraceAnimationNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner());
	if (!FighterCharacter) return;
	
	// Activate collision check when contacting phase starts
	FighterCharacter->ResetMeleeSwing();
	FighterCharacter->GetWeaponActor()->SetActorEnableCollision(true);
}

void UMeleeTraceAnimationNotify::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner());
	if (!FighterCharacter) return;
	
	// Deactivate collision check when contacting phase ends
	FighterCharacter->ResetMeleeSwing();
	FighterCharacter->GetWeaponActor()->SetActorEnableCollision(false);
}
