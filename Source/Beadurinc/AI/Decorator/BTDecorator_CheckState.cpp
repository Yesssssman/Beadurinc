// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Decorator/BTDecorator_CheckState.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Actor/Character/FighterCharacter.h"

UBTDecorator_CheckState::UBTDecorator_CheckState()
{
	// Sets a node name in the Behavior Tree
	NodeName = "State Check";
}

bool UBTDecorator_CheckState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AFighterCharacter* AsFighter = Cast<AFighterCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	
	// Exception handle for a case that either owner actor or blackboard invalid 
	if (!AsFighter) return false;
	
	// Used squared distance to avoid root operation
	return !AsFighter->GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(States);
}