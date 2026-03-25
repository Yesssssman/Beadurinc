# Fighter AI

- 정찰, 적 탐지, 공격 등 `AFighterCharacter`를 상속받은 모든 논플레이어블 액터는 AI에 의해 컨트롤됨
- AI를 동작시킬 때에는 `AIController`를 상속받는 블루프린트 클래스를 `Character`블루프린트에서 설정함
- AI를 구성하는 방법은 여러가지가 있으나 언리얼 엔진에서는 **Behavior Tree**와 **Blackboard**를 조합하여 트리 구조를 이용한 분기
  처리 방식이 이상적임

## Behavior Tree + Blackboard

- Behavior Tree는 상황별로 처리될 분기 로직을 트리 형태로 나타낼 수 있으며 트리의 최상단에서부터 왼쪽 노드를 방문함 (전위순회)
- 노드 종류는 하위 노드 분기용인 Composite와 캐릭터의 행위를 명시한 Task 노드가 있음
- Composite 노드는 브랜치 노드, Task 노드는 리프 노드를 담당함





### `BTTask_PlayMontage`

- 언리얼에서 기본 제공하는 `BTTask_PlayAnimation`은 현재 재생중인 애니메이션 블루프린트를 덧씌워서 애니메이션을 재생하기 때문에
  Fade Out시 T-Pose와 블렌딩되는 문제점이 있음
- 이를 해결하기 위해 `BTTask_PlayAnimation`를 참고하여 `PlayMontage` 함수를 호출하는 커스텀 태스크를 작성함