// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"
#include "GameFramework/Character.h"

// Sets default values
AWeaponActor::AWeaponActor()
{
 	// Instead of update colliding actors in each tick, 
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
}

// Returns a combo attack montage for given index
TObjectPtr<UAnimMontage> AWeaponActor::GetComboAttackAt(const unsigned int& Index) const
{
	if (GetComboSequenceLength() > Index)
	{
		return WeaponComboAttacks[Index];
	}
	
	return nullptr;
}