#include "FighterCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GameplayTag/GameplayEventTags.h"
#include "Components/CapsuleComponent.h"

void AFighterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Spawn a weapon actor and attach to the hand
	if (GetMesh() && WeaponActorBlueprint)
	{
		// Instigator != Owner under certain cases
		// e.g. Player shots an arrow using bow: Arrow's owner == bow, but arrow's instigator == player
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = this;
		ActorSpawnParameters.Instigator = GetInstigator();
		
		if (AWeaponActor* WeaponActor = GetWorld()->SpawnActor<AWeaponActor>(WeaponActorBlueprint, ActorSpawnParameters))
		{
			// Initialize main hand weapon actor
			WeaponActorInstance = WeaponActor;
			
			if (UCapsuleComponent* CapsuleCollider = WeaponActorInstance->FindComponentByClass<UCapsuleComponent>())
			{
				// Gives "NoCollision" at first since weapon only can hurt other characters when activated by anim notify.
				CapsuleCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				CapsuleCollider->OnComponentBeginOverlap.AddDynamic(this, &AFighterCharacter::OnMeleeContacts);
			}
			
			WeaponActorInstance->SetActorEnableCollision(false);
			
			// Attach a weapon actor to a skeleton socket
			WeaponActor->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				TEXT("Weapon_Socket")
			);
		}
	}
}

void AFighterCharacter::OnMeleeContacts
(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	AFighterCharacter* OtherFighter = Cast<AFighterCharacter>(OtherActor);
	
	// Terminate when the other actor is myself or not implementation of AFightCharacter.
	if (!OtherFighter || OtherFighter == this)
	{
		return;	
	}
	
	// Prevents "multiple hits" by checking whether the other actor is registered in HitActor container.
	if (HitActors.Contains(OtherActor))
	{
		return;
	}
	
	// Payload to contain data that is used in triggering hurt event 
	FGameplayEventData EventContext;
	
	// Fill the context
	EventContext.Instigator = this;
	EventContext.Target = OtherActor;
	EventContext.OptionalObject = GetWeaponActor();
	EventContext.EventMagnitude = GetWeaponActor()->GetWeaponBaseDamage();
	
	// Calculates the first hit location by the ray trace result calculated by "my collider center -> opponent's collider center"
	FHitResult PreciseHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	GetWorld()->LineTraceSingleByChannel(
		PreciseHit, 
		OverlappedComponent->GetComponentLocation(),
		OtherComp->GetComponentLocation(), 
		ECC_Pawn,
		QueryParams
	);
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(PreciseHit);
	TargetDataHandle.Add(NewData);
	// Store the line tracing result to event context (this is used by creating Sound Cue and particles)
	EventContext.TargetData = TargetDataHandle;
	
	// Trigger the event
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, GameplayEventTags::Event_Combat_Hit, EventContext);
}

void AFighterCharacter::ResetMeleeSwing()
{
	// Clear hit actors saved during melee track phase
	HitActors.Empty();
}