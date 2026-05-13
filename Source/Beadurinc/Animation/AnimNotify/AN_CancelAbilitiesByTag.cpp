#include "Animation/AnimNotify/AN_CancelAbilitiesByTag.h"

#include "Actor/Character/FighterCharacter.h"

void UAN_CancelAbilitiesByTag::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner());
	if (!FighterCharacter) return;
	
	FighterCharacter->GetAbilitySystemComponent()->CancelAbilities(&TagsToCancelAbilities);
}
