// Fill out your copyright notice in the Description page of Project Settings.

#include "BeadurincPlayerState.h"
#include "AbilitySystemComponent.h"

ABeadurincPlayerState::ABeadurincPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

