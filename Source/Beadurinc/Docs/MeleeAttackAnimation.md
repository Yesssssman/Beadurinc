# 근접 공격 애니메이션

- `FighterCharacter`의 근접 공격은 `MeleeTraceAnimationNotify`를 보유한 Animation Montage에 의해 이루어짐
- `MeleeTraceAnimationNotify`는 `AnimNotifyState` 를 상속받으며, 지정된 시간동안 `WeaponActor`의 충돌체를 활성화시켜
  Overlap된 액터들에게 데미지를 적용함
- 근접 공격은 한 번 휘두를 때 동일한 액터를 여러번 공격해서는 안되므로 데미지를 적용한 액터들을 하나의 Set에 저장하고 애니메이션이
  종료될 때 해당 Set을 초기화함

**FighterCharacter.h**
```c++
    /** List of Actors that hit by "current swing" */
    TSet<TObjectPtr<AActor>> HitActors;
```

**FighterCharacter.cpp**
```c++
void AFighterCharacter::OnMeleeContacts
(
    ...
)
{
    AFighterCharacter* OtherFighter = Cast<AFighterCharacter>(OtherActor);
    ...
    // Prevents "multiple hits" by checking whether the other actor is registered in HitActor container.
    if (HitActors.Contains(OtherActor))
    {
        return;
    }
    ...
}

void AFighterCharacter::ResetMeleeSwing()
{
    // Clear hit actors saved during melee track phase
    HitActors.Empty();
}
```

**MeleeTraceAnimationNotify.cpp**
```c++
void UMeleeTraceAnimationNotify::NotifyBegin
(
    ...
)
{
    // Check if the owner is a weapon holdable character
    if (AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner()))
    {
        // Activate collision check when contacting phase starts
        FighterCharacter->ResetMeleeSwing();
        FighterCharacter->GetWeaponActor()->SetActorEnableCollision(true);
    }
}

void UMeleeTraceAnimationNotify::NotifyEnd
(
    ...
)
{
    // Check if the owner is a weapon holdable character
    if (AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(MeshComp->GetOwner()))
    {
        // Deactivate collision check when contacting phase ends
        FighterCharacter->ResetMeleeSwing();
        FighterCharacter->GetWeaponActor()->SetActorEnableCollision(false);
    }
}
```

## Collider Overlap 이벤트의 **Dynamic Delegate** 설정

`WeaponActor`가 스폰될 때 해당 액터의 `CapsuleComponent`에서 OnComponentBeginOverlap의 Dynamic Delegate 설정해주었다. 여기서
Dynamic Delegate 란 해당 액터에서 델리게이트가 Subscribe중인 이벤트가 발생할 시에 실행할 작업을 함수 포인터 형태로 넘겨주어서
클래스 간의 Tight Coupling을 제거하는 소프트웨어 디자인 패턴이다.

```c++
void AFighterCharacter::BeginPlay()
{
    ...
        if (UCapsuleComponent* CapsuleCollider = WeaponActorInstance->FindComponentByClass<UCapsuleComponent>())
        {
            // Gives "NoCollision" at first since weapon only can hurt other characters when activated by anim notify.
            CapsuleCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            CapsuleCollider->OnComponentBeginOverlap.AddDynamic(this, &AFighterCharacter::OnMeleeContacts);
        }
    ...
}
```
*결과*

![AttackAnim](img/AttackAnim.gif)

`WeaponActor`가 월드 내부의 다른 액터와 오버랩 될 때마다 스크린에 메세지를 띄움