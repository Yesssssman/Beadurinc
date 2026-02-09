// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AncientKingCharacter.generated.h"

class AWeaponActor;

UCLASS()
class BEADURINC_API AAncientKingCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Equipments, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeaponActor> MeleeWeapon;

public:
	// Sets default values for this character's properties
	AAncientKingCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
