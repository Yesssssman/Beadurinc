#include "MeleeTraceAnimationNotify.h"

#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Actor/WeaponActor.h"
#include "Actor/Character/FighterCharacter.h"
#include "AbilitySystemComponent.h"

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
	
	if (!FighterCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateGameplayTags::State_Parried)) return;
	
	// Clear parry tag if exists
	FighterCharacter->GetAbilitySystemComponent()->RemoveLooseGameplayTag(StateGameplayTags::State_Parried);
}
