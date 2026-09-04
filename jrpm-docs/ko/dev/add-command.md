---
title: 새 게임 명령 추가
---

# 새 게임 명령 추가

본 프로젝트에서 구현된 `Commands::AutoGroupSharedOrders` (차량 자동 그룹화)를 템플릿으로 하여, jrpm에 새 명령을 추가하는 전체 단계를 소개합니다.

## 1. 명령 열거형에 선언

`src/command_type.h`, `enum class Commands` 중간에 삽입 (이후 ID가 순서대로 이동):

```cpp
AddSharedVehiclesToGroup,               ///< add all other shared vehicles to a group which are missing
AutoGroupSharedOrders,                  ///< auto-group all vehicles by their shared order lists
```

## 2. 명령 핸들러 선언 및 등록

`src/group_cmd.h` (`DEF_CMD_TUPLE_NT`는 동시에 핸들러 함수를 선언하고 `CommandTraits`를 등록):

```cpp
DEF_CMD_TUPLE_NT(Commands::AutoGroupSharedOrders, CmdAutoGroupSharedOrders, {}, CommandType::RouteManagement, CmdDataT<VehicleType>)
```

- `_NT`: 타일 매개변수 없음 (타일이 있으면 `DEF_CMD_TUPLE` 사용);
- `CmdDataT<...>`: 매개변수 유형 목록, 핸들러가 순서대로 언패킹.

## 3. 핸들러 구현

`src/group_cmd.cpp`:

```cpp
CommandCost CmdAutoGroupSharedOrders(DoCommandFlags flags, VehicleType type)
{
	if (!IsCompanyBuildableVehicleType(type)) return CMD_ERROR;

	CommandCost total_cost;
	for (const Vehicle *v : Vehicle::IterateTypeFrontOnly(type)) {
		// ... 비즈니스 로직 ...
		CommandCost ret = Command<Commands::CreateGroupFromList>::Do(flags, vli, CargoFilterCriteria::CF_ANY, name);
		if (ret.Failed()) return ret;
		total_cost.AddCost(ret.GetCost());
	}
	return total_cost;
}
```

포인트:
- `CMD_ERROR` 반환은 실패를 의미 (`CommandCost(INVALID_STRING_ID)`);
- `flags.Test(DoCommandFlag::Execute)`로 "실제 실행"과 "테스트 모드" 게이트 제어;
- 중첩 명령은 `Command<Commands::X>::Do(flags, ...)` 사용;
- GUI/네트워크 호출은 `Command<Commands::X>::Post(err_string, args...)` 사용 (자동 네트워크 동기화).

## 4. 진입점 추가

### 창 버튼 (선택 사항)

1. `src/widgets/<system>_widget.h` 열거형에 `WID_XXX` 추가;
2. 창 NWidget 레이아웃에 버튼 추가;
3. `OnClick`에 `case` 추가:
   ```cpp
   case WID_GL_AUTOGROUP_SHARED: {
       Command<Commands::AutoGroupSharedOrders>::Post(STR_ERROR_GROUP_CAN_T_CREATE, this->vli.vtype);
       break;
   }
   ```

### 콘솔 명령 (선택 사항)

`src/console_cmds.cpp`:

```cpp
static bool ConAutoGroup(std::span<std::string_view> argv) { /* ... */ }
// IConsoleStdLibRegister() 중:
IConsole::CmdRegister("autogroup", ConAutoGroup);
```

### 문자열 (선택 사항)

`src/lang/english.txt`에 추가 (예: `STR_GROUP_AUTOGROUP_SHARED_TOOLTIP`).

## 5. 완료 체크리스트

- [ ] `command_type.h` 열거형
- [ ] `<system>_cmd.h`의 `DEF_CMD_TUPLE*`
- [ ] `<system>_cmd.cpp` 핸들러
- [ ] (선택 사항) 컨트롤/콘솔/문자열
- [ ] 네트워크 및 저장 자동 적응 (`CommandTraits` 자동 생성)