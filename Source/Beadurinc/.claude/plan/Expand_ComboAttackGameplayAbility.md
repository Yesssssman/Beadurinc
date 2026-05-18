# Expand Combo Attack — Dash & Heavy — Implementation Plan (revised)

> Plan only. No code will be touched until the user explicitly approves.
>
> This revision folds the user's answers in `## Open Question Answers:` (preserved at the bottom) and addresses the two follow-up questions in `## My open questions:`.

## 1. Current state of the codebase (read line-by-line)

### 1.1 `UComboAttackGameplayAbility` — what it really does

File: `Source/Beadurinc/AbilitySystem/GameplayAbility/ComboAttackGameplayAbility.{h,cpp}`.

- `InstancingPolicy = InstancedPerActor`; carries an `int ComboCounter` and a kept `UAbilityTask_PlayMontageAndWait* LastComboMontagePlayTask`.
- `CanActivateAbility` rejects when:
  - avatar isn't an `APlayerCharacter`,
  - locked-on target is alive **and** has `State_VulnerableToExecution` (so `UExecutionGameplayAbility` takes over),
  - avatar has `State_ComboLocked`.
- `ActivateAbility`:
  1. Calls `PlayNextComboAttack()`.
  2. Cancels any active `Ability_BlockParry`.
  3. Adds `State_BlockingLocked` to the avatar's ASC.
  4. Calls `AFighterCharacter::ApplyStaminaRegenCooldown()`.
- `PlayNextComboAttack`:
  - Pulls the next montage from `WeaponActor::GetComboAttackAt(ComboCounter)`.
  - Snaps actor rotation Yaw to the camera control rotation Yaw.
  - Adds `State_ComboLocked` to the ASC (released later by `StateWindowAnimNotifyState` on the montage).
  - Cancels the previous play-montage task via `ExternalCancel()` before queueing the next.
  - Spawns `UAbilityTask_PlayMontageAndWait`, hooks `OnCompleted` → `OnMontageCompleted` (ends ability), `OnInterrupted` → `OnMontageInterrupted` (cancels ability).
  - `ComboCounter = (ComboCounter + 1) % WeaponActor->GetComboSequenceLength()`.
  - Calls `APlayerCharacter::ClearInputBuffer()`.
- `InputPressed` (fired by `AbilitySystemComponent::AbilitySpecInputPressed` while the ability is active):
  - If `CanActivateAbility` is still true → `PlayNextComboAttack()` (i.e. chains the combo).
  - Otherwise buffers the input via `APlayerCharacter::BufferInput(EAbilityId::Combo_Attack)`.
- `EndAbility` resets `ComboCounter` and clears `LastComboMontagePlayTask`.
- `OnMontageInterrupted` → `CancelAbility(...)` (which routes through `EndAbility` and resets the counter).

### 1.2 `APlayerCharacter` input plumbing

Relevant bindings (`SetupPlayerInputComponent`, line 186‑220) — Combo and Execution already share `ComboAttackAction.Started`, gated by their own `CanActivateAbility`:

```cpp
EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started,   this, &APlayerCharacter::PressAbility,   static_cast<int32>(EAbilityId::Combo_Attack));
EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::ReleaseAbility, static_cast<int32>(EAbilityId::Combo_Attack));
EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started,   this, &APlayerCharacter::PressAbility,   static_cast<int32>(EAbilityId::Execution));
```

`PressAbility(int32 InputId)` (line 375‑396):

```cpp
void APlayerCharacter::PressAbility(int32 InputId)
{
    if (HasBufferedInput()) ClearInputBuffer();

    if (FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromInputID(InputId))
    {
        if (Spec->IsActive())
        {
            AbilitySystemComponent->AbilitySpecInputPressed(*Spec);
        }
        else
        {
            if (!AbilitySystemComponent->TryActivateAbility(Spec->Handle))
            {
                BufferInput(InputId);
            }
        }
    }
}
```

Input buffer is a single `TOptional<FBufferedInput> BufferedInput` keyed by `InputID` + timestamp.

### 1.3 The input buffer is flushed by an anim notify

File: `Source/Beadurinc/Animation/AnimNotify/FireBufferedInputAnimNotify.cpp`:

```cpp
void UFireBufferedInputAnimNotify::Notify(...)
{
    if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        if (OwnerCharacter->IsLocallyControlled())
        {
            OwnerCharacter->FlushBufferedInput();
        }
    }
}
```

And `FlushBufferedInput` calls `AbilitySystemComponent->AbilityLocalInputPressed(InputID)`, which fires the ability bound to whatever input ID was buffered — Heavy will use exactly this path.

### 1.4 Sprint state — corrected per user's answer #1

`APlayerCharacter::Run` (line 231‑238):

```cpp
GetCharacterMovement()->MaxWalkSpeed = bRunning ? 200.0F : 500.0F;
bRunning = !bRunning;
```

Sprint detection uses **actual planar velocity**, not the configured cap:

```cpp
BCharacter->GetVelocity().Size2D() > 200.0F
```

This avoids the false-positive of "sprint toggled on but the character is standing still" and reflects the *fact* that the player is currently moving fast, which is what the dash branch wants. Matches user's accepted answer.

### 1.5 `AWeaponActor` slots

Already in the working tree (current `.h`):

```cpp
TArray<TObjectPtr<UAnimMontage>> WeaponComboAttacks;
TObjectPtr<UAnimMontage> DashAttack;
TObjectPtr<UAnimMontage> HeavyAttack;
```

Getters for the new fields need to be added; existing `GetComboAttackAt`/`GetComboSequenceLength` are reused unchanged.

### 1.6 Ability granting & EAbilityId

`ABeadurincPlayerState::BeginPlay` is the single place player abilities are granted. `EAbilityId` lists them; one new entry (`Heavy_Attack`) is needed. **No `Dash_Attack` entry needed any more** — Dash is no longer a separate ability (see §2).

### 1.7 Hold detection — Enhanced Input

Heavy's "hold the attack key past threshold" is implemented entirely in the Enhanced Input asset side: a separate `IA_HeavyAttack` with a `UInputTriggerHold` trigger, mapped to the same physical key as `IA_ComboAttack` in the IMC. Combo's `ETriggerEvent::Started` fires immediately on press (unchanged tap responsiveness); Heavy's `Started` fires only after the hold threshold while still pressed.

---

## 2. Revised approach (after user feedback)

The original plan proposed three sibling abilities (Combo, Dash, Heavy). User's question #1 challenged whether `UDashAttackGameplayAbility` deserves to be its own class given that the only thing distinguishing dash from combo is which montage to play. Re-examined: dash and combo share input, side-effects (`State_BlockingLocked`, `ApplyStaminaRegenCooldown`, `Ability_BlockParry` cancel), rotation snap, montage-task plumbing, and the `State_ComboLocked` lock window. They differ on:

- which montage is chosen (DashAttack vs `WeaponComboAttacks[ComboCounter]`),
- whether the combo counter advances.

Both of those collapse cleanly into a **branch inside `UComboAttackGameplayAbility::PlayNextComboAttack`**. So:

- **Dash** → branch inside `UComboAttackGameplayAbility`. No new class. No new `EAbilityId`. No new input binding. Sprint detection by velocity at swing time selects the dash montage and resets `ComboCounter` (user's question #2).
- **Heavy** → still its own class `UHeavyAttackGameplayAbility`. *Reason it stays separate*: it's triggered by a different Enhanced Input Action (`IA_HeavyAttack` with Hold trigger), so its dispatch lives in a different binding callback. Folding it into Combo would force Combo to peek at an out-of-band "is heavy mode" flag set by the player; that's more coupling, not less. Heavy's combo-counter reset (user's question #2) is achieved by the natural montage-interrupt path — Heavy's `PlayMontageAndWait` interrupts Combo's, which triggers Combo's `OnMontageInterrupted` → `CancelAbility` → `EndAbility` → `ResetComboCounter`.
- **Combo + Dash + Heavy chaining** — user's input plan from answer #5 works as follows on a single physical attack key:
  - quick tap → `IA_ComboAttack.Started` fires → combo (or dash, if velocity > 200) plays via the same ability;
  - hold past threshold → `IA_HeavyAttack.Started` fires → heavy plays (or buffers if combo lock is up; flushed by `FireBufferedInputAnimNotify` on the combo's state-window-end);
  - so combo1 → combo2 → heavy is the existing three-press flow with the third press being a hold. Sequential, never simultaneous, as user requires.

---

## 3. Full code snippets

### 3.1 `EAbilityId` — add **one** entry

**`Source/Beadurinc/AbilitySystem/AbilityId.h`**:

```cpp
#pragma once

enum class EAbilityId : int32
{
    Combo_Attack = 1,
    Block        = 2,
    Roll         = 3,
    Hit_React    = 4,
    Parry_React  = 5,
    Execution    = 6,
    Heavy_Attack = 7,
};
```

### 3.2 `AWeaponActor` — add getters

**`Source/Beadurinc/Actor/WeaponActor.h`** (additions inside the existing `public:` block):

```cpp
public:

    AWeaponActor();

public:

    TObjectPtr<UAnimMontage> GetComboAttackAt(const unsigned int& Index) const;

    FORCEINLINE uint32 GetComboSequenceLength() const { return WeaponComboAttacks.Num(); };

    FORCEINLINE TObjectPtr<UAnimMontage> GetDashAttack()  const { return DashAttack;  }
    FORCEINLINE TObjectPtr<UAnimMontage> GetHeavyAttack() const { return HeavyAttack; }

    FORCEINLINE float GetStaminaDamage() const { return StaminaDamage; };
    FORCEINLINE float GetHealthDamage()  const { return HealthDamage;  };
```

No `.cpp` change.

### 3.3 `UComboAttackGameplayAbility::PlayNextComboAttack` — dash branch

`CanActivateAbility` is **unchanged from current code** (no sprint check there — sprint routing happens *inside* the swing, not at the gate).

The only edit is in `PlayNextComboAttack`. Full revised method:

**`Source/Beadurinc/AbilitySystem/GameplayAbility/ComboAttackGameplayAbility.cpp`** (replaces lines 17‑54):

```cpp
/** Plays next combo / dash attack by ability task */
void UComboAttackGameplayAbility::PlayNextComboAttack()
{
    APlayerCharacter* BCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
    if (!BCharacter || !BCharacter->IsHoldingWeapon()) return;

    // Camera-yaw snap — shared across all attack montages (user's answer #3).
    const FRotator ActorRotation   = BCharacter->GetActorRotation();
    const FRotator ControlRotation = BCharacter->GetController()->GetControlRotation();
    BCharacter->SetActorRotation(FRotator(ActorRotation.Pitch, ControlRotation.Yaw, ActorRotation.Roll));

    BCharacter->GetAbilitySystemComponent()->AddLooseGameplayTag(StateGameplayTags::State_ComboLocked);

    if (LastComboMontagePlayTask)
    {
        LastComboMontagePlayTask->ExternalCancel();
    }

    // Sprint check uses actual planar velocity, not MaxWalkSpeed (user's answer #1).
    const bool bDashBranch = BCharacter->GetVelocity().Size2D() > 200.0F
                          && IsValid(BCharacter->GetWeaponActor()->GetDashAttack());

    TObjectPtr<UAnimMontage> NextMontage;
    if (bDashBranch)
    {
        NextMontage = BCharacter->GetWeaponActor()->GetDashAttack();
        // Dash always discards any in-progress combo chain (user's question #2).
        ResetComboCounter();
    }
    else
    {
        NextMontage = BCharacter->GetWeaponActor()->GetComboAttackAt(ComboCounter);
    }

    if (!IsValid(NextMontage)) return;

    UAbilityTask_PlayMontageAndWait* AbilityTaskPlayMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        TEXT("ComboAttack"),
        NextMontage
    );

    AbilityTaskPlayMontage->OnCompleted.AddDynamic(this,   &UComboAttackGameplayAbility::OnMontageCompleted);
    AbilityTaskPlayMontage->OnInterrupted.AddDynamic(this, &UComboAttackGameplayAbility::OnMontageInterrupted);
    AbilityTaskPlayMontage->ReadyForActivation();
    LastComboMontagePlayTask = AbilityTaskPlayMontage;

    // Only the combo path advances the counter; dash is single-shot.
    if (!bDashBranch)
    {
        ComboCounter = (ComboCounter + 1) % BCharacter->GetWeaponActor()->GetComboSequenceLength();
    }

    BCharacter->ClearInputBuffer();
}
```

No header change — `PlayNextComboAttack` keeps the same signature, `ComboCounter` is reused, `LastComboMontagePlayTask` is reused.

### 3.4 `UHeavyAttackGameplayAbility` — new sibling ability

**`Source/Beadurinc/AbilitySystem/GameplayAbility/HeavyAttackGameplayAbility.h`** (new):

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "HeavyAttackGameplayAbility.generated.h"

/**
 * Single-shot heavy attack. Triggered by holding the attack key, implemented in
 * APlayerCharacter::SetupPlayerInputComponent via a separate Enhanced Input
 * Action with a UInputTriggerHold modifier.
 *
 * Buffered through the existing input-buffer path (APlayerCharacter::BufferInput /
 * FlushBufferedInput / FireBufferedInputAnimNotify), so 'combo1 -> combo2 -> heavy'
 * works without any cross-ability coupling.
 */
UCLASS()
class BEADURINC_API UHeavyAttackGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:

    UHeavyAttackGameplayAbility();

protected:

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageInterrupted();

private:

    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
};
```

**`HeavyAttackGameplayAbility.cpp`** (new):

```cpp
#include "AbilitySystem/GameplayAbility/HeavyAttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Actor/Character/PlayerCharacter.h"
#include "Actor/WeaponActor.h"
#include "AbilitySystem/GameplayTag/AbilityTags.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"

UHeavyAttackGameplayAbility::UHeavyAttackGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UHeavyAttackGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;

    APlayerCharacter* Owner = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!Owner || !Owner->IsHoldingWeapon() || !Owner->GetWeaponActor()->GetHeavyAttack()) return false;

    // Same global attack lock as combo (user's answer #2).
    return !ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(StateGameplayTags::State_ComboLocked);
}

void UHeavyAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    APlayerCharacter* Owner = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!Owner)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    // Camera-yaw snap — same convention as combo / dash (user's answer #3).
    const FRotator ActorRot   = Owner->GetActorRotation();
    const FRotator ControlRot = Owner->GetController()->GetControlRotation();
    Owner->SetActorRotation(FRotator(ActorRot.Pitch, ControlRot.Yaw, ActorRot.Roll));

    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    if (!ASC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    ASC->AddLooseGameplayTag(StateGameplayTags::State_ComboLocked);

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, TEXT("HeavyAttack"), Owner->GetWeaponActor()->GetHeavyAttack());

    MontageTask->OnCompleted.AddDynamic(this,   &UHeavyAttackGameplayAbility::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageInterrupted);
    MontageTask->ReadyForActivation();

    Owner->ClearInputBuffer();

    // Same side-effects as combo: cancel block, lock blocking, eat stamina regen.
    FGameplayTagContainer BlockTags;
    BlockTags.AddTag(AbilityTags::Ability_BlockParry);
    ASC->CancelAbilities(&BlockTags);
    ASC->AddLooseGameplayTag(StateGameplayTags::State_BlockingLocked);

    if (AFighterCharacter* Fighter = Cast<AFighterCharacter>(Owner))
    {
        Fighter->ApplyStaminaRegenCooldown();
    }
}

void UHeavyAttackGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    MontageTask = nullptr;
}

void UHeavyAttackGameplayAbility::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UHeavyAttackGameplayAbility::OnMontageInterrupted()
{
    CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false);
}
```

> Why no explicit `ResetComboCounter()` call in Heavy: when Heavy's `PlayMontageAndWait` starts, the animation system interrupts any currently playing montage on the avatar's mesh. That fires the active `UComboAttackGameplayAbility::OnMontageInterrupted`, which calls `CancelAbility` → `EndAbility` → `ResetComboCounter()`. The counter is therefore zeroed by the existing path, without Heavy having to reach into Combo. (User's question #2 — satisfied.) If Combo wasn't active in the first place, `ComboCounter` was already 0 from the previous `EndAbility` reset.

### 3.5 `APlayerCharacter` — one new Input Action field + one binding

**`Source/Beadurinc/Actor/Character/PlayerCharacter.h`** — add one field next to `ComboAttackAction` (line 176‑177):

```cpp
UPROPERTY(EditAnywhere, Category="Input")
UInputAction* ComboAttackAction;

/** Heavy Attack Ability Input Action — distinct from ComboAttackAction so a UInputTriggerHold can fire only after the hold threshold without delaying combo taps. */
UPROPERTY(EditAnywhere, Category="Input")
UInputAction* HeavyAttackAction;
```

**`PlayerCharacter.cpp` `SetupPlayerInputComponent`** — append after the existing Combo / Execution bindings (line 207‑211). The Combo binding is unchanged; only one new line is added for Heavy:

```cpp
// Combo Attack Ability (unchanged)
EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started,   this, &APlayerCharacter::PressAbility,   static_cast<int32>(EAbilityId::Combo_Attack));
EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::ReleaseAbility, static_cast<int32>(EAbilityId::Combo_Attack));

// Execution Ability (unchanged)
EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::PressAbility, static_cast<int32>(EAbilityId::Execution));

// Heavy Attack Ability — IA_HeavyAttack carries a UInputTriggerHold modifier so 'Started' fires after the hold threshold elapses while still pressed.
EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::PressAbility, static_cast<int32>(EAbilityId::Heavy_Attack));
```

**No dash binding is added** — dash is decided inside `UComboAttackGameplayAbility::PlayNextComboAttack` at swing time.

### 3.6 `ABeadurincPlayerState` — grant Heavy

**`Source/Beadurinc/GameData/BeadurincPlayerState.h`** — add one field alongside the other `TSubclassOf<UGameplayAbility>`:

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
TSubclassOf<UGameplayAbility> HeavyAttackAbility;
```

**`BeadurincPlayerState.cpp`** — extend `BeginPlay` (line 17‑63) with one more grant block, after `ExecutionAbility`:

```cpp
// Give heavy attack ability
if (HeavyAttackAbility)
{
    FGameplayAbilitySpec HeavyAttackAbilitySpec(HeavyAttackAbility, 1, static_cast<int32>(EAbilityId::Heavy_Attack), this);
    AbilitySystemComponent->GiveAbility(HeavyAttackAbilitySpec);
}
```

No dash entry — dash is not a separate spec.

---

## 4. User-side checklist (assets / BP / IMC — handled by you)

In response to user's answer #5 ("Let me know if there should be a complementary plan or editor side works"):

1. **New Input Action asset** — create `Content/.../Input/IA_HeavyAttack` (duplicate `IA_ComboAttack` as a base if convenient). On its `Triggers` array add one `Input Trigger Hold` entry; set `Hold Time Threshold` to the desired duration (recommend ~0.25s — long enough to differentiate from a tap, short enough to feel responsive). Leave `Is One Shot` off so the threshold trigger fires once per press cycle.
2. **Input Mapping Context** — in the project's IMC (e.g. `IMC_Default`), add a new mapping: same physical key as the existing `IA_ComboAttack` binding (LMB, presumably), action = `IA_HeavyAttack`. Both actions will evaluate the same key press — combo fires immediately on `Started`, heavy fires on `Started` only after the hold threshold; sequence-of-events is therefore: tap → combo only; long press → combo on press *and* heavy queued at threshold.
3. **Player BP** (`BP_PlayerCharacter` or subclass) — set the new `HeavyAttackAction` property to `IA_HeavyAttack`. `ComboAttackAction` remains pointed at `IA_ComboAttack` unchanged.
4. **Weapon BP** (`BP_RustySword` / weapon BPs) — set `DashAttack = AM_Sword_DashAttack` and `HeavyAttack = AM_Sword_HeavyAttack` in the weapon defaults. The asset files already exist in the working tree.
5. **Player-state defaults BP** — set `HeavyAttackAbility` to the new BP wrapper class (or the C++ class directly if the project pattern allows).
6. **Anim notifies on `AM_Sword_DashAttack` / `AM_Sword_HeavyAttack`**:
   - **Both** must include `UStateWindowAnimNotifyState` covering the swing window with `State_ComboLocked` in its `StateTags`. Without it the lock is never released and the next attack input stays buffered indefinitely (or, more practically, until the buffer window of 0.25s expires and the input is dropped).
   - **Both** should include a `UFireBufferedInputAnimNotify` at the chain point, so a buffered heavy press during dash/combo flushes at the right time. `AM_Sword_Combo*` already do this — mirror that configuration.

---

## 5. Resolved questions (from `## Open Question Answers:`)

Preserved here for traceability; user's text reproduced in §7 verbatim.

1. **Sprint detection** — ✅ velocity-based. `BCharacter->GetVelocity().Size2D() > 200.0F` (§3.3).
2. **Lock tag** — ✅ keep `State_ComboLocked`; it's the global attack lock and is reused by Heavy as well (§3.4 `CanActivateAbility`).
3. **Yaw snap for Heavy** — ✅ included for consistency with combo / dash (§3.4 `ActivateAbility`).
4. **Input buffering for Dash and Heavy** — ✅ alive for both.
   - Dash buffers automatically because it's a branch of Combo — Combo's `InputPressed` already routes failing inputs through `BufferInput(EAbilityId::Combo_Attack)`, and on flush the sprint check still decides dash vs combo.
   - Heavy buffers automatically because `PressAbility(Heavy_Attack)` already calls `BufferInput(InputId)` on `TryActivateAbility` failure (which is what happens when `State_ComboLocked` is set during a combo swing). `FireBufferedInputAnimNotify` flushes the buffer at the chain window, which calls `AbilityLocalInputPressed(Heavy_Attack)` and activates Heavy.
5. **Input plan** — ✅ Tap = `IA_ComboAttack` (combo or dash depending on velocity). Hold = `IA_HeavyAttack` with `UInputTriggerHold`. Both bound to the same physical key in the IMC. Editor-side work spelled out in §4 items 1‑2. No risk of Dash + Heavy firing simultaneously per user's requirement — Dash is decided *inside* Combo at swing-time, Heavy is queued through the buffer and plays sequentially after the dash swing's state-window opens.

---

## 6. Answers to user's follow-up questions (from `## My open questions:`)

### Q1 — Is `UDashAttackGameplayAbility` really a good choice?

**Agreed: no.** The only meaningful axes of difference between dash and combo are (a) which montage to play and (b) whether the combo counter advances. Both fold into a one-line branch in `PlayNextComboAttack`. Separating dash into its own ability duplicated rotation snap, lock-tag handling, `Ability_BlockParry` cancel, `State_BlockingLocked` add, `StaminaRegenCooldown` apply, `PlayMontageAndWait` plumbing, `OnMontageCompleted`/`OnMontageInterrupted` handlers, and the input-buffer dispatch — all for what reduces to *"play this other montage and skip the counter"*. The revised §3.3 absorbs dash into Combo and the plan loses an entire class + cpp + EAbilityId entry + input binding line + grant block.

`UHeavyAttackGameplayAbility` stays separate because its **dispatch** is genuinely different (different Input Action, different trigger semantics — Hold vs press). Folding Heavy into Combo would force Combo to read an out-of-band "next attack is heavy" flag set by Heavy's input callback, which is more coupling than just letting Heavy be its own short ability that interrupts Combo via the natural montage path. The internals (montage / side effects) are similar, but the *cost of duplication* for Heavy is small (~60 lines), and the gain in clarity at the input boundary is worth it.

### Q2 — Does the plan reset `ComboCounter` on dash / heavy?

**Yes — both paths reset, by different mechanisms:**

- **Dash**: in `PlayNextComboAttack` the dash branch explicitly calls `ResetComboCounter()` before playing the dash montage (§3.3 line `if (bDashBranch) { ... ResetComboCounter(); ... }`). After the dash montage completes, `OnMontageCompleted` → `EndAbility` → `ResetComboCounter()` again (defensive). So whether dash fires from a fresh activation or as a chained `InputPressed` mid-combo, the counter ends up at 0.
- **Heavy**: when Heavy's `UAbilityTask_PlayMontageAndWait` starts on the avatar's mesh, it interrupts any currently playing montage. Combo's `OnMontageInterrupted` fires → `CancelAbility(...)` → `EndAbility(..., bWasCancelled=true)` → `ResetComboCounter()`. No cross-ability coupling needed. If Combo wasn't running, `ComboCounter` was already 0 (`EndAbility` had reset it the previous time).

---

## 7. User's original answer + questions (verbatim, for record)

### Open Question Answers (user)

1. You're right. It is better approach than mine since it checks the actual player movement speed, not just configured value. Take you approach.
2. No, `State_ComboLocked` is globally used for every attack behaviors. Take the current approach.
3. Yes, my intention is syncing the player character's view to camera whenever any **montage** based animation is played. Take the current approach.
4. Typically, when you're able to do dash attacks, it is regarded as the whole conditions are met because `State_ComboLocked` can't exist while running.
   But alive the input buffering feature for dash also for consistency. Heavy attack should be able to use combined with combo attacks (e.g. combo1 -> combo2 -> heavy) so
   the input buffer should alive.
5. Let me explain my input triggering plan. I want the heavy attacks triggered when player *hold* the key (when press time exceeds a predefined threshold time) that is same as
   combo attack key, and when player *press* (click and release the key before threshold time ends) the key it plays combo or dash attacks as the current behavior.
   According to my plan there should be no chance to trigger both DashAttack and HeavyAttack simultaneously.
   Let me know if there should be a complementary plan or editor side works (e.g. creating InputAction or mapping in InputMappingContext asset) for my approach.

### My open questions (user)

1. Is `DashAttackGameplayAbility` really a good choice? Because the only difference is a "movement speed" (as I concluded in Answer 1),
   I feel it like redundant decoupling since dash attack has the same input binding and triggering. `HeavyAttackGameplayAbility` looks valid
   but re-consider making a `DashAttackGameplayAbility`.
2. When player triggers dash or heavy attacks, they should reset the current combo count, the `ComboAttackGameplayAbility#ComboCounter` variable.
   Can your plan handle this case?

---

## 8. Summary of files that will change (revised)

| File | Change |
|------|--------|
| `Source/Beadurinc/AbilitySystem/AbilityId.h` | +1 enum entry (`Heavy_Attack`). No `Dash_Attack` entry. |
| `Source/Beadurinc/Actor/WeaponActor.h` | +2 `FORCEINLINE` getters (`GetDashAttack`, `GetHeavyAttack`) |
| `Source/Beadurinc/AbilitySystem/GameplayAbility/ComboAttackGameplayAbility.cpp` | Replace `PlayNextComboAttack` body to add the velocity-based dash branch and call `ResetComboCounter` before playing the dash montage. `CanActivateAbility` unchanged. |
| `Source/Beadurinc/AbilitySystem/GameplayAbility/HeavyAttackGameplayAbility.h` | New |
| `Source/Beadurinc/AbilitySystem/GameplayAbility/HeavyAttackGameplayAbility.cpp` | New |
| `Source/Beadurinc/Actor/Character/PlayerCharacter.h` | +1 `UInputAction*` field (`HeavyAttackAction`) |
| `Source/Beadurinc/Actor/Character/PlayerCharacter.cpp` | +1 `BindAction` line (Heavy on `IA_HeavyAttack` Started) |
| `Source/Beadurinc/GameData/BeadurincPlayerState.h` | +1 `TSubclassOf<UGameplayAbility>` field (`HeavyAttackAbility`) |
| `Source/Beadurinc/GameData/BeadurincPlayerState.cpp` | +1 `GiveAbility` block |

Files **removed from the previous plan**: `DashAttackGameplayAbility.h`, `DashAttackGameplayAbility.cpp` (no longer needed — folded into Combo).

No existing ability class is deleted. `UComboAttackGameplayAbility` change is localized to one method (`PlayNextComboAttack`); header, ctor, `ActivateAbility`, `EndAbility`, and `InputPressed` are untouched.