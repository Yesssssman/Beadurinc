#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponActor.generated.h"

class UGameplayEffect;
class ACharacter;

UCLASS()
class BEADURINC_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:
	
	// Sets default values for this actor's properties
	AWeaponActor();
	
public:
	
	// Returns a combo attack montage for given index
	TObjectPtr<UAnimMontage> GetComboAttackAt(const unsigned int& Index) const;
	
	FORCEINLINE uint32 GetComboSequenceLength() const { return WeaponComboAttacks.Num(); };
	
	FORCEINLINE float GetStaminaDamage() const { return StaminaDamage; };
	
	FORCEINLINE float GetHealthDamage() const { return HealthDamage; };
	
private:
	
	/** Combo attack sequence for owner */
	UPROPERTY(EditAnywhere, Category="Animation", meta=(AllowPrivateAccess=true))
	TArray<TObjectPtr<UAnimMontage>> WeaponComboAttacks;
	
	/** Damage deal to health when hit other actors */
	UPROPERTY(EditAnywhere, Category="Attribute", meta=(AllowPrivateAccess=true))
	float HealthDamage;
	
	/** Damage deal to stamina when hit other actors */
    UPROPERTY(EditAnywhere, Category="Attribute", meta=(AllowPrivateAccess=true))
    float StaminaDamage;
};