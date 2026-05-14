#include "AI/Task/BTTask_AttackMove.h"

#include "AIController.h"
#include "AbilitySystem/GameplayTag/AttackTypeTags.h"
#include "Actor/Character/FighterCharacter.h"
#include "Animation/Metadata/AttackMetaData.h"
#include "EnhancedInput/BeadurincPlayerController.h"

const FGameplayTagContainer UBTTask_AttackMove::DangerAttackTypes = FGameplayTagContainer::CreateFromArray(
	TArray<FGameplayTag>({ AttackTypeTags::AttackType_Pierce, AttackTypeTags::AttackType_LowAttack })
);

UBTTask_AttackMove::UBTTask_AttackMove()
{
	NodeName = "Attack Move";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_AttackMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Checked the super function is empty
	//Super::ExecuteTask(OwnerComp, NodeMemory);
	
	AAIController* const AIController = OwnerComp.GetAIOwner();
	UAnimMontage* AnimMontage = MontageToPlay.GetValue<UAnimMontage>(OwnerComp);
	if (!AnimMontage || !AIController) return EBTNodeResult::Failed;
	
	AFighterCharacter* const AICharacter = Cast<AFighterCharacter>(AIController->GetPawn());
	if (!AICharacter) return EBTNodeResult::Failed;
	
	UAnimInstance* AnimInstance = AICharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return EBTNodeResult::Failed;
	
	// Block the stamina regen
	AICharacter->ApplyStaminaRegenCooldown();
	
	// Play montage (unsafe)
	PlayMontageMulticast(AICharacter, AnimMontage);
	
	// If found any "critical" type like pierce or low attacks, alert to the target if it's a player.
	if (AICharacter->GetAttackTarget())
	{
		if (ABeadurincPlayerController* PlayerController = Cast<ABeadurincPlayerController>(AICharacter->GetAttackTarget()->GetController()))
		{
			// Examine metadata and find attack type
			for (UAnimMetaData* ComboMetaData : AnimMontage->GetMetaData())
			{
				if (UAttackMetaData* AttackAnimMetaData = Cast<UAttackMetaData>(ComboMetaData))
				{
					if (AttackAnimMetaData->DangerAttackTypeTag != FGameplayTag::EmptyTag)
					{
						PlayerController->AlertCriticalAttacks(AttackAnimMetaData->DangerAttackTypeTag);
						break;
					}
				}
			}
		}
	}
	
	// Always returns in-progress, the task finish time will be judged by `TickTask`
	return EBTNodeResult::InProgress;
}

void UBTTask_AttackMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	EBTNodeResult::Type NodeState = ValidateOwnerState(OwnerComp);
	
	if (NodeState != EBTNodeResult::InProgress)
	{
		FinishLatentTask(OwnerComp, NodeState);
	}
}

/** Validate and return node result type based on GameplayTags */
EBTNodeResult::Type UBTTask_AttackMove::ValidateOwnerState(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* const AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	AFighterCharacter* const AICharacter = Cast<AFighterCharacter>(AIController->GetPawn());
	if (!AICharacter) return EBTNodeResult::Failed;
	
	if (FAnimMontageInstance* ActiveMontageInstance = AICharacter->GetMesh()->GetAnimInstance()->GetActiveMontageInstance())
	{
		// When elapsed time less than blend time: wait until notify state set
		if (ActiveMontageInstance->GetPosition() < ActiveMontageInstance->GetBlendTime())
		{
			return EBTNodeResult::InProgress;
		}
	}
	
	UAbilitySystemComponent* ASC = AICharacter->GetAbilitySystemComponent();
	if (!ASC) return EBTNodeResult::Failed;
	
	// Found any failure tags: failed right away
	if (ASC->HasAnyMatchingGameplayTags(FailureTags))
	{
		return EBTNodeResult::Failed;
	}
	
	if (ASC->HasAnyMatchingGameplayTags(InProgressTags))
	{
		return EBTNodeResult::InProgress;
	}
	
	return EBTNodeResult::Succeeded;
}

void UBTTask_AttackMove::PlayMontageMulticast_Implementation(ACharacter* CharacterIn, UAnimMontage* MontageIn)
{
	if (CharacterIn->GetMesh() && CharacterIn->GetMesh()->GetAnimInstance())
	{
		CharacterIn->PlayAnimMontage(MontageIn);
	}
}
