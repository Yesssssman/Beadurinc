// Fill out your copyright notice in the Description page of Project Settings.


#include "AncientKingCharacter.h"
#include "Actor/WeaponActor.h"

// Sets default values
AAncientKingCharacter::AAncientKingCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAncientKingCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(GetMesh()) && IsValid(MeleeWeapon))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = GetInstigator();
		
		// Spawn a weapon actor
		AWeaponActor* SpawnedWeaponActor = GetWorld()->SpawnActor<AWeaponActor>(
			MeleeWeapon,
			SpawnParameters
		);
		
		SpawnedWeaponActor->SetActorEnableCollision(false);
		
		// Attach the spawned actor to a bone socket
		SpawnedWeaponActor->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("Right_Forehand_Socket")
		);
	}
}

// Called every frame
void AAncientKingCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAncientKingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

