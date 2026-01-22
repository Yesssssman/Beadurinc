# Gameplay Ability System (GAS)

The **Gameplay Ability System** is Unreal Engine's framework to handle every single player
behavior in a modularized way. Developers can efficiently manage the behaviors by decoupling
them from player classes, and even reuse for non-player characters.

There are suitable cases that you can consider applying GAS.

First, when your behavior needs to check or modify the player state attributes(e.g. Health,
Mana, Stamina), you should consider making them as GAS's **Attribute Set** instead of managing
them as primitive variables since they provide more advanced code maintenance and callback
system for value modifications.

Second, if you want to make your project fully support multiplayer environment you're better
choose GAS to implement player behaviors. It will conduct the complicated networking process
and developer only needs to know whether each event is triggered in the client, or the server.
Even tho you don't have any plan to migrate your project from singleplayer to multiplayer, it
is better to use GAS to meet OOP programming principles and code quality to lower the effort
of refactoring your code in the future.

# Components of GAS
## UAbilitySystemComponent
**Ability System Component** is a manager class that in charge of maintaining any other GAS
components. As it introduced as fully modularized and decoupled, developers freely determine
the timing of each GAS component being attached or detached in runtime.

By implementing **IAbilitySystemInterface**, you declare that your character will use GAS system.
The interface only contains a method that returns **Ability System Component** pointer.

In Player.h,
```c++
class APlayerCharacter : public ACharacter, public IAbilitySystemInterface {
...
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
...
}
```
In Player.cpp,
```c++
APlayerCharacter::APlayerCharacter()
{
    // Creates an ASC instance
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
        TEXT("AbilitySystemComponent")
    );
}

void APlayerCharacter::BeginPlay()
{
    if (AbilitySystemComponent)
    {
        // Sets the onwer of ASC
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
    }
}

/** Returns ASC */
UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent()
{
    return AbilitySystemComponent;
}
```

Then, your character is ready to use GAS.

## UGameplayAbility

### Generating an Ability Class that inherits `UGameplayAbility`

This is an abstraction of each player behavior that affects either the player, or any other actors
in a world. There is a bunch of virtual method that you may override to define a task when each 

```c++
class UMyGameplayAbility : public UGameplayAbility {
    /** Returns whether the ability is castable (Checks required Mana, ) */
    virtual bool CanActivateAbility(
        ... // method parameters
    ) const override;
    
    /** A task when ability is activated (Spawns and shots a fireball) */
    virtual void ActivateAbility(
        ... // method parameters
    ) override;
    
    /**
     * A task when a player presses ability key *while* the ability is activated
     * (called both in client and server side)
     */
    virtual void InputPressed(
        ... // method parameters
    ) override;
    
    /**
     * A task when a player releases ability key *while* the ability is activated
     * (called both in client and server side)
     */
    virtual void InputReleased(
        ... // method parameters
    ) override;
    
    /** A task when the ability is forcely interrupted */
    virtual void CancelAbility(
        ... // method parameters
    ) override;
    
    /** A task when the ability is completely finished */
    virtual void EndAbility(
        ... // method parameters
    ) override;
    
    ...
}
```
The activation state is statefully managed by `UAbilitySystemComponent`, but finishing the
activation state is in charge of a developer. Note `UGameplayAbility` is not an instance
created each time when a player activates ability. It only defines *what to do* when each
lifecycle callback being triggered, and more likely to be instantiated when a player
*acquired* the ability.

### Adding Ability to `UAbilitySystemComponent`

Once you create a `UGameplayAbility` class, you'll give the ability to a player. Since the ability is
modularized, the point in time when ability is added to player is at the developer's discretion. For
some innate skills like combo attacks, the ability will be given to the player upon his joining a world.
Whereas, for some skills that is acquired during the gameplay, the ability will be given when the player
learns/unlocks/equips abilities.

In player cpp,
```c++
void APlayerCharacter::BeginPlay()
{
    if (AbilitySystemComponent)
    {
        ...
        // Arguments to create gameplay ability instance
        FGameplayAbilitySpec AbilitySpec(
            ComboAttackAbility, // Ability class (TSubclassOf)
            1,                  // Ability Level
            AbilityID,          // AbilityID (Used in mapping input actions)
            this                // Ability Owner
        );
        
        // Ability instance is created in this point and loaded to ASC
        AbilitySystemComponent->GiveAbility(AbilitySpec);
    }
}
```

There are a bunch of ways to activate an ability. You need to know different behaviors of
each lifecycle callback in client and server.

### Mapping Abilities to `InputAction`s (Client Side)

It is the most typical and classic way to activate an ability. A player presses key in client,
then a server accepts ability activation payload. On this approach, the only thing you need to
do is mapping the input action into the ability instance.

In player cpp,
```c++
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    ...
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        ...
        EnhancedInputComponent->BindAction(
            MyInputAction,                   // InputAction object
            ETriggerEvent::Start,            // Triggering time
            this,                            // Owner of the input action
            &APlayerCharacter::PressAbility, // Bind triggering callback method with one int32 parameter
            AbilityID                        // The AbilityID we featured on giving ability to ACS
        );
        
        EnhancedInputComponent->BindAction(
            MyInputAction,
            ETriggerEvent::Completed,
            this,
            &APlayerCharacter::ReleaseAbility,
            AbilityID
        );
    }
}
...
void ABeadurincCharacter::PressAbility(int32 InputId)
{
    // Triggers player input. CanActivateAbility and ActivateAbility is called if the ability is not activated.
    // Otherwise, InputPressed is called when the ability is activated
    AbilitySystemComponent->AbilityLocalInputPressed(InputId);
}

void ABeadurincCharacter::ReleaseAbility(int32 InputId)
{
    // If the ability is activated, InputRelease is called in consequence.
    AbilitySystemComponent->AbilityLocalInputReleased(InputId);
}
```
Remind that we used `AbilityID`(an integer constant) to give an ability to a player. There is no canonical way to decide
an ability id, but I recommend using an enum to map each behavior into a numeric value.

Each callback method gives an information of a key state whether it's being pressed or released. As it implies, GAS
distinguishes abilities by a unique ID that each ability has, optimizing payload shipping and ease the identification.

You may give an independent ability ID for all abilities, but this will cause infinitely increasement of keybinds, input
actions and their mappings. The core concept of having ability ID is based on one player action. Suppose you want to
create a wizard character with normal and ultimate spells. All normal spells(GameplayAbility) will maybe identified by
Ability ID 0. Then, ultimate spells will be identified by 1 since the wizard can't have multiple normal or ultimate spells.

| Normal spells(Ability ID = 0) | Ultimate spells(Ability ID = 1) |
|-------------------------------|---------------------------------|
| Fireball                      | Meteor                          |
| Lighting Bolt                 | Judgement Thunderstroke         |
| ,                             | ,                               |
| ,                             | ,                               |
| ,                             | ,                               |

And this is the ideal abstraction model that GAS suggests.

### Triggering Abilities Manually (Both in Client and Server)

Sometimes, you may want to activate abilities by an interaction of actors in a world. `UGameplayAbility` also supports
manual activation by a developer, but you should keep in mind their different actions in client and server.


## UAbilityTask



## UGameplayTag