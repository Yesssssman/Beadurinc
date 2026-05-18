#### Premises

- When you analyze code, do not assume functionalities of modules just by names. Read all cod lines and understand how
  they exactly work.
- Do not start implementing after you finish writing the plan file. I'll review your plans first, and will ask to start 
  implementing.
- Insert the full code snippet so I can grasp the context what you're exactly going to do.

# Goals

- Expand the functionality of `ComboAttackGameplayAbility` to play Dash and Heavy attacks also
  - Dash attack should be triggered when player is sprinting. (you can judge whether the player is sprinting by
    CharacterMovement component's WalkSpeed field)
  - Heavy attack should be triggered when player holds the attack key (mapped by EnhancedInputSystem)
- Suggest me various ways to implement such functionalities like adding parameter to GAS triggering context, or creating
  a new `GameplayAbility` to trigger them with new input mappings (For input mappings, look `APlayerCharacter` how
  abilities are triggered by player inputs). If there is a self-evident way that no need to consider other implementation
  methods, let me know and don't try to struggle to suggest me another methods.
