#include "Animation/AnimNotify/ANS_SetLandMovementMode.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UANS_SetLandMovementMode::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	ACharacter* Owner = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Owner) return;
	
	if (!Owner->GetCharacterMovement()) return;
	
	Owner->GetCharacterMovement()->SetMovementMode(MovementModeToChange);
}

void UANS_SetLandMovementMode::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	ACharacter* Owner = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Owner) return;
	
	if (!Owner->GetCharacterMovement()) return;
	
	Owner->GetCharacterMovement()->SetMovementMode(MovementModeToRevert);
}
