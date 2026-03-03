// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"
#include "WeaponActor.generated.h"

class ACharacter;

UCLASS()
class BEADURINC_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
	/** Combo attack sequence for owner */
	UPROPERTY(EditAnywhere, Category="Animation", meta=(AllowPrivateAccess=true))
	TArray<TObjectPtr<UAnimMontage>> WeaponComboAttacks;
	
	/** Damage attribute provided to the player when equipping */
	UPROPERTY(EditAnywhere, Category="Attribute", meta=(AllowPrivateAccess=true))
	float WeaponBaseDamage;
	
public:	
	// Sets default values for this actor's properties
	AWeaponActor();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:
	// Returns a combo attack montage for given index
	TObjectPtr<UAnimMontage> GetComboAttackAt(const unsigned int& Index) const;
	
	FORCEINLINE uint32 GetComboSequenceLength() const { return WeaponComboAttacks.Num(); };
	
	FORCEINLINE float GetWeaponBaseDamage() const { return WeaponBaseDamage; };
};
