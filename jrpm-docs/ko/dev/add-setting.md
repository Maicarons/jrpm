---
title: 새 설정 항목 추가
---

# 새 설정 항목 추가

jrpm의 설정 시스템은 **INI 기반**: `src/table/settings/*.ini`가 설정 정의 소스이며, 빌드 시 settingsgen이 코드를 생성; 해당 구조체 필드는 `src/settings_type.h`에 있습니다. 본 프로젝트에서 구현된 `network.content_download_parallel`을 예시로 합니다.

## 1. 구조체 필드

`src/settings_type.h`, 해당 설정 구조체 찾기 (네트워크 → `NetworkSettings`, 게임 → `GameSettings` 관련 하위 구조, 스크립트 → `ScriptSettings`):

```cpp
struct NetworkSettings {
	// ...
	std::string content_mirrors;                          ///< 쉼표로 구분된 미러 URI
	uint8_t content_download_parallel = 4;                ///< 병렬 다운로드 파일 수
	// ...
};
```

## 2. INI 정의

`src/table/settings/network_settings.ini`, `[SDTC_VAR]` 블록 추가 (클라이언트 설정은 `SDTC_*`, 게임 설정은 `SDT_*` 사용):

```ini
[SDTC_VAR]
var      = network.content_download_parallel
type     = SLE_UINT8
flags    = SettingFlag::NotInSave, SettingFlag::NoNetworkSync
def      = 4
min      = 1
max      = 8
cat      = SC_BASIC
```

자주 사용하는 필드:

| 필드 | 설명 |
|---|---|
| `var` | 설정 전체 이름 (구조체 경로, 예: `network.xxx` / `game.script.xxx`) |
| `type` | `SLE_UINT8/16/32/64`, `SLE_INT*`, `SLE_BOOL`, `SLE_STR` (문자열, `length` 필요) |
| `flags` | `SettingFlag::NotInSave` (저장 안 함), `NoNetworkSync` (동기화 안 함), `NetworkOnly`, `GuiZeroIsSpecial` 등 |
| `def/min/max/interval` | 기본값/범위/단계 |
| `str/strhelp` | 설정 화면 표시 문자열 |
| `cat` | 설정 분류 (`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`) |

## 3. 문자열 (설정 화면 표시)

`src/lang/english.txt`:

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :Allow AIs to access whole-game data: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :Allow AIs to use the Global API...
```

- 부울 설정은 `{STRING2}` (켜기/끄기)로 끝남;
- 숫자 설정은 동일한 유형의 문자열 형식 참조.

## 4. 코드에서 사용

```cpp
// 클라이언트 설정
_settings_client.network.content_download_parallel

// 게임 설정
_settings_game.script.allow_global_ai_access
```

## 5. 특수 시나리오

- **서버 설정**: `network.*`는 멀티플레이에서 서버가 전송 (`NetworkOnly` + 서버 `sync` 메커니즘);
- **저장 연관**: 게임 설정 (`game.*`)은 저장과 함께 저장됨 (기본값); 클라이언트 설정 (`network.*`/`gui.*`)은 기본적으로 `NotInSave`;
- **콜백**: `pre_cb`/`post_cb`로 값 변경 처리 훅 가능 (예: `UpdateClientConfigValues()`).

## 완료 체크리스트

- [ ] `settings_type.h` 필드
- [ ] 해당 `.ini`의 `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` 블록
- [ ] (설정 화면 표시용) english.txt 문자열
- [ ] CMake 재설정 + 빌드