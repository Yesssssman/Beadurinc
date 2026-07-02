#include "GameData/BeadurincGameInstance.h"

#include "FullContextSaveGame.h"
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
	GameProfiles->DeleteSavedProfile(SlotName);
}

void UBeadurincGameInstance::LoadGame(const FString& SlotName)
{
	UFullContextSaveGame* CampaignSave = CastChecked<UFullContextSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	
	// This is a case where game only created but hasn't ever loaded.
	if (!IsValid(CampaignSave))
	{
		CampaignSave = CastChecked<UFullContextSaveGame>(UGameplayStatics::CreateSaveGameObject(UFullContextSaveGame::StaticClass()));
		UGameplayStatics::SaveGameToSlot(CampaignSave, SlotName, 0);
	}
	
	//UGameplayStatics::OpenLevel(nullptr, CampaignSave);
}

void UBeadurincGameInstance::SaveProgression()
{
	if (ActivatedSaveGameSlotName.IsEmpty())
	{
		// No game has loaded. Skip.
		return;
	}
	
	
}
