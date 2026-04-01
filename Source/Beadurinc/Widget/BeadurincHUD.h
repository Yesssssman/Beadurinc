// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BeadurincHUD.generated.h"

class APlayerCharacter;
class AFighterCharacter;

/**
 * 
 */
UCLASS(abstract)
class BEADURINC_API ABeadurincHUD : public AHUD
{
	GENERATED_BODY()

private:
	/**
	 * A target indicator when player activates Camera Lock-on feature
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	UUserWidget* LockOnIndicator;

	/**
	 * A stackable widget container that displays boss stats in progression bar
	 * 
	 * Called on boss characters perceives the player and removed when the boss
	 * no more sensing the player.
	 */
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	UUserWidget* BossBarLayout;

	/**
	 * Main player character
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<APlayerCharacter> PlayerCharacter;
	
public:
	
	/**
	 * Adds a boss stat bar to widget
	 * @param BossActor A boss character that will bound to widget
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void AddBossEntry(const AFighterCharacter* BossActor);

	/**
	 * Removes a boss stat bar from widget
	 * @param BossActor A boss character who will be removed from boss bars
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void RemoveBossEntry(const AFighterCharacter* BossActor);

	/**
	 * Initialize the player character
	 * Called on PlayerController starts possessing character
	 */
	void InitPlayer(const TObjectPtr<APlayerCharacter> SpawnedPlayer);
};
