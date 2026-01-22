# Combo Attack System

## Synopsys

The **Combo Attack** system in 3d action game usually consists of the most basic part of player's
offensive behavior. Commonly, this system doesn't contain a complicated triggering condition or
input mechanism. I goaled to implement a combo system that ensures the coherent player behavior in
any type of inputs (Rapid press, in-time press) with input buffering system.

## Definition of Player State

Before I start implementing, I needed to define a *player state* whether the combo attacks are
allowed or blocked. After observing some commercial games, I figured out that players can *skip*
the current combo before the animation fully ends. Players either AFK to observe proper recovery
animation being played or skip the recovery time and go to the next combo by pressing key.

### `StateWindowAnimNotifyState`

![StateWindowNotifyState](StateWindowNotifyState.png)

I made a custom notify state to give and get rid of GameplayTags when the animation enters and
exits the notify state. Then, made players can only do combo attacks when the tag doesn't exist.
This prevented an awkward animation skip while the player is preparing or swinging a weapon.

## Workflow

Combo Attack uses GAS(Gameplay Ability System). Let's get through what happens in each stage.

![ComboAttackFlowChart](ComboAttackFlowChart.png)

- ## `PlayerCharacter` part

### PressedAbility

This stage is triggered by the user input, bound by EnhancedInput system. The combo ability is
treated as activated while the animation is being played
