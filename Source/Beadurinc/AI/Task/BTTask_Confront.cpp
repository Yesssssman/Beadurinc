#include "AI/Task/BTTask_Confront.h"

#include "AIController.h"
#include "Actor/Character/FighterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_Confront::UBTTask_Confront()
{
	NodeName = "Confront";
	bNotifyTick = true;
}

void UBTTask_Confront::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	// Checked super function is empty
	// Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	
	if (InitType == EBTMemoryInit::Initialize)
	{
		new (NodeMemory) FBTConfrontMemory();
	}
}

EBTNodeResult::Type UBTTask_Confront::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTConfrontMemory* Memory = CastInstanceNodeMemory<FBTConfrontMemory>(NodeMemory);
	
	// Init per-execution properties (task duration, character move direction)
	Memory->RemainingTime = TimeWaiting + FMath::RandRange(0.0F, TimeWaitingDistribution);
	Memory->MoveDirection = FMath::RandRange(0, 1) > 0 ? FVector(0, -1, 0) : FVector(0, 1, 0);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Confront::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTConfrontMemory* Memory = CastInstanceNodeMemory<FBTConfrontMemory>(NodeMemory);
	Memory->RemainingTime -= DeltaSeconds;
	
	AFighterCharacter* AsFighter = Cast<AFighterCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	
	if (!AsFighter)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (Memory->RemainingTime < 0)
	{
		// Timeout
		AsFighter->GetCharacterMovement()->Velocity = FVector(0, 0, 0);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	
	if (AsFighter->GetAttackTarget())
	{
		if (AsFighter->GetDistanceTo(AsFighter->GetAttackTarget()) < DistanceThreshold)
		{
			// Enemy approached first; force terminate
			AsFighter->GetCharacterMovement()->Velocity = FVector(0, 0, 0);
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
		else
		{
			// Rotate to the target with exponential smoothing
			FRotator DesiredRot = (AsFighter->GetAttackTarget()->GetActorLocation() - AsFighter->GetActorLocation()).Rotation();
			FRotator NewRot = FMath::RInterpTo(AsFighter->GetActorRotation(), DesiredRot, DeltaSeconds, 8.f);
			
			// Fix pitch
			NewRot.Pitch = AsFighter->GetActorRotation().Pitch;
			AsFighter->SetActorRotation(NewRot);
			
			FVector Velocity = NewRot.RotateVector(Memory->MoveDirection);
			
			// Move toward the direction in memory
			AsFighter->AddMovementInput(Velocity, 0.4F);
		}
		
		// In-Progress...
		return;
	}
	
	// No attack target detected: terminate the task
	FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
}
