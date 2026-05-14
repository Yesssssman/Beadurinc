#### Premises

- When you analyze code, do not assume functionalities of modules just by names. Read all cod lines and understand how
  they exactly work.
- Do not start implementing after you finish writing the plan file. I'll review your plans first, and will ask to start 
  implementing.
- Insert the full code snippet so I can grasp the context what you're exactly going to do.

# Goals

- Make monster-version blocking and parrying gameplay ability by referencing `BlockParryGameplayAbility`.
  - The ability is for player, it is handled by user inputs and has a solid rule to activate "parrying"
  - For monsters, in other words the actors controlled by `AIController`, we do not need the validate process specified
    at `CanActivateAbility` since they're conducted on Behavior Tree.
  - Split Blocking and Parrying abilities so that they can be triggered based on "behavior score" in BehaviorTree.
  - I'll be in charge of handling ability blueprints and building behavior trees. You're responsibility is making c++
    code that activates mob's proper state 
  - Block & Parry states are distinguished by according `GameplayTags`.
