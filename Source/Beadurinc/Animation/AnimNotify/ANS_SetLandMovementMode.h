// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_SetLandMovementMode.generated.h"

UCLASS()
class BEADURINC_API UANS_SetLandMovementMode : public UAnimNotifyState
{
	GENERATED_BODY()
	
protected:
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
private:
	
	/** Movement mode to set while the notify state is activated */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup, meta = (AllowPrivateAccess = true))
	TEnumAsByte<EMovementMode> MovementModeToChange;
	
	/** Movement mode to go back when the notify state ends */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup, meta = (AllowPrivateAccess = true))
	TEnumAsByte<EMovementMode> MovementModeToRevert;
};
