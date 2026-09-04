---
title: 차량 자동 그룹화
---

## 현재 상태 (조사 결론)

- 그룹 데이터 구조: `src/group.h` `Group` (name/owner/vehicle_type/flags/livery/statistics/parent/number), `GroupID`, `DEFAULT_GROUP`;
- 기존 명령 (`src/group_cmd.h` / `group_cmd.cpp`):
  - `CmdCreateGroup`, `CmdDeleteGroup`, `CmdAlterGroup` (이름 변경/부모 그룹 설정)
  - `CmdAddVehicleGroup` (단일 차량을 그룹에 추가)
  - `CmdAddSharedVehicleGroup` (**특정 차량의 공유 명령 차량**을 **기존** 그룹에 추가)
  - `CmdCreateGroupFromList` (차량 목록에서 그룹 생성, `VL_SHARED_ORDERS` 목록 유형 지원; 자동 이름 지정)
- 자동 이름 지정 도우미: `GenerateAutoNameForVehicleGroup()` (group_cmd.cpp:899, 노선 시작/종료 도시 기준 `STR_VEHICLE_AUTO_GROUP_ROUTE` / `_LOCAL_ROUTE`) -- jgrpp에 이미 "노선별 이름 지정" 인프라 있음;
- 명령 등록 메커니즘: `command_type.h` `enum class Commands` + `DEF_CMD_TUPLE_NT` (매크로가 동시에 핸들러 선언 및 `CommandTraits` 등록);
- 차량↔그룹: `vehicle_base.h` `Vehicle::group_id`; 공유 명령 체인 `FirstShared()/NextShared()`; `OrderList *orders`.

**결론**: "전 회사 차량을 순회하여 공유 명령별로 자동 그룹 생성 및 배정"하는 전체 명령이 부족합니다.

## 본 기능 구현

### 새 명령 `Commands::AutoGroupSharedOrders` (`CmdAutoGroupSharedOrders`)

절차 (`src/group_cmd.cpp`):
1. 해당 회사의 지정된 유형 모든 주요 차량 순회 (`Vehicle::IterateTypeFrontOnly(type)`), 자사 차량이 아니거나 명령이 없거나 사용자 정의 그룹에 이미 속한 차량 건너뛰기;
2. 각 공유 명령 체인 카운트; 동일한 명령 목록을 공유하는 차량이 ≥2대일 때:
   - `GenerateAutoNameForVehicleGroup(v)`로 그룹명 생성 (예: "A도시 ↔ B도시");
   - `VehicleListIdentifier(VL_SHARED_ORDERS, ...)`를 구성하여 `Command<Commands::CreateGroupFromList>::Do(flags, ...)`를 중첩 호출하여 그룹 생성 및 모든 공유 차량 이동;
   - 완료 후 해당 그룹 내 차량의 `group_id`는 더 이상 기본 그룹이 아니므로 루프가 자동으로 건너뜀 (동일한 명령 목록에 대해 하나의 그룹만 생성);
3. `GroupChangeDeferredUpdateScope`가 그룹 통계를 일괄 지연 업데이트.

### 세 가지 진입점

| 진입점 | 위치 | 설명 |
|---|---|---|
| 그룹화 창 버튼 | `src/group_gui.cpp`에 새 `WID_GL_AUTOGROUP_SHARED` (도구 모음 LIVERY 옆), OnClick에서 명령 전송 | 스프라이트 재사용 `SPR_GROUP_CREATE_TRAIN + vtype` |
| 콘솔 명령 | `src/console_cmds.cpp` `autogroup [train\|road\|ship\|aircraft]` (매개변수 없음 = 네 가지 유형 모두 실행) | `IConsole::CmdRegister` |
| 스크립트/기타 | 모든 코드에서 `Command<Commands::AutoGroupSharedOrders>::Post(...)` 가능 | 명령이 네트워크 동기화를 거치므로 멀티플레이어 서버에서 안전 |

### 관련 파일

- `src/command_type.h`: `Commands` 열거형에 `AutoGroupSharedOrders` 추가 (**주의: 열거형 중간에 삽입하면 이후 명령 ID가 순서대로 이동하여 구버전 바이너리와 신버전 바이너리가 멀티플레이에서 불일치하지만, fork 내에서는 정상 현상임**)
- `src/group_cmd.h` / `group_cmd.cpp`: 명령 선언 및 구현
- `src/widgets/group_widget.h`: 새 컨트롤 ID
- `src/group_gui.cpp`: 도구 모음 버튼 (NWidget + OnPaint 스프라이트 + OnClick)
- `src/console_cmds.cpp`: 콘솔 명령
- `src/lang/english.txt`: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`

## 검증 포인트

1. 두 대 이상의 차량이 동일한 명령 목록 공유 → 버튼/`autogroup` 클릭 후 "노선명" 그룹 생성 및 모든 차량이 그룹에 배정됨;
2. 이미 그룹화된 차량은 다시 이동되지 않음; 다른 명령 목록은 각각 별도 그룹 생성;
3. 그룹 내 차량 통계 (수량/수익)가 올바르게 업데이트됨 (`GroupChangeDeferredUpdateScope`에 의존).