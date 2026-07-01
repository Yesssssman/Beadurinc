#include "GameData/BeadurincGameInstance.h"

#include "ProfileSaveGame.h"
#include "Kismet/GameplayStatics.h"

UBeadurincGameInstance::UBeadurincGameInstance()
{
	static FString GameProfileSlotName = FString(TEXT("GameProfiles"));
	
	if (USaveGame* Profiles = UGameplayStatics::LoadGameFromSlot(GameProfileSlotName, 0))
	{
		GameProfiles = CastChecked<UProfileSaveGame>(Profiles);
	}
	else
	{
		// Initial start
		GameProfiles = CastChecked<UProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UProfileSaveGame::StaticClass()));
		UGameplayStatics::SaveGameToSlot(GameProfiles, GameProfileSlotName, 0);
	}
}

void UBeadurincGameInstance::CreateNewGame(const FString& SlotName, const FString& WorldName)
{
	GameProfiles->PushNewGameProfile(WorldName);
	
	
}

void UBeadurincGameInstance::DeleteSavedGame(const FString& SlotName)
{
	
}

void UBeadurincGameInstance::LoadGame(const FString& SlotName)
{
	
}
