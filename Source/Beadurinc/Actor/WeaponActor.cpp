// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"

AWeaponActor::AWeaponActor()
{
 	// Instead of update colliding actors in each tick, 
	PrimaryActorTick.bCanEverTick = false;
}

TObjectPtr<UAnimMontage> AWeaponActor::GetComboAttackAt(const unsigned int& Index) const
{
	// Index guard
	if (Index < 0 || GetComboSequenceLength() <= Index) return nullptr;
	
	return WeaponComboAttacks[Index];
}