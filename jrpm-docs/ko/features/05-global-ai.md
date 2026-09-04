---
title: 전판 인식 AI 인터페이스
---

## 현재 상태 (조사 결론)

- NoAI 프레임워크: `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config) -- 원본 AI 체계 **그대로 유지**, 본 기능은 건드리지 않음;
- 스크립트 API 체계: `src/script/api/script_*.hpp/.cpp` (60+ 클래스), Squirrel 바인딩은 빌드 도구가 **자동 생성**:
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)`가 새 API 클래스를 자동 발견 (`ai_*.sq.hpp` / `gs_*.sq.hpp` 생성), **새 클래스에 수동 등록 불필요**;
  - `.cpp`는 CMake 소스 목록에 추가해야 함; 클래스 주석 `@api ai game`으로 AI/GS 노출 제어;
- 회사 데이터: `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy), `GetAvailableMoney()`;
- GS 신 모드: `ScriptCompanyMode::IsDeity()`;
- 기존 제한: AI는 기본적으로 자신의 회사 데이터만 안정적으로 접근 가능, 전판 집계 API 없음, 접근 스위치 없음.

**결론**: `ScriptGlobal` API 클래스 (자동 등록) + `game.script.allow_global_ai_access` 스위치를 추가하면 "NoAI 유지, 전판 인식 AI 추가, 접근 제어 포함"을 구현할 수 있습니다.

## 본 기능 구현

### 1. `ScriptGlobal` API (`src/script/api/script_global.hpp/.cpp`)

AI와 GS에 모두 노출 (`@api ai game`)되는 정적 메서드:

| 메서드 | 반환 | 데이터 소스 |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| 설정 스위치` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`, `GetDate()`, `GetYear()` | int | `MapSizeX/Y`, `EconTime::CurDate`, `CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()`로 owner별 카운트 |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- 회사 ID는 `ScriptCompany::CompanyID` 재사용 (COMPANY_SELF는 현재 회사로 해석);
- **접근 제어**: 모든 메서드 첫 줄에서 `IsGlobalAccessAllowed()` 검증, 불만족 시 -1/nullopt/false 반환 (GS는 항상 사용 가능, AI는 스위치 제어);
- `ScriptGlobalCompanyList : ScriptList`: `ScriptList::FillList<Company>`로 모든 회사 열거.

### 2. 접근 스위치 (설정)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`, 기본 false, `SC_EXPERT`);
- 구조체 필드 `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`);
- 문자열 `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`).

### 3. 예시 AI: `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController);
- 데모: 모든 회사 재무/차량/역/평가 및 맵 정보를 읽고 로그 출력; 접근 제어 설명 (스위치 미활성화 시 메시지 표시);
- 이 AI는 "전판 인식 + 접근 제어"의 참조 구현이며, 추후 AI 제어 로직은 `Start()` 내에서 확장 가능.

### 관련 파일

- 새 파일 `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` ( .cpp 추가)
- `src/table/settings/script_settings.ini`, `src/settings_type.h`, `src/lang/english.txt`
- 새 파일 `bin/ai/GlobalAI/info.nut`, `main.nut`

## 검증 포인트

1. 새 AI가 AI 설정 화면에서 보임 (`ai/GlobalAI` 스캔);
2. `game.script.allow_global_ai_access`를 활성화하지 않으면 AI 로그에 권한 없음 메시지; 활성화 후 각 회사의 전체 데이터 출력;
3. GameScript는 스위치 없이 `GSGlobal`에 접근 가능;
4. 빌드 시 자동 생성된 `ai_global.sq.hpp` / `gs_global.sq.hpp` 오류 없음 (`file(GLOB)`에 의존하므로 CMake 재설정 필요).

## 확장 방향

- 경제/대출/인프라 (`GetCompanyInfrastructure`) 등 더 많은 집계 데이터 추가;
- 이벤트 (회사 파산/인수/신규 회사 설립) 구독 추가;
- AI 결정 로직을 구성 가능한 매개변수 (`GetSettings()`)로 만들기.