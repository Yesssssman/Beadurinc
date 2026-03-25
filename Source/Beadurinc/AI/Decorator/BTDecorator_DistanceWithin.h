// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_DistanceWithin.generated.h"

/**
 * 
 */
UCLASS()
class BEADURINC_API UBTDecorator_DistanceWithin : public UBTDecorator
{
	GENERATED_BODY()

	/** The Blackboard key for target actor */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector TargetActorKey;

	/** The threshold distance to judge if target is within boundary */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (AllowPrivateAccess = true))
	float Within = 100.0F;

public:
	
	/** Constructor */
	UBTDecorator_DistanceWithin();
	
protected:
	
	/** Returns a check result of decorator */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
