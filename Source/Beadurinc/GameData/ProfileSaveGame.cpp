#include "GameData/ProfileSaveGame.h"

#include "FullContextSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UProfileSaveGame::PushNewGameProfile(FString WorldName)
{
	Seq++;
	
	UFullContextSaveGame* FullContextSaveGame = CastChecked<UFullContextSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UFullContextSaveGame::StaticClass())
	);
	
	UGameplayStatics::SaveGameToSlot(GameProfiles, GameProfileSlotName, 0);
}

void UProfileSaveGame::DeleteProfileAt(const int32& Index)
{
	
}
