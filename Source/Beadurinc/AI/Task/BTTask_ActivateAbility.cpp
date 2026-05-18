#include "AI/Task/BTTask_ActivateAbility.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Actor/Character/FighterCharacter.h"

UBTTask_ActivateAbility::UBTTask_ActivateAbility()
{
	NodeName = "Activate Ability";
	bNotifyTick = true;
}

void UBTTask_ActivateAbility::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	if (InitType == EBTMemoryInit::Initialize)
	{
		new (NodeMemory) FBTActivateAbilityMemory();
	}
}

EBTNodeResult::Type UBTTask_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTActivateAbilityMemory* Memory = CastInstanceNodeMemory<FBTActivateAbilityMemory>(NodeMemory);
	Memory->TrackedHandle = FGameplayAbilitySpecHandle();

	if (!AbilityTriggerTag.IsValid()) return EBTNodeResult::Failed;

	AAIController* const AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AFighterCharacter* const AICharacter = Cast<AFighterCharacter>(AIController->GetPawn());
	if (!AICharacter || !AICharacter->GetAbilitySystemComponent()) return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = AICharacter->GetAbilitySystemComponent();

	// Snapshot active specs before firing the event so we can diff to find the newly activated one.
	// AbilityTriggers on UGameplayAbility is protected, so we can't filter by trigger tag directly.
	TSet<FGameplayAbilitySpecHandle> ActiveBefore;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.IsActive()) ActiveBefore.Add(Spec.Handle);
	}

	FGameplayEventData Payload;
	Payload.EventTag = AbilityTriggerTag;
	Payload.Instigator = AICharacter;
	Payload.Target = AICharacter;

	const int32 ActivatedCount = ASC->HandleGameplayEvent(AbilityTriggerTag, &Payload);

	if (ActivatedCount <= 0) return EBTNodeResult::Failed;

	if (!bWaitForAbilityEnd)
	{
		return EBTNodeResult::Succeeded;
	}

	// Pick the first spec that became active as a result of the event.
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.IsActive() && !ActiveBefore.Contains(Spec.Handle))
		{
			Memory->TrackedHandle = Spec.Handle;
			break;
		}
	}

	// Event fired but no spec is currently active (e.g. fire-and-forget ability that ended same frame).
	if (!Memory->TrackedHandle.IsValid()) return EBTNodeResult::Succeeded;

	return EBTNodeResult::InProgress;
}

void UBTTask_ActivateAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	
	
	FBTActivateAbilityMemory* Memory = CastInstanceNodeMemory<FBTActivateAbilityMemory>(NodeMemory);
	if (!Memory || !Memory->TrackedHandle.IsValid())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	AAIController* const AIController = OwnerComp.GetAIOwner();
	if (!AIController) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AFighterCharacter* const AICharacter = Cast<AFighterCharacter>(AIController->GetPawn());
	if (!AICharacter || !AICharacter->GetAbilitySystemComponent())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UAbilitySystemComponent* ASC = AICharacter->GetAbilitySystemComponent();
	
	if (ASC->HasAnyMatchingGameplayTags(ProgressingTags))
	{
		// In Progress...
		return;
	}
	
	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Memory->TrackedHandle);

	// Spec gone or no longer active → ability finished.
	if (!Spec || !Spec->IsActive())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	
	// In Progress...
}