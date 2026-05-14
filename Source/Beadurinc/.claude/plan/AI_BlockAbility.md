# AI Block / Parry Ability — Implementation Plan

> Plan only. No code will be touched until the user explicitly approves.

## 1. Current state of the codebase (verified by reading every relevant file)

### 1.1 Player reference: `UBlockParryGameplayAbility`

File: `Source/Beadurinc/AbilitySystem/GameplayAbility/BlockParryGameplayAbility.{h,cpp}`

- `InstancingPolicy = InstancedPerActor`.
- `CanActivateAbility` rejects when:
  - `BlockLocomotion` montage is invalid,
  - avatar has `State_BlockingLocked`,
  - avatar is `CharacterMovement->IsFalling()`.
- `ActivateAbility`:
  1. Adds **two** loose tags to the avatar's ASC: `State_Blocking` **and** `State_Parry`.
  2. Plays `BlockLocomotion` directly through `PlayAnimMontage` (not through `AbilityTask_PlayMontageAndWait`).
  3. Starts `UAbilityTask_WaitDelay(0.5s)` → `OnParryWindowFinished` strips only `State_Parry`.
  4. Starts `UAbilityTask_WaitInputRelease` → `OnInputReleased` calls `EndAbility(... bWasCancelled=false)`.
- `EndAbility` stops the montage and strips `State_Blocking` and `State_Parry` if present.

Key point for the AI port: **the parry window is encoded as "first 0.5s the avatar has both tags, after that only `State_Blocking`"**. This is exactly what `UHitReactGameplayAbility::ActivateAbility` consumes — see §1.2.

### 1.2 Consumer side: `UHitReactGameplayAbility`

File: `Source/Beadurinc/AbilitySystem/GameplayAbility/HitReactGameplayAbility.cpp`, line 52‑111.

The block / parry result is decided **entirely from the defender's state tags on the moment of being hit**:

```cpp
if (
    OwnerACS->HasMatchingGameplayTag(StateGameplayTags::State_Blocking) &&
    MetaData->DangerAttackTypeTag != AttackTypeTags::AttackType_LowAttack &&
    (MetaData->DangerAttackTypeTag != AttackTypeTags::AttackType_Pierce ||
     OwnerACS->HasMatchingGameplayTag(StateGameplayTags::State_Parry))
) {
    if (OwnerACS->HasMatchingGameplayTag(StateGameplayTags::State_Parry))
    {
        // parry branch
    }
    else
    {
        // block branch
    }
}
```

Conclusions for the AI ability:
- To **block**, the avatar only needs `State_Blocking`.
- To **parry**, the avatar needs **both** `State_Blocking` **and** `State_Parry` at the moment the hit is registered.
- Tag layering must therefore match the player exactly — no new tags are required.

### 1.3 Existing AI ability stubs

Files (already in tree, currently no‑op):
- `Source/Beadurinc/AbilitySystem/GameplayAbility/AIBlockGameplayAbility.{h,cpp}`
- `Source/Beadurinc/AbilitySystem/GameplayAbility/AIParryGameplayAbility.{h,cpp}`

Both:
- inherit `UGameplayAbility`,
- ctor sets `InstancingPolicy = InstancedPerActor`,
- `ActivateAbility` / `EndAbility` only call `Super::…` — i.e. they do not set any state tag yet,
- header doc-comment already declares the design intent: *"simply triggers the blocking state without any state check. It must be filtered by decorators in Behavior Tree."*

`Config/DefaultEngine.ini:283` has a `ClassRedirects` from `AIBlockParryGameplayAbility` → `AIBlockGameplayAbility`, confirming the split is the intended target shape. There is **no** existing redirect for `AIParryGameplayAbility`, so it can be filled in cleanly.

### 1.4 BT infrastructure already present

- `Source/Beadurinc/AI/Task/BTTask_ActivateAbility.{h,cpp}` — declared with a single property `FGameplayTag AbilityTriggerTag` and a tick callback, but the body is a stub (`return Super::ExecuteTask(...)`). This is the node that must be filled in to actually fire the ability.
- `Source/Beadurinc/AI/Decorator/BTDecorator_Score.{h,cpp}` — does `FMath::FRand() <= ScoreKey/100.0F`. So selection-by-behavior-score is already wired; the BT design just needs separate Block and Parry branches each guarded by its own score decorator.
- `Source/Beadurinc/AI/Service/BTService_SetScore.{h,cpp}` — increments a blackboard score on relevance. Already usable.
- `Source/Beadurinc/AI/Decorator/BTDecorator_CheckState.{h,cpp}` and `BTDecorator_CheckEnemyState.{h,cpp}` — gating by ASC tags on self or target.

### 1.5 Ability granting pipeline

- Player abilities are granted in `ABeadurincPlayerState::BeginPlay` (`Source/Beadurinc/GameData/BeadurincPlayerState.cpp:18‑62`).
- Per-character passive abilities (`HitReactAbility`, `ParryReactAbility`) are granted in `AFighterCharacter::BeginPlay` (`Source/Beadurinc/Actor/Character/FighterCharacter.cpp:23‑61`) using `FGameplayAbilitySpec`. This is the natural place to grant AI-only abilities, but only on the monster subclass (we don't want players to own the AI versions).
- `AMonsterCharacter` (`Source/Beadurinc/Actor/Character/MonsterCharacter.h`) is the right place to expose two new `TSubclassOf<UGameplayAbility>` fields and grant them on `BeginPlay`. Granting them on `AFighterCharacter` would leak the AI ability onto players.
- `EAbilityId` (`Source/Beadurinc/AbilitySystem/AbilityId.h`) currently lists Combo_Attack..Execution. New input IDs are not strictly needed because the AI abilities will be triggered by GameplayEvent (via `AbilityTriggers`), not by an input.

### 1.6 Existing gameplay tags relevant here

`Source/Beadurinc/AbilitySystem/GameplayTag/AbilityTags.{h,cpp}`:
- `Ability.ComboAttack`, `Ability.BlockParry`, `Ability.Roll` are declared.
- Nothing yet for AI-specific block/parry trigger tags.

`Source/Beadurinc/AbilitySystem/GameplayTag/StateGameplayTags.{h,cpp}`:
- `State.Blocking`, `State.Parry`, `State.BlockingLocked`, `State.StaminaRegenCooldown` are all declared and behave as documented.

---

## 2. Design decisions

1. **Keep the AI split (Block / Parry) as the request asks.** Each is its own `UGameplayAbility` so the BT can score each branch independently.
2. **No `CanActivateAbility` override** — per the request, validity is the BT's responsibility (decorators).
3. **Both abilities use `AbilityTriggers`** (configured in the derived Blueprint, not in C++) so the BT can launch them by `SendGameplayEventToActor`. This is the pattern already implied by `BTTask_ActivateAbility::AbilityTriggerTag`. The user-facing wiring is: BP sets `AbilityTriggers[0]` to e.g. `Ability.AI.Block`, the BT task sends that tag.
4. **Add two new tags under `AbilityTags`:** `Ability.AI.Block` and `Ability.AI.Parry`. These are dedicated trigger tags (not state tags). Reusing `Ability.BlockParry` would conflate the player ability and the AI abilities and prevent independent BT routing.
5. **State tags written by the AI abilities are exactly the same ones the player ability writes** — `State_Blocking` for block, `State_Blocking + State_Parry` for parry. This guarantees `HitReactGameplayAbility` reacts identically regardless of attacker.
6. **Duration model** (chosen to match the player's behavior shape, but driven by data on the BP):
   - `UAIBlockGameplayAbility` holds `State_Blocking` for a `BlockDuration` (default e.g. `1.5s`, edit‑defaults‑only), then ends.
   - `UAIParryGameplayAbility` holds **both** `State_Blocking` and `State_Parry` for a `ParryWindow` (default e.g. `0.4s`), then ends.
   - Both use `UAbilityTask_WaitDelay`, matching the existing pattern in `BlockParryGameplayAbility::ActivateAbility`. No input-release task — AI has no input.
7. **Montage:** optional `UAnimMontage* BlockMontage` on each ability, played via `PlayAnimMontage` like the player version. The user can leave it null and rely on ABP state-driven anims.
8. **`BTTask_ActivateAbility` is fleshed out** to:
   - Resolve the controlled pawn's `UAbilitySystemComponent`.
   - Send a gameplay event with `AbilityTriggerTag` (the existing field) via `UAbilitySystemBlueprintLibrary::SendGameplayEventToActor`.
   - Optionally watch for the matching state tag(s) to clear and `FinishLatentTask(Succeeded)` once the ability ends. (See §3.4 for the exact policy decision the user should weigh in on.)
9. **Granting on the monster side:** add two new `TSubclassOf<UGameplayAbility>` fields on `AMonsterCharacter` (`AIBlockAbility`, `AIParryAbility`) and grant them with `FGameplayAbilitySpec` in `AMonsterCharacter::BeginPlay`. No `EAbilityId` entry needed (event‑triggered, not input‑triggered).

---

## 3. Full code snippets (proposed)

### 3.1 New tags — `AbilityTags`

**`Source/Beadurinc/AbilitySystem/GameplayTag/AbilityTags.h`** (add the two `UE_DECLARE_GAMEPLAY_TAG_EXTERN` lines):

```cpp
#pragma once

#include "NativeGameplayTags.h"

namespace AbilityTags
{
    /** GameplayTag definition macro */
    BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( Ability_ComboAttack );
    BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( Ability_BlockParry  );
    BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( Ability_Roll        );
    BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( Ability_AI_Block    );
    BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( Ability_AI_Parry    );
}
```

**`Source/Beadurinc/AbilitySystem/GameplayTag/AbilityTags.cpp`**:

```cpp
#include "AbilityTags.h"

namespace AbilityTags
{
    /** GameplayTag generating macro */
    UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_ComboAttack, "Ability.ComboAttack", "An ability for playing successive weapon swings"        );
    UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_BlockParry,  "Ability.BlockParry",  "An ability for blocking or parrying enemy attacks"      );
    UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_Roll,        "Ability.Roll",        "An ability for rolling ground with short invincible time" );
    UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_AI_Block,    "Ability.AI.Block",    "Trigger tag the BT sends to activate an AI block ability" );
    UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_AI_Parry,    "Ability.AI.Parry",    "Trigger tag the BT sends to activate an AI parry ability" );
}
```

### 3.2 `UAIBlockGameplayAbility`

**`AIBlockGameplayAbility.h`** (replaces current file):

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AIBlockGameplayAbility.generated.h"

/**
 * Monster-side blocking ability. The Behavior Tree is responsible for deciding
 * whether activation is appropriate (via decorators / score). On activation we
 * simply hold State_Blocking for BlockDuration, then end.
 *
 * Trigger from BT: SendGameplayEventToActor with tag AbilityTags::Ability_AI_Block.
 * The trigger is wired in the derived Blueprint via AbilityTriggers.
 */
UCLASS()
class BEADURINC_API UAIBlockGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:

    UAIBlockGameplayAbility();

protected:

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

    /** Callback after BlockDuration elapses */
    UFUNCTION()
    void OnBlockDurationFinished();

private:

    /** Optional locomotion / stance montage. May be null; ABP can drive the pose from State_Blocking instead. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> BlockMontage;

    /** How long the avatar holds State_Blocking before the ability ends. Tuned in BP. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Behavior, meta = (AllowPrivateAccess = "true", ClampMin = "0.05"))
    float BlockDuration = 1.5F;
};
```

**`AIBlockGameplayAbility.cpp`**:

```cpp
#include "AbilitySystem/GameplayAbility/AIBlockGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Actor/Character/FighterCharacter.h"

UAIBlockGameplayAbility::UAIBlockGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAIBlockGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
    if (!Fighter || !Fighter->GetAbilitySystemComponent())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    // Enter blocking state — consumed by UHitReactGameplayAbility on incoming hit.
    Fighter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateGameplayTags::State_Blocking);

    // Optional stance montage. ABP can also drive the pose purely from State_Blocking.
    if (IsValid(BlockMontage)) Fighter->PlayAnimMontage(BlockMontage);

    UAbilityTask_WaitDelay* AT_WaitDelay = UAbilityTask_WaitDelay::WaitDelay(this, BlockDuration);
    AT_WaitDelay->OnFinish.AddDynamic(this, &UAIBlockGameplayAbility::OnBlockDurationFinished);
    AT_WaitDelay->ReadyForActivation();
}

void UAIBlockGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
    if (!Fighter) return;

    if (IsValid(BlockMontage)) Fighter->StopAnimMontage(BlockMontage);

    UAbilitySystemComponent* ASC = Fighter->GetAbilitySystemComponent();
    if (!ASC) return;

    if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Blocking))
    {
        ASC->RemoveLooseGameplayTag(StateGameplayTags::State_Blocking);
    }
}

void UAIBlockGameplayAbility::OnBlockDurationFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

### 3.3 `UAIParryGameplayAbility`

**`AIParryGameplayAbility.h`**:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AIParryGameplayAbility.generated.h"

/**
 * Monster-side parrying ability. Holds State_Blocking + State_Parry for a short
 * window, then ends. The BT decides when to use this vs the plain block ability
 * (via behavior score).
 *
 * Also ends early if the avatar is hit during the window. The defender's
 * UHitReactGameplayAbility is triggered first (via AbilityTriggers), reads the
 * State_Blocking + State_Parry tags, and applies the parry branch. Our
 * UAbilityTask_WaitGameplayEvent listener fires after the triggered ability
 * dispatch — by then HitReact is done consuming the tags, so we can safely
 * EndAbility here.
 *
 * Trigger from BT: SendGameplayEventToActor with tag AbilityTags::Ability_AI_Parry.
 * The trigger is wired in the derived Blueprint via AbilityTriggers.
 */
UCLASS()
class BEADURINC_API UAIParryGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:

    UAIParryGameplayAbility();

protected:

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

    /** Callback after ParryWindow elapses */
    UFUNCTION()
    void OnParryWindowFinished();

    /** Callback when the avatar receives Event.Combat.Hit while parrying. Ends the ability early. */
    UFUNCTION()
    void OnHitReceived(FGameplayEventData Payload);

private:

    /** Optional parry stance montage. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> ParryMontage;

    /** Duration the avatar holds both State_Blocking and State_Parry. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Behavior, meta = (AllowPrivateAccess = "true", ClampMin = "0.05"))
    float ParryWindow = 0.4F;
};
```

**`AIParryGameplayAbility.cpp`**:

```cpp
#include "AbilitySystem/GameplayAbility/AIParryGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/GameplayTag/GameplayEventTags.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Actor/Character/FighterCharacter.h"

UAIParryGameplayAbility::UAIParryGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAIParryGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
    if (!Fighter || !Fighter->GetAbilitySystemComponent())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    UAbilitySystemComponent* ASC = Fighter->GetAbilitySystemComponent();

    // UHitReactGameplayAbility requires BOTH tags to take the parry branch.
    ASC->AddLooseGameplayTag(StateGameplayTags::State_Blocking);
    ASC->AddLooseGameplayTag(StateGameplayTags::State_Parry);

    if (IsValid(ParryMontage)) Fighter->PlayAnimMontage(ParryMontage);

    // Hard time-out: end after the parry window.
    UAbilityTask_WaitDelay* AT_WaitDelay = UAbilityTask_WaitDelay::WaitDelay(this, ParryWindow);
    AT_WaitDelay->OnFinish.AddDynamic(this, &UAIParryGameplayAbility::OnParryWindowFinished);
    AT_WaitDelay->ReadyForActivation();

    // Early-out: end as soon as a hit is consumed during the window so the BT isn't blocked waiting.
    // OnlyTriggerOnce=true so the task auto-cancels itself after a single event.
    UAbilityTask_WaitGameplayEvent* AT_WaitHit = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        GameplayEventTags::Event_Combat_Hit,
        /*OptionalExternalTarget*/ nullptr,
        /*OnlyTriggerOnce*/ true,
        /*OnlyMatchExact*/ false
    );
    AT_WaitHit->EventReceived.AddDynamic(this, &UAIParryGameplayAbility::OnHitReceived);
    AT_WaitHit->ReadyForActivation();
}

void UAIParryGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    AFighterCharacter* Fighter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
    if (!Fighter) return;

    if (IsValid(ParryMontage)) Fighter->StopAnimMontage(ParryMontage);

    UAbilitySystemComponent* ASC = Fighter->GetAbilitySystemComponent();
    if (!ASC) return;

    if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Parry))
    {
        ASC->RemoveLooseGameplayTag(StateGameplayTags::State_Parry);
    }
    if (ASC->HasMatchingGameplayTag(StateGameplayTags::State_Blocking))
    {
        ASC->RemoveLooseGameplayTag(StateGameplayTags::State_Blocking);
    }
}

void UAIParryGameplayAbility::OnParryWindowFinished()
{
    if (!IsActive()) return;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAIParryGameplayAbility::OnHitReceived(FGameplayEventData Payload)
{
    // HitReact (triggered via AbilityTriggers) has already run by the time this generic
    // listener fires, so the State_Blocking + State_Parry tags have been consumed for
    // routing. Safe to tear down here and free the BT.
    if (!IsActive()) return;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
```

### 3.4 `BTTask_ActivateAbility`

The existing stub already has the right field shape. Two finish policies are possible:

- **(A) Fire-and-finish** — task succeeds immediately after sending the event. Simpler; BT moves on while the ability runs in parallel.
- **(B) Latent** — task stays InProgress until the ability ends (detected by tag-removed callback or by listening to `OnGameplayAbilityEndedWithData`). Lets the BT branch be "wait for the block to finish before doing anything else", which matches `BTTask_AttackMove`'s pattern.

Recommendation: **(B)**, because Block/Parry both have a fixed duration and the BT will typically want to wait. The same node serves both abilities.

**`BTTask_ActivateAbility.h`** (small additions: notify on tick already true, add a node memory struct, add an optional `bWaitForEnd`):

```cpp
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpecHandle.h"
#include "BTTask_ActivateAbility.generated.h"

struct FBTActivateAbilityMemory
{
    /** Spec handle assigned to the activation we triggered, used to wait for End. */
    FGameplayAbilitySpecHandle TrackedHandle;
    /** Set true once we have observed the ability end. */
    bool bEnded = false;
};

UCLASS()
class BEADURINC_API UBTTask_ActivateAbility : public UBTTaskNode
{
    GENERATED_BODY()

public:

    UBTTask_ActivateAbility();

protected:

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTActivateAbilityMemory); }

private:

    /** Gameplay tag sent as the trigger event (matched by the ability's AbilityTriggers). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Ability", meta = (AllowPrivateAccess = true))
    FGameplayTag AbilityTriggerTag;

    /** If true, the task stays InProgress until the activated ability ends. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Ability", meta = (AllowPrivateAccess = true))
    bool bWaitForAbilityEnd = true;
};
```

**`BTTask_ActivateAbility.cpp`** (replacing the current stub body):

```cpp
#include "AI/Task/BTTask_ActivateAbility.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/Character/FighterCharacter.h"

UBTTask_ActivateAbility::UBTTask_ActivateAbility()
{
    NodeName = "Activate Ability";
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FBTActivateAbilityMemory* Memory = CastInstanceNodeMemory<FBTActivateAbilityMemory>(NodeMemory);
    *Memory = FBTActivateAbilityMemory{};

    AAIController* const AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    AFighterCharacter* const AICharacter = Cast<AFighterCharacter>(AIController->GetPawn());
    if (!AICharacter || !AICharacter->GetAbilitySystemComponent()) return EBTNodeResult::Failed;

    if (!AbilityTriggerTag.IsValid()) return EBTNodeResult::Failed;

    FGameplayEventData Payload;
    Payload.EventTag = AbilityTriggerTag;
    Payload.Instigator = AICharacter;
    Payload.Target = AICharacter;

    const int32 ActivatedCount = UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        AICharacter, AbilityTriggerTag, Payload);

    if (ActivatedCount <= 0) return EBTNodeResult::Failed;

    if (!bWaitForAbilityEnd)
    {
        return EBTNodeResult::Succeeded;
    }

    // Capture the most recently activated spec handle matching the trigger tag.
    UAbilitySystemComponent* ASC = AICharacter->GetAbilitySystemComponent();
    for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
    {
        if (Spec.Ability && Spec.IsActive() &&
            Spec.Ability->AbilityTriggers.ContainsByPredicate(
                [&](const FAbilityTriggerData& T){ return T.TriggerTag == AbilityTriggerTag; }))
        {
            Memory->TrackedHandle = Spec.Handle;
            break;
        }
    }

    // If we couldn't find an active spec, the event still fired — treat as immediate success.
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
    { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    UAbilitySystemComponent* ASC = AICharacter->GetAbilitySystemComponent();
    const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Memory->TrackedHandle);

    // Spec gone or no longer active → ability finished.
    if (!Spec || !Spec->IsActive())
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
```

> Note on `CastInstanceNodeMemory`: this is the standard UE5 helper; if the engine variant in use does not expose it, the equivalent is `reinterpret_cast<FBTActivateAbilityMemory*>(NodeMemory)` plus a placement-new in `InitializeMemory`. I'll match whichever helper is already used elsewhere in `BTTask_AttackMove` once we begin implementation.

### 3.5 Granting on the monster

**`Source/Beadurinc/Actor/Character/MonsterCharacter.h`** — add two protected `TSubclassOf<UGameplayAbility>` fields:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Actor/Character/FighterCharacter.h"
#include "GameData/MobData.h"
#include "MonsterCharacter.generated.h"

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

    AMonsterCharacter();

    FORCEINLINE void SetBTState(const EBehaviorTreeState BehaviorTreeState) { BTState = BehaviorTreeState; }

protected:

    /** Sets up AI-only abilities (block / parry) on the authoritative side. */
    virtual void BeginPlay() override;

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DataAsset, meta = (AllowPrivateAccess = true))
    TObjectPtr<UMobData> MobDataAsset;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = State, meta = (AllowPrivateAccess = true))
    EBehaviorTreeState BTState;

    /** Granted on BeginPlay (authority only). Triggered via Ability.AI.Block gameplay event. */
    UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
    TSubclassOf<UGameplayAbility> AIBlockAbility;

    /** Granted on BeginPlay (authority only). Triggered via Ability.AI.Parry gameplay event. */
    UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
    TSubclassOf<UGameplayAbility> AIParryAbility;
};
```

**`MonsterCharacter.cpp` `BeginPlay`** — add the grant block after the existing constructor body (the file currently doesn't override `BeginPlay`; we'll add one that calls `Super::BeginPlay()` first):

```cpp
void AMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystemComponent && HasAuthority())
    {
        // IsValid guards both null and pending-kill class refs (TSubclassOf → UClass*).
        if (IsValid(AIBlockAbility))
        {
            FGameplayAbilitySpec Spec(AIBlockAbility, 1, INDEX_NONE, this);
            AbilitySystemComponent->GiveAbility(Spec);
        }
        if (IsValid(AIParryAbility))
        {
            FGameplayAbilitySpec Spec(AIParryAbility, 1, INDEX_NONE, this);
            AbilitySystemComponent->GiveAbility(Spec);
        }
    }
}
```

> The InputID is `INDEX_NONE` because these abilities are activated by GameplayEvent, not input. They still need an `AbilityTriggers` entry in the BP (TriggerTag = `Ability.AI.Block` / `Ability.AI.Parry`, TriggerSource = `GameplayEvent`) — that's the BP-side work the user owns.

---

## 4. User-side checklist (handled by you, not by the C++ code)

1. Create `BP_AIBlockAbility` (parent: `UAIBlockGameplayAbility`).
   - In **Class Defaults → Triggers**, add one row: TriggerTag = `Ability.AI.Block`, TriggerSource = `GameplayEvent`.
   - Set `BlockMontage` and `BlockDuration` to taste.
2. Create `BP_AIParryAbility` (parent: `UAIParryGameplayAbility`).
   - Triggers row: `Ability.AI.Parry` / `GameplayEvent`.
   - Set `ParryMontage` and `ParryWindow` (parry should usually be tighter than block).
3. On the monster Blueprint (e.g. `BP_AncientKingCharacter`), set:
   - `AIBlockAbility = BP_AIBlockAbility`
   - `AIParryAbility = BP_AIParryAbility`
4. In the Behavior Tree:
   - Block branch: `BTDecorator_Score (BlockScore)` → `BTService_SetScore (BlockScore)` → `BTTask_ActivateAbility (AbilityTriggerTag = Ability.AI.Block, bWaitForAbilityEnd = true)`.
   - Parry branch: same shape but with `Ability.AI.Parry`.
   - Optionally a `BTDecorator_CheckEnemyState` checking the player is `State.Attacking` to gate the whole subtree.

---

## 5. Open questions — resolved

1. **Trigger tag naming** — ✅ Resolved: add `Ability.AI.Block` and `Ability.AI.Parry` as new dedicated trigger tags (§3.1).
2. **`BTTask_ActivateAbility` finish policy** — ✅ Resolved: option **(B)** latent. Spec-handle tracking + tick polling as shown in §3.4. `bWaitForAbilityEnd` defaults to `true`.
3. **Where to grant the abilities** — ✅ Resolved: `AMonsterCharacter::BeginPlay` (§3.5). Guard each slot with `IsValid(...)` so the base class can stay abstract-friendly and monsters that don't want block/parry simply leave the fields null.
4. **Stamina cooldown on block/parry** — ✅ Resolved: the AI abilities do **not** call `ApplyStaminaRegenCooldown`. Stamina cost (and the regen-block) is applied by `UHitReactGameplayAbility` on hit, which is shared between players and mobs, so behavior matches the player path automatically.
5. **AI parry hit-cancel** — ✅ Resolved: implement. §3.3 now schedules a `UAbilityTask_WaitGameplayEvent` for `Event.Combat.Hit` alongside the parry-window `UAbilityTask_WaitDelay`. The hit event triggers HitReact first (via `AbilityTriggers`) so the parry routing still happens, then the listener fires and the parry ability ends, freeing the BT.

## Open question answering

1. Add the new tag names, `Ability.AI.Block` and `Ability.AI.Parry`.
2. Yup, (B) looks better in my eyes.
3. `AMonsterCharacter::BeginPlay`, but check `IsValid` for block and parry ability classes.
4. Mobs has the same hit react from player, the stamina regen block is also done in there.
5. That makes more sense and we can prevent over-waitings by manually cancel the ability on hit. Implement the process also.

---

## 6. Summary of files that will change

| File | Change |
|------|--------|
| `Source/Beadurinc/AbilitySystem/GameplayTag/AbilityTags.h` | +2 `UE_DECLARE_GAMEPLAY_TAG_EXTERN` |
| `Source/Beadurinc/AbilitySystem/GameplayTag/AbilityTags.cpp` | +2 `UE_DEFINE_GAMEPLAY_TAG_COMMENT` |
| `Source/Beadurinc/AbilitySystem/GameplayAbility/AIBlockGameplayAbility.h` | Replace stub |
| `Source/Beadurinc/AbilitySystem/GameplayAbility/AIBlockGameplayAbility.cpp` | Replace stub |
| `Source/Beadurinc/AbilitySystem/GameplayAbility/AIParryGameplayAbility.h` | Replace stub |
| `Source/Beadurinc/AbilitySystem/GameplayAbility/AIParryGameplayAbility.cpp` | Replace stub |
| `Source/Beadurinc/AI/Task/BTTask_ActivateAbility.h` | Add memory struct + `bWaitForAbilityEnd` |
| `Source/Beadurinc/AI/Task/BTTask_ActivateAbility.cpp` | Replace stub body |
| `Source/Beadurinc/Actor/Character/MonsterCharacter.h` | +2 ability subclass fields, override `BeginPlay` |
| `Source/Beadurinc/Actor/Character/MonsterCharacter.cpp` | Implement `BeginPlay` to grant the two abilities |

No file is deleted. No existing player ability is modified.