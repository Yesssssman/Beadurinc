#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "BeadurincPlayerController.generated.h"

class AFighterCharacter;
class UInputMappingContext;
class UUserWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class ABeadurincPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Plays a UI sound via RPC */
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void PlayLocalSound(USoundBase* SoundToPlay, const float VolumeModifier = 1.0F);

	/**
	 * Adds a boss stat bar
	 * @param DisplayedBoss A boss character that will bound to widget
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void DisplayBossBar(const AFighterCharacter* DisplayedBoss);

	/**
	 * Removes a boss stat bar
	 * @param DisconnectedBoss A boss character who will be removed from boss bars
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void CloseBossBar(const AFighterCharacter* DisconnectedBoss);

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Called on begin play with spectating mode */
	virtual void BeginSpectatingState() override;
	
	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** On the controller starts possessing player character in server */
	virtual void OnPossess(APawn* InPawn) override;

	/** Replicate function when the controller starts possessing player character in server */
	virtual void OnRep_Pawn() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

public:
	
	/**
	 * Display a warning widget on the top of the player, that indicates an incoming critical
	 * attacks (pierce, low attack)
	 * 
	 * @param AttackType
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void AlertCriticalAttacks(const FGameplayTag& AttackType);
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;
};