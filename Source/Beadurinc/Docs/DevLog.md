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

## 캐릭터 속성값 (`AttributSet`)

체력, 스태미나처럼 게임플레이의 중요한 부분을 차지하는 변수값들은 일반 필드로 관리되기 보다는 하나의 공통된 모듈을 통해 관리되는것이
유리하다. 이들은 주로 화면에 UI 형식으로 정보를 나타내야 하므로 최대값을 저장하여 현재값과 최대값의 비율을 Widget으로 전송하는 것이
코드 간결성의 측면에서 이점이 있을 것이다.

Gameplay Abilities 플러그인에는 이러한 변수들을 각각 `Attribute`로 취급하여 하나의 `AttributeSet`으로 구성하는 기능을 제공한다.
각 `Attribute`들은 최대값, 최소값, 기본값을 가질 수 있고 Modifier를 적용하여 속성 값의 변화를 효율적으로 추적, 관리할 수 있다.
(예를 들면 플레이어의 기본 공격력이 10이고 현재 +5의 공격력 버프 스킬이 적용되어 있다면, 최종 공격력은 15로 계산될 것이고 버프 스킬
의 지속시간이 끝난다면 Modifier를 제거하여 기본 공격력 10으로 쉽게 복구할 수 있다.)

또한 언리얼 엔진의 Replication 동기화 시스템의 제어를 받으므로 클라이언트-서버 방식의 멀티플레이 구현에도 높은 이점을 제공한다.
개발자가 Replication Policy만 설정해주면 복잡한 네트워킹 함수 없이도 `Attribute`들의 변경 사항을 클라이언트에 동기화 할 수 있다.

