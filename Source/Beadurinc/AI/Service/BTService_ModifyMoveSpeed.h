// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ModifyMoveSpeed.generated.h"

/**
 * 
 */
UCLASS()
class BEADURINC_API UBTService_ModifyMoveSpeed : public UBTService
{
	GENERATED_BODY()
	
public:
	
	UBTService_ModifyMoveSpeed();
	
private:
	
	/** Max walk speed addition */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = State, meta = (AllowPrivateAccess = true))
	float SpeedAddition;
	
protected:
	
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
