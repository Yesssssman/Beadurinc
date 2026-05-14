#pragma once

#include "CoreMinimal.h"
#include "FighterCharacter.h"
#include "MonsterCharacter.h"
#include "AncientKingCharacter.generated.h"

UCLASS()
class BEADURINC_API AAncientKingCharacter : public AMonsterCharacter
{
	GENERATED_BODY()

public:

	// Sets default values for this character's properties
	AAncientKingCharacter();

protected:

	virtual void BeginPlay() override;
	
public:

	/** Returns currently targeting actor by AI Controller */
	virtual TObjectPtr<ACharacter> GetAttackTarget() override;

private:
	/** Attribute Set Class */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Abilities")
	TSubclassOf<UAttributeSet> AttributeSetClass;

	/** Data Table for initializing AttributeSet data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Asset", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> InitialStatsTable;
};