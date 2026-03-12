// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"
#include "GameFramework/Character.h"

AWeaponActor::AWeaponActor()
{
 	// Instead of update colliding actors in each tick, 
	PrimaryActorTick.bCanEverTick = false;
}

TObjectPtr<UAnimMontage> AWeaponActor::GetComboAttackAt(const unsigned int& Index) const
{
	if (GetComboSequenceLength() > Index)
	{
		return WeaponComboAttacks[Index];
	}
	
	return nullptr;
}