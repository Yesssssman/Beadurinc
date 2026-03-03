#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "WeaponHolderInterface.generated.h"

class AActor;
class AWeaponActor;

/// Usually characters that grab a weapon in their hand.
/// 
/// this will help get a weapon actor to toggle its collider
/// in notifies, and hook the overlap event to apply damage and stuns.
/// 
/// NAMING RULE MATTERS: In Unreal Engine, Interfaces consist of a stub
/// class that inherits `UInterface` and declared by `UINTERFACE` macro,
/// and the actual interface type that is used in codebase with a name
/// that the first case was replaced from 'U' to 'I' (e.g. UMyInterfcae ->
/// IMyInterface)
/// 
/// https://dev.epicgames.com/documentation/unreal-engine/interfaces?application_version=4.27
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UWeaponHolderInterface : public UInterface
{
	// This is just a stub class to expose Unreal Engine API. The type you'll use
	// is declared in below.
	GENERATED_BODY()
};

class IWeaponHolderInterface
{
	GENERATED_BODY()
public:
	
	/** Returns whether this character is holding a weapon actor */
	virtual bool IsHoldingWeapon() const = 0;
	
	/** Returns a weapon in an actor's main hand */
	virtual TObjectPtr<AWeaponActor> GetWeaponActor() const = 0;
};