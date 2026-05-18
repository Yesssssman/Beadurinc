#include "Animation/AnimNotify/AN_ConsumeHeal.h"

#include "Actor/Character/PlayerCharacter.h"
#include "GameData/BeadurincPlayerState.h"
#include "GameFramework/PlayerState.h"

void UAN_ConsumeHeal::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter) return;
	
	ABeadurincPlayerState* PlayerState = Cast<ABeadurincPlayerState>(OwnerCharacter->GetPlayerState());
	if (!PlayerState) return;
	
	PlayerState->ConsumeHealingPotion();
}
