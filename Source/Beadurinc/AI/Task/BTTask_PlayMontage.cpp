// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Task/BTTask_PlayMontage.h"
#include "AIController.h"
#include "GameFramework/Character.h"

UBTTask_PlayMontage::UBTTask_PlayMontage()
{
	NodeName = "Play Montage Latent";
	
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const MyController = OwnerComp.GetAIOwner();
	EBTNodeResult::Type Result = EBTNodeResult::Failed;

	UAnimMontage* AnimMontage = MontageToPlay.GetValue<UAnimMontage>(OwnerComp);
	
	if (AnimMontage && MyController && MyController->GetPawn())
	{
		if (ACharacter* const MyCharacter = Cast<ACharacter>(MyController->GetPawn()))
		{
			// Get Anim Instance to use delegate for montage finishes
			if (UAnimInstance* AnimInstance = MyCharacter->GetMesh()->GetAnimInstance())
			{
				if (MyCharacter->PlayAnimMontage(AnimMontage) > 0.0F)
				{
					// Used delegate pattern for the montage finish
					FOnMontageEnded MontageEndDelegate;
					MontageEndDelegate.BindUObject(this, &UBTTask_PlayMontage::OnMontageFinished, &OwnerComp);
					// Give delegate and montage object, finishes of AnimMontage will call the delegate
					AnimInstance->Montage_SetEndDelegate(MontageEndDelegate, AnimMontage);
					Result = EBTNodeResult::InProgress;
				}
				else
				{
					// For 0 length montages return succeeded right away
					Result = EBTNodeResult::Succeeded;
				}
			}
		}
	}

	return Result;
}

/** Latent task for montage finishes */
void UBTTask_PlayMontage::OnMontageFinished(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp) const
{
	if (OwnerComp)
	{
		// Tells BT to continue to the next node
		FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}