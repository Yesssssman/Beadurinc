# Fix plan: HeavyAttack never-ends & AI Block early-cancel

This document analyzes two bugs and proposes fixes. **No code changes are made in this stage.** Each fix item is rated:

- **Apparent** — root cause is shown directly in the code; the fix removes the cause.
- **Suspicious** — strong candidate, but the failure requires a specific timing/state combination that wasn't reproduced in this read-through.
- **Minor possible** — defensive cleanup; could plausibly contribute, low confidence.

---

## Bug A — `UHeavyAttackGameplayAbility` becomes "uncastable" because it never ends

### Symptom

After (intermittently) using the heavy attack, the player can no longer activate it. The ability spec stays in `IsActive() == true`, so:

- `UAbilitySystemComponent::TryActivateAbility` is short-circuited by `Spec->IsActive()` + `bRetriggerInstancedAbility = false` (default for this ability, `InstancingPolicy = InstancedPerActor`).
- In `APlayerCharacter::PressAbility` the active-spec branch routes the press into `AbilitySpecInputPressed`, which the heavy ability does not implement, so the keypress becomes a no-op.

### Code path that *should* end the ability

`Source/Beadurinc/AbilitySystem/GameplayAbility/HeavyAttackGameplayAbility.cpp:51-63`

```cpp
MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
    this, TEXT("HeavyAttack"), Owner->GetWeaponActor()->GetHeavyAttack()
);

MontageTask->OnCompleted.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageCompleted);
MontageTask->OnInterrupted.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageInterrupted);
MontageTask->ReadyForActivation();
```

Only `OnCompleted` and `OnInterrupted` are bound. `UAbilityTask_PlayMontageAndWait` exposes **four** simple delegates: `OnCompleted`, `OnBlendOut`, `OnInterrupted`, `OnCancelled`. Two are unbound.

### Cause 1 (Apparent) — `OnCancelled` is unbound, leaks the ability when the montage fails to play

Inside `UAbilityTask_PlayMontageAndWait::Activate()` (engine code), if `AbilitySystemComponent->PlayMontage(...)` returns `0.f` (the montage didn't start), the task takes the `bPlayedMontage == false` branch and broadcasts **only** `OnCancelled` before ending the task. Since `OnCancelled` has no listener here, no one calls `EndAbility` and the ability remains active forever. The same `OnCancelled` broadcast also fires from `UAbilityTask_PlayMontageAndWait::OnAbilityCancelled()` whenever `ExternalCancel()` is called on the task directly (i.e. without going through `UGameplayAbility::CancelAbility`).

Realistic triggers for `PlayMontage` returning 0 in this project:

- `Owner->GetWeaponActor()->GetHeavyAttack()` returns a montage whose skeleton mismatches the avatar mesh, the asset is unloaded, or the avatar's `AnimInstance` is momentarily null (mesh rebuild during sword swap / weapon change). `CanActivateAbility` only checks `GetHeavyAttack() != nullptr`, not that the asset is valid for this mesh.
- The heavy is reactivated mid-life of the previous instance. The current `if (MontageTask) MontageTask->ExternalCancel();` branch (line 53-55) routes through `OnAbilityCancelled` → `OnCancelled.Broadcast()`. With nothing bound, the OLD activation's instance state (`State_ComboLocked` was already added, `MontageTask` is now stale) is left in place if the new activation also fails to play. Note: with `bRetriggerInstancedAbility = false` (default) the engine *should* reject re-entry, so this branch is effectively dead code today — but if `bRetriggerInstancedAbility` is ever flipped to true in the Blueprint subclass, this becomes a live failure path.

### Cause 2 (Suspicious) — `OnBlendOut` is unbound

The engine fires `OnMontageBlendingOut(bInterrupted=false)` → `OnBlendOut.Broadcast()` first, then later `OnMontageEnded(bInterrupted=false)` → `OnCompleted.Broadcast()`. The common case is fine because `OnCompleted` still fires. But if the montage is stopped between BlendOut and End (for example a HitReact swapping the player's animating ability while heavy is in its blend-out tail), the End may arrive with `bInterrupted = true` and `OnCompleted` is skipped. `OnInterrupted` is bound and *does* fire from `OnMontageBlendingOut` when bInterrupted is true at blend-out time — so the failure window is narrow (BlendOut already fired with `bInterrupted=false`, then a different code path stops the montage before End). Less likely than Cause 1.

### Cause 3 (Minor possible) — `EndAbility` does not defensively end `MontageTask`

```cpp
void UHeavyAttackGameplayAbility::EndAbility(...)
{
    Super::EndAbility(...);
    MontageTask = nullptr;
}
```

If `EndAbility` is reached without going through the task's delegates (e.g. another ability calls `CancelAbilities` with a tag matching this one) the parent `EndAbility` already cancels tasks via `UGameplayAbility::EndAbility` → `OnGameplayAbilityEnded.Broadcast()`, so the task tears down cleanly. Defensive only.

### Cause 4 (Minor possible) — `State_ComboLocked` release relies on `UStateWindowAnimNotifyState::NotifyEnd`

The heavy montage adds `State_ComboLocked` at `ActivateAbility` (line 49) and is supposed to have it released by `UStateWindowAnimNotifyState::NotifyEnd` on the montage itself (per `Expand_ComboAttackGameplayAbility.md:466`). If the heavy montage is missing the notify state, or it's authored as `NotifyState` covering a window that never closes (e.g. extends past montage end with `bIsTickPaused` quirks), the lock is stuck forever and the ability appears uncastable even when the ability spec is not active. The user described it as "the ability never ends," so this is unlikely to be the same symptom, but it would *also* present as "can't activate heavy". Worth verifying the asset has the notify state and that its end frame lies *before* the montage end.

### Fix proposal — Bug A

**Apparent fix (closes Cause 1).** Bind `OnCancelled` and `OnBlendOut` to safe terminators:

```cpp
MontageTask->OnCompleted.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageCompleted);
MontageTask->OnBlendOut.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageCompleted);    // treat blend-out as normal end
MontageTask->OnInterrupted.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageInterrupted);
MontageTask->OnCancelled.AddDynamic(this, &UHeavyAttackGameplayAbility::OnMontageInterrupted);  // treat cancel as interruption
```

Notes:
- `OnMontageCompleted` already calls `EndAbility(..., bWasCancelled=false)` and is safe to call twice (the second `EndAbility` is a no-op because `Spec->IsActive()` is already false after the first).
- `OnMontageInterrupted` already calls `CancelAbility`; same idempotency argument.
- After this change, every termination path (complete / blend-out / interrupt / cancel / failed-to-play) ends the ability.

**Defensive cleanup (Causes 3 & 4).**
- Add `MontageTask = nullptr;` *before* `Super::EndAbility` to avoid the parent's task teardown attempting to call delegates that reach back into this ability.
- Remove or repurpose the dead `if (MontageTask) ExternalCancel();` branch in `ActivateAbility`. Either delete it (matches reality — heavy is single-shot, not retriggerable) or set `bRetriggerInstancedAbility = true` in the constructor *and* keep the cancel so re-entry actually works. Pick one.
- Spot-check the `HeavyAttack` montage asset in the editor and confirm it has `UStateWindowAnimNotifyState` covering the lock window with `State_ComboLocked` in `StateTags`.

### Severity rating — Bug A: **Apparent**

The unbound `OnCancelled` is a direct, demonstrable leak path. Binding it (plus `OnBlendOut`) closes the most plausible cause. Cause 4 (notify-state authoring) is asset-side and should be verified alongside the C++ fix.

---

## Bug B — Monster cancels `UAIBlockGameplayAbility` before getting hit

### Symptom

The AI activates `UAIBlockGameplayAbility`, but the ability ends almost immediately — before the player's swing collides — so no actual block happens, and the BT moves on.

### Code path

`Source/Beadurinc/AbilitySystem/GameplayAbility/AIBlockGameplayAbility.cpp:30-35`

```cpp
FGameplayTagContainer GameplayTags;
GameplayTags.AddTag(StateGameplayTags::State_Attacking);
UAbilityTask_WaitAttackTargetState* AT_WaitTargetState =
    UAbilityTask_WaitAttackTargetState::WaitState(this, EGameplayContainerMatchType::Any, GameplayTags, /*Inverse=*/true);
AT_WaitTargetState->OnConditionMet.AddDynamic(this, &UAIBlockGameplayAbility::OnTargetNoMoreAttacking);
AT_WaitTargetState->ReadyForActivation();
```

`Source/Beadurinc/AbilitySystem/AbilityTask/AbilityTask_WaitAttackTargetState.cpp:17-48`

```cpp
void UAbilityTask_WaitAttackTargetState::Activate()
{
    Super::Activate();

    AFighterCharacter* Fighter = Cast<AFighterCharacter>(GetAvatarActor());
    if (!Fighter) { EndTask(); return; }

    ACharacter* Target = Fighter->GetAttackTarget();
    if (!IsValid(Target)) { EndTask(); return; }

    TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
    if (!TargetASC) { EndTask(); return; }

    if (IsConditionMet())                  // <-- evaluated SYNCHRONOUSLY at activation
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnConditionMet.Broadcast();    // <-- fires OnTargetNoMoreAttacking → EndAbility
        }

        EndTask();
        return;
    }
    // ... only here do we register tag listeners
}
```

And the condition with `Inverse = true`:

```cpp
bool UAbilityTask_WaitAttackTargetState::IsConditionMet() const
{
    ...
    case EGameplayContainerMatchType::Any:
        Result = OwnedTags.HasAny(GameplayTags);  // does the player own State_Attacking right now?
        break;
    ...
    if (Inverse) Result = !Result;
    return Result;
}
```

### Cause (Apparent)

`State_Attacking` is added on the player only inside a `UStateWindowAnimNotifyState` window of the player's attack montage (it is declared in `StateGameplayTags.h:9` but never `AddLooseGameplayTag`'d in C++; the only source is the notify state). This window is intentionally narrow — typically the actual swing/contact frames.

The BT triggers the block ability when it scores high enough — usually **before** the player has reached the swing window, i.e. while the player is still in the wind-up of their montage and `State_Attacking` is **not yet** present on the player's ASC.

Result, traced through `Activate()`:

1. `IsConditionMet()` evaluates `OwnedTags.HasAny({State_Attacking})` → `false` (player hasn't entered the swing window yet).
2. `Inverse = true` flips it to `true`.
3. `OnConditionMet.Broadcast()` fires **synchronously** inside `ReadyForActivation()` — same call stack as `UAIBlockGameplayAbility::ActivateAbility`.
4. `OnTargetNoMoreAttacking` calls `EndAbility(...)`. The block ability ends before control returns to `ActivateAbility`.
5. The subsequent `UAbilityTask_WaitGameplayEvent` for `Event.Combat.Hit` is still wired, but `IsActive()` is already false, so the early-out handler does nothing.

The semantics in the task header read "Waits until the avatar's attack target satisfies a gameplay tag condition" — but with `Inverse = true`, "satisfies" means "*lacks* the watched tag," which is the default state. The task fires its terminator the moment it is activated against an idle target.

The same construction in `UAIParryGameplayAbility::ActivateAbility` (lines 33-38) has the identical bug; the parry ability ends instantly under the same conditions.

### Why the early-out via `Event.Combat.Hit` doesn't save us

The hit event is only sent when the player's weapon collider actually overlaps the AI (`AFighterCharacter::OnMeleeContacts`). By the time that happens, the block ability has already self-ended; the `WaitGameplayEvent` handler checks `IsActive()` and bails out.

### Fix proposal — Bug B

Two acceptable directions. Either fixes the bug; pick the one that matches design intent.

**Option 1 — Fix the task to wait for a real transition (preferred).**

In `UAbilityTask_WaitAttackTargetState::Activate()`, do not broadcast/end on the initial evaluation when `Inverse == true`. The natural reading of "wait until the target stops attacking" requires having first observed them attacking. Two concrete shapes:

- (a) Track whether the watched condition has ever been *not-met*; only broadcast `OnConditionMet` after a transition from "watched tag present" → "watched tag absent."
- (b) Simpler: register the tag listener regardless on activation, and only fire if the change comes from a tag-change event (skip the synchronous initial check entirely when `Inverse` is true). For `Inverse == false` (wait for tag to appear), the existing synchronous check is fine.

Either way, the public surface of the task does not change for `UAIParryGameplayAbility` — the parry path benefits from the same fix.

**Option 2 — Replace the task in the block/parry abilities with a fixed-duration delay.**

If the design is "block for N seconds, end early on hit," the block ability already has a `BlockMontage` whose duration is naturally bounded; pair `UAbilityTask_PlayMontageAndWait` (terminate on `OnCompleted`/`OnInterrupted`) with the existing `UAbilityTask_WaitGameplayEvent` for `Event.Combat.Hit`. Remove `UAbilityTask_WaitAttackTargetState` from the block/parry path. The task class can stay for other future callers — or be deleted, depending on whether anyone else uses it (currently nobody does; it's only constructed from AIBlock and AIParry).

**Recommendation: Option 1.** It keeps the existing block/parry shape, fixes the parry ability for free, and the original intent ("end as soon as the player's swing window closes, otherwise rely on the hit event") still holds — only now it triggers on the real 1→0 transition rather than the trivially-met "already 0" initial state.

### Severity rating — Bug B: **Apparent**

The synchronous broadcast on initial-condition-met inside `Activate()` is the demonstrable root cause. The fix removes the race entirely.

---

## Summary

| Bug | Cause | Severity |
|-----|-------|----------|
| Heavy never ends | `UAbilityTask_PlayMontageAndWait::OnCancelled` (and `OnBlendOut`) unbound; ability has no terminator when the montage fails to play or is externally cancelled | **Apparent** |
| Heavy never ends | Dead-code re-entry branch (`if (MontageTask) ExternalCancel();`) that depends on `bRetriggerInstancedAbility=true`, which isn't set | Minor possible |
| Heavy never ends | `State_ComboLocked` release authored on the wrong frame of the heavy montage | Minor possible (asset-side check) |
| AI block cancels early | `UAbilityTask_WaitAttackTargetState::Activate()` broadcasts synchronously when `IsConditionMet()` is already true at start; with `Inverse=true` this matches the default idle state of the target | **Apparent** |
| AI parry cancels early | Same root cause as AI block (`UAIParryGameplayAbility` uses the same task in the same shape) | **Apparent** — fixed by the same change |

## Files that the fix will touch (when implementation begins)

- `Source/Beadurinc/AbilitySystem/GameplayAbility/HeavyAttackGameplayAbility.cpp` — bind `OnBlendOut`/`OnCancelled`; tidy dead-code branch.
- `Source/Beadurinc/AbilitySystem/AbilityTask/AbilityTask_WaitAttackTargetState.cpp` — gate initial broadcast on a real transition for `Inverse` semantics.
- (Asset, not code) `HeavyAttack` montage — verify `UStateWindowAnimNotifyState` covers the lock window with `State_ComboLocked`.

No public API changes; both fixes are local to the affected `.cpp` files.