# Hit React

**Hit React**란 플레이어 혹은 몬스터가 공격 당하는 이벤트가 발생하였을 때 이루어지는 일련의 작업들을 말하며 여기에는 경직 애니메이션
재생, 체력 저하, 사운드 혹은 파티클 재생 등이 해당된다. 해당 프로젝트는 근접 공격이 위주로 된 Soul-Like 장르이므로 무기에 해당하는
액터의 충돌체가 캐릭터와 Overlap 될 때 이벤트를 발생시키고 그에 따른 Hit React를 수행하도록 설계하였다.

### 첫 번째 시도 (`GameplayEffect`, `GameplayEffectExecutionCalculation`)

플레이어 혹은 몬스터가 공격 애니메이션 몽타주를 재생하면 해당 애니메이션의 공격 타이밍에 충돌한 모든 캐릭터에게 데미지를
적용하고 사운드+파티클을 발생시키도록 함. 이를 위해 `GameplayEffect`를 사용하여 `AttributeSet`의 값을 수정하도록 시도함.

**FighterCharacter.cpp**
```c++
/** Called on `WeaponActor` overlaps any opponent */
void AFighterCharacter::OnMeleeContacts
(
  ...
)
{
  ...
  
  if (ASCOther && ASCMyself && WeaponCollider && GetWeaponActor()->GetDamageEffect())
  {
      // Create GE context specifying instigator and causer actor
      FGameplayEffectContextHandle GEContext = ASCMyself->MakeEffectContext();
      ...
  
      // Create GE spec handler to apply damage
      FGameplayEffectSpecHandle SpecHandle = ASCMyself->MakeOutgoingSpec(
          GetWeaponActor()->GetDamageEffect(),
          1.0F,
          GEContext
      );
      
      if (SpecHandle.IsValid())
      {
          // Apply GameplayEffect to the opponent's AbilitySystem component
          ASCMyself->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASCOther);
      }
  }
}
```

**GEEC_StrikeTarget.cpp**
```c++
void UGEEC_StrikeTarget::Execute_Implementation
(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
) const
{
    ...
    // Extract the original magnitude (inflicts to health)
    float OriginalDamage;
    ...
    
    // If the hitter is taking a blocking stance, deflate stamina and apply blocking gameplay cue
    if (bIsBlocking) {
        float StaminaDamage = OriginalDamage * 0.5f;
        if (OnBlock) OwnerCharacter->PlayAnimMontage(OnBlock);
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(GetDamageStatics().StaminaProperty, EGameplayModOp::Additive, StaminaDamage));
        TargetASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeBlock")), CueParams);
    }
    // If not blocking, apply damage to health and play stun animation montage
    else
    {
        if (OnHurt) OwnerCharacter->PlayAnimMontage(OnHurt);
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(GetDamageStatics().HealthProperty, EGameplayModOp::Additive, OriginalDamage));
        TargetASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeHurt")), CueParams);
    }
}
```

### `GameplayEffect` + `GameplayEffectExecutionCalculation` 구조의 한계

구현하고 보니 무언가 매끄럽지 않다고 느낌. `GameplayEffectExecutionCalculation`에서 기존 데미지를 복사하여 새로운 Context에
붙여넣는 과정이 번거롭게 느껴졌고 무엇보다 `GameplayEffect` 에서 애니메이션 몽타주 재생, `GameplayCue` 재생을 담당해야 하는지
의문이 생김.

따라서 `GameplayEvent`를 통해서 Hit React 실행에 필요한 메타데이터만 전달해주고 나머지는 `AFighterCharacter`에서 처리하는 방법을
고안하여 생성형AI에게 자문을 구함.

> *Gemini의 응답:*
>
> Your instinct to move away from a massive `ExecutionCalculation` for "Hit Logic" is correct. `ExecutionCalculations` are great for math (Damage = Attack - Defense), but they are terrible for state-driven logic like playing animations or checking block angles.
>
> **Is your approach valid?**
>
> **Yes, with one important adjustment.** Instead of doing everything inside AFighterCharacter::applyMeleeHit via a standard C++ function, you should use SendGameplayEventToActor.
>
> This allows you to keep the flexibility of C++ for the data gathering, but uses the **Gameplay Ability System (GAS)** to handle the "State" (Stun/Block) through a `GA_HitReact` ability. This avoids casting every time and keeps your code clean.

요악하자면, `GameplayEffect + GameplayEffectExecutionCalculation`를 혼합한 방법은 데미지를 **계산**하는 **수학 공식**에 대한
솔루션을 제공하기 위해 설계되었으며 `Actor`의 상태에 따라 수행해야 할 **로직이 변경**되는 상황에서는 적합하지 않다는 답변. 더하여
Gemini는 `AFighterCharacter`에서 공격 반응을 처리하는 함수를 작성하기보단 공격 반응을 담당하는 `GameplayAbility` 클래스를 하나
만들어서 그곳에 모든 Hit React 수행에 필요한 로직을 작성하는것이 코드 간결성 측면에서 낫다는 의견을 피력함.

`GameplayCue`와 `AttributeSet`이 GAS 플러그인에 포함되기 때문에 공격 반응 처리 함수를 `AFighterCharacter`에 포함시키는 것 보다
`GameplayAbility`로 만드는것이 더 API 친화적인 설계라고 판단함. 결정적으로 `GameplayEvent`는 `GameplayAbility`의 활성화를 위해
설계된 장치이기 때문에 AI의 답변을 채택함.

### 두 번째 시도 (`GameplayEvent`, `GameplayAbility`)

**FighterCharacter.cpp**
```c++
/** Called on `WeaponActor` overlaps any opponent */
void AFighterCharacter::OnMeleeContacts
(
  ...
)
{
  AFighterCharacter* OtherFighter = Cast<AFighterCharacter>(OtherActor);
  
  ...
  
  // Payload to contain data that is used in triggering hurt event 
  FGameplayEventData EventContext;
  
  // Fill the context
  EventContext.Instigator = this;
  EventContext.Target = OtherActor;
  EventContext.OptionalObject = GetWeaponActor();
  EventContext.EventMagnitude = GetWeaponActor()->GetWeaponBaseDamage();
  EventContext.ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
  
  ...
  
  // Trigger GameplayEvent
  UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, GameplayEventTags::Event_Combat_Hit, EventContext);
}
```

**HitReactGameplayAbility.cpp**
```c++
void UHitReactGameplayAbility::ActivateAbility
(
  ...
)
{
  ...
  
  if (OwnerCharacter && OwnerACS)
  {
    FGameplayCueParameters CueParams;
    FGameplayEffectContextHandle Context = OwnerACS->MakeEffectContext();
    ...
    
    // When blocking activated
    if (OwnerACS->HasMatchingGameplayTag(StateGameplayTags::State_Blocking))
    {
      if (OnBlock) OwnerCharacter->PlayAnimMontage(OnBlock);
      // Plays gameplay cue for block
      OwnerACS->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeBlock")), CueParams);
      // TODO: add stamina consume
    }
    else
    {
      if (OnHurt) OwnerCharacter->PlayAnimMontage(OnHurt);
      // Plays gameplay cue for hurt
      OwnerACS->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeHurt")), CueParams);
      // TODO: add health damage
    }
    
    ...
  }
}
```

체력, 스태미나와 같은 Attribute값 변경과 이펙트를 재생하는 GameplayCue, Hit Stun 애니메이션 몽타주 재생 등 Hit React에 로직은 전부
GameplayAbility내에서 구현함. 이렇게 함으로써 Hit React 로직을 `FighterCharacter` 로부터 분리시킬 수 있었으며 GameplayEvent를
호출만 하면 Hit React를 발동시킬 수 있으므로 코드 간결성 또한 달성 가능함.

*결과*

![HitReactDemo](img/HitReactDemo.gif)