// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/FighterCharacter.h"
#include "GameData/MobData.h"
#include "MonsterCharacter.generated.h"

UCLASS(abstract)
class BEADURINC_API AMonsterCharacter : public AFighterCharacter
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataAsset, meta = (AllowPrivateAccess = true))
	TObjectPtr<UMobData> MobDataAsset;
	
};
