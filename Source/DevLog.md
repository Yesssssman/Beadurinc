개발로그

# Actors

## FighterCharacter

- 전투와 연관된 행위를 하는 모든 Character를 추상화한 클래스
- WeaponActor를 소유하고 있으며 AnimNotify를 통해 공격 타이밍에 충돌체를 활성화함
- AbilitySystemComponent의 소유자이며 전투 관련 로직들은 GameplayAbilities에 구현되어 있음 (코드 간결성 유지)

## WeaponActor

- FighterCharacter가 장비할 수 있는 무기에 대한 Blueprint
- StaticMesh와 충돌체, 그리고 공격력에 대한 정보를 가짐
- Pawn이나 지형과의 충돌을 방지하기 위해 커스텀 ObjectChannel인 Weapon타입으로 설정됨

## PlayerCharacter

- 플레이어 컨트롤러에 의해 조작되는 캐릭터
- EnhancedInput을 통해 Character갸 컨트롤됨
- InputBuffering 시스템이 적용되어 BufferWindow동안 입력 가능 상태가 되면 자동으로 입력된 어빌리티를 발동시킴

# Gameplay Abilities

## 캐릭터 속성값 설정 (`AttributSet`)



## 충돌 탐지와 데미지 적용 (`GameplayAbility`, `GameplayEvent`, `GameplayCue`)

플레이어 혹은 몬스터가 공격과 관련된 `AnimMontage`를 호출하면 해당 애니메이션의 공격 타이밍에 충돌한 모든 캐릭터에게 데미지를
적용하고 사운드+파티클을 발생시키도록 하고 싶었음. 이를 위해 `GameplayEffect`를 사용하여 `AttributeSet`의 값을 수정하도록 시도함.

```c++
AFighterCharacter* OtherFighter = Cast<AFighterCharacter>(OtherActor);

// Terminate when the other actor is myself or not implementation of AFightCharacter.
if (!OtherFighter || OtherFighter == this)
{
    return;	
}

UAbilitySystemComponent* ASCMyself = GetAbilitySystemComponent();
UAbilitySystemComponent* ASCOther = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
UCapsuleComponent* WeaponCollider = Cast<UCapsuleComponent>(OverlappedComponent);

if (ASCOther && ASCMyself && WeaponCollider && GetWeaponActor()->GetDamageEffect())
{
    // Create GE context specifying instigator and causer actor
    FGameplayEffectContextHandle GEContext = ASCMyself->MakeEffectContext();
    GEContext.AddInstigator(GetInstigator(), this);
    GEContext.AddSourceObject(GetWeaponActor());

    // Create GE spec handler to apply damage
    FGameplayEffectSpecHandle SpecHandle = ASCMyself->MakeOutgoingSpec(
        GetWeaponActor()->GetDamageEffect(),
        1.0F,
        GEContext
    );
    
    if (SpecHandle.IsValid())
    {
        ASCMyself->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASCOther);
    }
}
```

### `GameplayEffect` + `GameplayEffectExecutionCalculation` 구조의 약점

이전에 시도한 GameplayEffect는 AttributeSet의 값을 변경하는 데에는 Robust한 API를 제공해 주었지만, Blocking 이나 Parrying 같은
복잡한 로직 처리에는 부족하다 느낌. 따라서 `GameplayEvent`를 통해서 어빌리티 활성화에 필요한 메타데이터만 전달해주고 나머지는
`AFighterCharacter`에서 처리하는 방법을 고안함.

> *Gemini의 응답:*
>
> Your instinct to move away from a massive `ExecutionCalculation` for "Hit Logic" is correct. `ExecutionCalculations` are great for math (Damage = Attack - Defense), but they are terrible for state-driven logic like playing animations or checking block angles.
>
> **Is your approach valid?**
> 
> **Yes, with one important adjustment.** Instead of doing everything inside AFighterCharacter::applyMeleeHit via a standard C++ function, you should use SendGameplayEventToActor.
>
> This allows you to keep the flexibility of C++ for the data gathering, but uses the **Gameplay Ability System (GAS)** to handle the "State" (Stun/Block) through a `GA_HitReact` ability. This avoids casting every time and keeps your code clean.

요악하자면, `GameplayEffect + GameplayEffectExecutionCalculation` 구조는 복잡한 데미지 계산 공식을 수행하는데 탁월함이 있지,
Actor의 상태에 따라 수행해야 할 로직이 변경되는 설계에서는 한계가 명확하다는 답변. 하지만 AI의 제안은 `AFighterCharacter`에서
공격 반응을 처리하기 보단 공격 반응을 담당하는 `GameplayAbility` 클래스를 하나 만들어서 그 곳에서 공격 반응을 수행하다록 하는것이
더 바람직하다는 의견을 제시함.

`GameplayCue`와 `AttributeSet`이 GAS 플러그인에 포함되기 때문에 공격 반응 처리 함수를 `AFighterCharacter`에 포함시키는 것 보다
`GameplayAbility`로 만드는것이 더 API 친화적인 설계라고 판단함. 결정적으로 `GameplayEvent`는 `GameplayAbility`의 활성화를 위해
설계된 장치이기 때문에 AI의 답변을 채택함.

# 플레이어 (APlayerCharacter)

## 기본 공격 (EnhancedInput, GameplayAbility)

### Input Buffering