#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "BTTask_PlayMontageLatent.generated.h"

UCLASS()
class BEADURINC_API UBTTask_PlayMontageLatent : public UBTTaskNode
{
	GENERATED_BODY()

public:

	/** Constructor */
	UBTTask_PlayMontageLatent();

protected:

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	/** Latent task for montage finishes */
	void OnMontageFinished(UAnimMontage* MontageIn, bool bInterrupted, UBehaviorTreeComponent* OwnerComp) const;
	
	UFUNCTION(NetMulticast, Reliable)
	void PlayMontageMulticast(ACharacter* CharacterIn, UAnimMontage* MontageIn);
	
private:
	/** Animation Montage to play. Must compatible with the skeleton that BT owner has */
	UPROPERTY(Category = Animation, EditAnywhere)
	FValueOrBBKey_Object MontageToPlay = TObjectPtr<UAnimMontage>();
	
	/** If set true, the node ends when montage finishes */
	UPROPERTY(Category = Animation, EditAnywhere)
	bool StatefulProgress;
};