// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Task/BTTask_PlayMontage.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Actor/Character/FighterCharacter.h"
#include "GameFramework/Character.h"

UBTTask_PlayMontage::UBTTask_PlayMontage()
{
	NodeName = "Play Montage Latent";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const AIController = OwnerComp.GetAIOwner();
	UAnimMontage* AnimMontage = MontageToPlay.GetValue<UAnimMontage>(OwnerComp);
	AFighterCharacter* const AICharacter = Cast<AFighterCharacter>(AIController->GetPawn());
	
	if (!AnimMontage || !AIController || !AICharacter)
	{
		return EBTNodeResult::Failed;
	}
	
	UAnimInstance* AnimInstance = AICharacter->GetMesh()->GetAnimInstance();
	
	if (!AnimInstance)
	{
		return EBTNodeResult::Failed;
	}
	
	// Get Anim Instance to use delegate for montage finishes
	if (AICharacter->PlayAnimMontage(AnimMontage) > 0.0F)
	{
		// Used delegate pattern for the montage finish
		FOnMontageEnded MontageEndDelegate;
		MontageEndDelegate.BindUObject(this, &UBTTask_PlayMontage::OnMontageFinished, &OwnerComp);
		// Give delegate and montage object, finishes of AnimMontage will call the delegate
		AnimInstance->Montage_SetBlendingOutDelegate(MontageEndDelegate, AnimMontage);
		
		AICharacter->GetAbilitySystemComponent()->AddLooseGameplayTags(FinishConditionStates);
		
		return EBTNodeResult::InProgress;
	}
	
	// For 0 length montages return succeeded right away
	return EBTNodeResult::Succeeded;
}

/** Check whether all gameplay tags don't exist, then terminate the task */
void UBTTask_PlayMontage::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (OwnerComp.GetActiveNode() != this) return;
	
	AAIController* const AIController = OwnerComp.GetAIOwner();
	AFighterCharacter* AsFighter = Cast<AFighterCharacter>(AIController->GetPawn());
	if (!AsFighter) return;
	
	UAbilitySystemComponent* ACS = AsFighter->GetAbilitySystemComponent();
	if (!ACS) return;
	
	if (ACS->HasAnyMatchingGameplayTags(FinishConditionStates)) return;
	
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

/** Latent task for montage finishes */
void UBTTask_PlayMontage::OnMontageFinished(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp) const
{
	// Prevents the node being success if the montage is interrupted by another way
	if (OwnerComp && OwnerComp->GetActiveNode() == this)
	{
		// Tells BT to continue to the next node
		FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}