#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetRepState.generated.h"

enum class EBehaviorTreeState : uint8;

UCLASS()
class BEADURINC_API UBTService_SetRepState : public UBTService
{
	GENERATED_BODY()
	
public:
	
	UBTService_SetRepState();
	
private:
	
	/** A state when the node becomes relevant */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = State, meta = (AllowPrivateAccess = true))
	EBehaviorTreeState StateOnRelevant;
	
	/** A state when the node being ceased */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = State, meta = (AllowPrivateAccess = true))
	EBehaviorTreeState StateOnCease;
	
protected:
	
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
