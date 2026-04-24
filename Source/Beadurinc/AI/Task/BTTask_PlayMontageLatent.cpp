#include "AI/Task/BTTask_PlayMontageLatent.h"

#include "AIController.h"
#include "Actor/Character/FighterCharacter.h"
#include "GameFramework/Character.h"

UBTTask_PlayMontageLatent::UBTTask_PlayMontageLatent()
{
	NodeName = "Play Montage Latent";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PlayMontageLatent::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const AIController = OwnerComp.GetAIOwner();
	UAnimMontage* AnimMontage = MontageToPlay.GetValue<UAnimMontage>(OwnerComp);
	if (!AnimMontage || !AIController) return EBTNodeResult::Failed;
	
	AFighterCharacter* const AICharacter = Cast<AFighterCharacter>(AIController->GetPawn());
	if (!AICharacter) return EBTNodeResult::Failed;
	
	UAnimInstance* AnimInstance = AICharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return EBTNodeResult::Failed;
	
	// Play montage (unsafe)
	PlayMontageMulticast(AICharacter, AnimMontage);
	
	if (StatefulProgress)
	{
		// Delegate for montage finish
		FOnMontageEnded MontageEndDelegate;
		MontageEndDelegate.BindUObject(this, &UBTTask_PlayMontageLatent::OnMontageFinished, &OwnerComp);
		// Give delegate and montage object, finishes of AnimMontage will call the delegate
		AnimInstance->Montage_SetBlendingOutDelegate(MontageEndDelegate, AnimMontage);
	}
	
	return StatefulProgress ? EBTNodeResult::InProgress : EBTNodeResult::Succeeded;
}

/** Latent task for montage finishes */
void UBTTask_PlayMontageLatent::OnMontageFinished(UAnimMontage* MontageIn, bool bInterrupted, UBehaviorTreeComponent* OwnerComp) const
{
	// Prevents the node being success if the montage is interrupted by another way
	if (OwnerComp && OwnerComp->GetActiveNode() == this)
	{
		// Tells BT to continue to the next node
		FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}

/** RPC implementation */
void UBTTask_PlayMontageLatent::PlayMontageMulticast_Implementation(ACharacter* CharacterIn, UAnimMontage* MontageIn)
{
	if (CharacterIn->GetMesh() && CharacterIn->GetMesh()->GetAnimInstance())
	{
		CharacterIn->PlayAnimMontage(MontageIn);
	}
}