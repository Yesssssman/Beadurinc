#include "GameData/ProfileSaveGame.h"

#include "FullContextSaveGame.h"
#include "Kismet/GameplayStatics.h"

FString& UProfileSaveGame::PushNewGameProfile(const FString& WorldName)
{
	Seq++;
	
	UFullContextSaveGame* FullContextSaveGame = CastChecked<UFullContextSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UFullContextSaveGame::StaticClass())
	);
	
	FullContextSaveGame->RecentWorld = FString("Lobby");
	
	FString SlotName = FString("Campaign" + Seq);
	FullContextSaveGame->SaveSlotName = SlotName;
	
	UGameplayStatics::SaveGameToSlot(FullContextSaveGame, SlotName, 0);
}

void UProfileSaveGame::DeleteSavedProfile(const FString& SaveSlotName)
{
	for (FGameProfile& Profile : Profiles)
	{
		if (Profile.SaveSlotName.Equals(SaveSlotName))
		{
			UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
		}
	}
}
