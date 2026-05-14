#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/FighterCharacter.h"
#include "GameData/MobData.h"
#include "MonsterCharacter.generated.h"

/**
 * Represent BT state to replace animation in ABP
 * For example, mobs could have different walk animation when patrolling
 * and chasing. The state is determined by BT so won't exist in client
 * side (where ABP animation logic runs)
 * 
 * By using UE Replicate system we synchronize such state 
 */
UENUM(BlueprintType)
enum class EBehaviorTreeState : uint8
{
	Walk UMETA(DisplayName = "Walk"),
	Rush UMETA(DisplayName = "Rush"),
};

UCLASS(abstract)
class BEADURINC_API AMonsterCharacter : public AFighterCharacter
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	AMonsterCharacter();
	
public:

	/** Set the BehaviorTree state replicated to client */
	FORCEINLINE void SetBTState(const EBehaviorTreeState BehaviorTreeState) { BTState = BehaviorTreeState; }

protected:

	/** Grants AI-only abilities (block / parry) on the authoritative side. */
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataAsset, meta = (AllowPrivateAccess = true))
	TObjectPtr<UMobData> MobDataAsset;

	/** A state that will be replicate to client */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = State, meta = (AllowPrivateAccess = true))
	EBehaviorTreeState BTState;

	/** Granted on BeginPlay (authority only). Triggered via Ability.AI.Block gameplay event. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TSubclassOf<UGameplayAbility> AIBlockAbility;

	/** Granted on BeginPlay (authority only). Triggered via Ability.AI.Parry gameplay event. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TSubclassOf<UGameplayAbility> AIParryAbility;

};
