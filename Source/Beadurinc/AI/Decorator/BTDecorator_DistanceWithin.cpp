// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_DistanceWithin.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"

/** Constructor */
UBTDecorator_DistanceWithin::UBTDecorator_DistanceWithin()
{
	// Sets a node name in the Behavior Tree
	NodeName = "Distance Check";
	
	// Seta a type of object key to `ACharacter`
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_DistanceWithin, TargetActorKey), ACharacter::StaticClass());
}

/** Returns a check result of decorator */
bool UBTDecorator_DistanceWithin::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const TObjectPtr<APawn> BTOwner = OwnerComp.GetAIOwner()->GetPawn();
	const UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	
	// Exception handle for a case that either owner actor or blackboard invalid 
	if (!BTOwner || !BBComp) return false; 
	
	const ACharacter* Target = CastChecked<ACharacter>(BBComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	
	// In case target is null (should be filtered by root node)
	if (!Target) return false;
	
	double DistanceSqr = BTOwner->GetSquaredDistanceTo(Target);
	
	// Used squared distance to avoid root operation
	return DistanceSqr <= Max * Max && Min * Min <= DistanceSqr;
}