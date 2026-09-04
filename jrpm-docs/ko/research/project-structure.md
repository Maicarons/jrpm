---
title: 프로젝트 구조와 코드 조직 조사
---

> 조사 대상: `G:\GitHub\OpenTTD-patches` (jgrpp 소스 코드 작업 영역)
> 참고 자료: `G:\game\openttd-jgrpp` (컴파일된 완전한 게임, 읽기 전용)
> 조사 일자: 2026-08-14
> 브랜치: jgrpp (현재 HEAD `5b5c452e1b`, 약 16.0 버전)

---

## 1. 프로젝트 개요

OpenTTD jgrpp (JGR's Patchpack)는 OpenTTD 기반의 유명한 강화 브랜치이며, 본 작업 영역 소스 코드는 버전 **16.0**에 해당하며, 대량의 JGR 고유 기능 (tracerestrict, programmable signals, scheduled dispatch, 템플릿 교체, 강화 신호 등)을 포함합니다.

| 항목 | 설명 |
|---|---|
| 언어 | C++20 (`CMAKE_CXX_STANDARD 20`, 확장 없음) |
| 빌드 | CMake (≥3.17), `src/CMakeLists.txt`가 디렉토리별로 구성 |
| 스크립트 엔진 | Squirrel (`src/3rdparty/squirrel/`), AI / GameScript / 템플릿 스크립트용 |
| 네트워크 | 자체 개발 TCP/UDP 프로토콜 계층 (`src/network/`), HTTP는 WinHttp (Windows) / libcurl (기타) / JS (Emscripten) 사용 |
| 저장 | `src/saveload/` + `src/sl/` 자체 개발 바이너리 형식 |
| 스레드 | `src/thread.h` + `src/worker_thread.cpp` (WorkerThreadPool 작업 풀), `src/timer/` 타이머 |

---

## 2. 소스 코드 디렉토리 구조와 모듈 구분

최상위: `CMakeLists.txt`가 루트 빌드 스크립트; `src/`가 모든 C++ 소스 코드; `bin/`이 런타임 데이터 (AI 호환 스크립트, 언어 파일 등); `media/`, `os/`, `cmake/`, `docs/`가 각각 리소스, 플랫폼 코드, 빌드 스크립트와 문서.

`src/` 내 주요 하위 디렉토리와 역할:

| 디렉토리 | 역할 |
|---|---|
| `src/core/` | 기본 도구: pool 유형, bitset, 컨테이너, 스레드 래퍼, 비트 연산 등 |
| `src/network/` | 네트워킹: 서버/클라이언트/UDP/HTTP/콘텐츠 다운로드/관리자 프로토콜 (`core/` 하위 디렉토리는 프로토콜 기초) |
| `src/script/` | 스크립트 실행 프레임워크: Squirrel 래퍼, 인스턴스, 설정; `api/`는 AI/GS에 노출되는 모든 API 클래스 |
| `src/ai/` | NoAI 프레임워크 (AI 인스턴스, 스캐너, 설정, GUI) |
| `src/game/` | GameScript 프레임워크 (GS 인스턴스, 설정, GUI) |
| `src/newgrf/` | NewGRF 디코딩 및 처리 |
| `src/pathfinder/` | 경로 탐색 (YAPF, NPF) |
| `src/saveload/`, `src/sl/` | 저장 읽기/쓰기 |
| `src/blitter/`, `src/video/`, `src/fontcache/`, `src/music/`, `src/sound/` | 렌더링/오디오/비디오 백엔드 |
| `src/lang/` | 지역화 문자열 소스 (strgen 생성) |
| `src/table/` | 정적 테이블; `table/settings/*.ini`가 **설정 정의 소스** (settingsgen 생성) |
| `src/3rdparty/` | 서드파티 라이브러리 (squirrel, llvm, icu 등) |
| `src/timer/`, `src/os/`, `src/misc/` | 타이머, 플랫폼, 기타 |

최상위 산재 파일은 시스템별로 이름 지정 (예: `rail_gui.cpp`, `group_cmd.cpp`, `vehicle.cpp`, `order_cmd.cpp`, `economy.cpp`), OpenTTD 관례 준수: `*_cmd` 명령 로직, `*_gui` 창, `*_base/_type/_func` 데이터 구조와 인라인 함수.

---

## 3. 빌드 방식

- **CMake 3단계**: `cmake -B build ..` → `cmake --build build` → 산출물 `openttd.exe`. 저장소에 `build.sh` / `build-dedicated.sh` 포함.
- **의존성**: `vcpkg.json` 선언 (zlib, lzma, lzo, zstd, png, SDL2, freetype, harfbuzz, icu, opus 등); Windows는 WinHttp 사용 (curl 불필요), 비 Windows는 libcurl 사용 (`CMakeLists.txt:121-127`).
- **도구 체인 (host tools)**: `strgen` (언어 파일), `settingsgen` (`src/table/settings/*.ini`에서 설정 코드 생성), `squirrel_export` (`src/script/api/script_*.hpp`에서 Squirrel 바인딩 생성).
- **주요 생성물**: `generated/script/api/<ai|gs>/...sq.hpp` (API 바인딩, **`file(GLOB script_*.hpp)`로 자동 발견, 새 API 클래스는 등록 목록 수정 불필요**, `.hpp`만 추가하고 `.cpp`를 `src/script/api/CMakeLists.txt` 소스 목록에 추가하면 됨); `generated/rev.cpp`; `generated/ottdres.rc`.
- **설정 시스템**: 현대 버전은 **INI 기반**으로 변경됨 -- `src/table/settings/*.ini` (`[SDTC_VAR]` 섹션, `cat=SC_*` 분류, `flags`, `post_cb` 등 포함), settingsgen이 `settings_*.cpp/h`를 생성; 해당 구조체 멤버는 `src/settings_type.h`에 있음 (예: `NetworkSettings`는 575행부터).

---

## 4. 다섯 가지 기능 시스템 구현 위치

### 4.1 리소스 다운로드 (콘텐츠 다운로드 / BaNaNaSplit)

| 관심 지점 | 위치 |
|---|---|
| 콘텐츠 클라이언트 메인 클래스 | `src/network/network_content.h/.cpp` -- `ClientNetworkContentSocketHandler` (겸 `ContentCallback` + `HTTPCallback`) |
| 콘텐츠 다운로드 GUI | `src/network/network_content_gui.cpp/.h` |
| HTTP 클라이언트 | `src/network/core/http.h/.cpp` -- `NetworkHTTPSocketHandler::Connect(uri, callback, data)`, 비동기 이벤트 기반 (비블로킹, 메인 루프 폴링) |
| 콘텐츠 서버 연결 문자열 | `src/network/core/config.cpp` -- `NetworkContentServerConnectionString()`: 환경 변수 `OTTD_CONTENT_SERVER_CS`, 기본값 `content.openttd.org` (TCP 메타데이터 프로토콜) |
| 미러 URI | `src/network/core/config.cpp` -- `NetworkContentMirrorUriString()`: 환경 변수 `OTTD_CONTENT_MIRROR_URI`, 기본값 `https://binaries.openttd.org/bananas` |
| 다운로드 흐름 | `DownloadSelectedContent()` → `DownloadSelectedContentHTTP()` (모든 content ID를 미러에 POST, 미러가 다중 파일 tar 스트림 반환, 파일별로 디스크에 쓰기) → `AfterDownload()`에서 gunzip + `TarScanner` 압축 풀기 |
| 압축 해제 | `GunzipFile()` (zlib), `TarScanner`/`ExtractTar` (`src/tar_type.h` / `src/fileio.cpp`) |
| 스레드 시설 (병렬에 사용 가능) | `src/worker_thread.h/.cpp` -- `WorkerThreadPool` + `EnqueueJob`; `src/thread.h`는 플랫폼 스레드 래퍼 |

**현재 상태 결론**: ① 미러가 1개뿐이고 환경 변수로만 설정 가능, 게임 내 설정 없음; ② 다운로드는 **단일 연결, 직렬** (한 번에 모든 파일 POST); ③ 압축 해제가 메인 스레드에서 동기 실행. → 다중 스레드/다중 미러 개조 지점이 명확함.

### 4.2 서버 온라인 플레이어 / 회사 상한

| 관심 지점 | 위치 |
|---|---|
| 클라이언트 상한 상수 | `src/network/network_type.h:21` -- `static const uint MAX_CLIENTS = 255;` |
| 클라이언트 풀 | 동일 파일 `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>`; `ClientID`는 `uint32_t` |
| 회사 ID 풀 | `src/company_type.h` -- `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15`; 가짜 회사가 253/254/255 점유 |
| 회사 마스크 | 동일 파일 `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>` (16비트, 16개 회사만 추적 가능) |
| 서버 수락 판단 | `src/network/network_server.cpp:360` -- `_network_clients_connected < MAX_CLIENTS`; `static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| 클라이언트 설정 항목 | `src/table/settings/network_settings.ini:231/241` -- `network.max_companies` (def 15, max MAX_COMPANIES), `network.max_clients` (def 25, max MAX_CLIENTS); 구조체는 `src/settings_type.h` `NetworkSettings` |
| **프로토콜 비트폭 (하드 제약)** | `src/network/core/network_game_info.cpp` -- `companies_max`와 `clients_max`가 모두 **`Send_uint8`/`Recv_uint8`** 로 전송 (251-296, 422-432행) |
| 서버 목록 표시 | `src/network/network_gui.cpp:519` 등 |

**현재 상태 결론**:
- **클라이언트 상한 = 255는 이미 프로토콜 상한** (uint8 필드 + 게임 코디네이터/서버 브라우저 호환). 돌파하려면 `network_game_info` 관련 필드를 uint16으로 변경해야 함 (client↔server 전체 링크 + UDP 브로드캐스트 + 게임 코디네이터 프로토콜), 이는 외부 프로토콜 변경에 해당.
- **회사 상한 = 15** (`CompanyIDTag`의 End=0xF). **252**까지 안전하게 상향 가능 (End=0xFC): 기본은 여전히 uint8, 저장 바이트 폭 변경 없음 (이전 저장 호환), `CompanyMask` (uint16→uint32)와 인터페이스/루프 가정을 동시에 완화해야 함.

### 4.3 차량 그룹화

| 관심 지점 | 위치 |
|---|---|
| 그룹 데이터 구조 | `src/group.h` -- `Group : GroupPool::PoolItem` (name/owner/vehicle_type/flags/livery/statistics/parent/number); `GroupID`, `DEFAULT_GROUP`, `IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| 그룹화 명령 | `src/group_cmd.cpp` -- `CmdCreateGroup`(536), `CmdDeleteGroup`(585), `CmdAlterGroup`(646), `CmdAddVehicleGroup`, `CmdAddSharedVehicleGroup` (공유 명령 차량을 기존 그룹에 추가, 718행 근처 `AddVehicleToGroup`) |
| 명령 등록 | `src/group_cmd.h:27-35` -- `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)`; 열거형은 `src/command_type.h` `enum class Commands` (492행부터) |
| 그룹 GUI | `src/group_gui.cpp/.h`, `src/vehiclelist.cpp` |
| 차량↔그룹 | `src/vehicle_base.h` (`Vehicle::group_id`), `SetTrainGroupID/UpdateTrainGroupID` (group.h:130-131) |
| 명령/공유 스케줄 | `src/order_base.h` (`OrderList`, `VehicleOrdersID`), `src/order_cmd.cpp`, `src/order_func.h`, `src/schdispatch.h/.cpp` (scheduled dispatch, order list에 바인딩) |
| 그룹 통계 | `GroupStatistics` (group.h:60-66), `GetGroupNumVehicle` 등 (group.h:125-128) |

**현재 상태 결론**: 이미 `CmdAddSharedVehicleGroup` (차량의 공유 명령 차량을 그룹에 추가)와 `CmdCreateGroupFromList` (목록에서 그룹 생성)가 있지만, **"공유 명령별 자동 그룹 생성/배정"의 전체 로직은 없음**. 새 명령 `AutoGroupSharedOrders` (회사 전체 주요 차량 순회 → `OrderList`별 집계 → 자동 그룹 생성 및 배정) 구현 경로가 명확함.

### 4.4 건설 tooltip (철로 등 건설 시 마우스 위 가격 표시)

| 관심 지점 | 위치 |
|---|---|
| 철로 건설 GUI/로직 | `src/rail_gui.cpp` (`BuildRailToolbarWindow`), `src/rail_cmd.cpp`, `src/rail.h/.cpp`; 도로 `road_gui.cpp/road_cmd.cpp` |
| 비용 추정 | 각 `*_cmd.cpp`의 `DoCommand`가 `CommandCost` 반환; GUI 내에서 `DC_QUERY_COST` 모드로 견적 가능 |
| 마우스 아래 타일 | `src/viewport_func.h:36` -- `GetTileBelowCursor()`; `_cursor.pos` (화면 좌표); `src/viewport.cpp:1056` |
| 기존 텍스트 힌트 메커니즘 | `src/texteff.hpp` -- `AddTextEffect(msg, x, y, duration, mode, ...)` (월드 좌표 플로팅 텍스트), `UpdateTextEffect`; `src/texteff.cpp` |
| 기존 건설 힌트 UX | 철로 도구 모음 `OnPlaceDrag` 드래그 시 선택 영역과 비용 누적 (`_thd` tilehighlight, `src/tilehighlight_func.h`); 상태 표시줄 `statusbar_gui.cpp`에서 도구 비용 표시 가능 |
| 매 프레임 새로고침 지점 | 각 도구 모음 창 `OnMouseLoop` / `viewport.cpp`의 `HandleMouseEvents` (5422/5733행) |

**현재 상태 결론**: "마우스 위 가격 tooltip" 없음. `AddTextEffect`를 사용하여 마우스가 있는 타일에 고정 (커서가 있는 타일 따라가기)하거나 화면 좌표 tooltip을 직접 그리기 가능; 비용은 `DC_QUERY_COST`로 현재 도구에 대해 단일 타일 견적 + 드래그 선택 영역 누적 가능.

### 4.5 AI 인터페이스 (NoAI 유지 + 전판 인식 AI)

| 관심 지점 | 위치 |
|---|---|
| NoAI 프레임워크 | `src/ai/` -- `ai_core.cpp` (AICore 메인 루프), `ai_instance.cpp` (AIInstance/Squirrel VM), `ai_scanner.cpp` (`ai/` 디렉토리 스캔), `ai_gui.cpp` (선택/설정), `ai_config.cpp` |
| GameScript 프레임워크 | `src/game/` -- `game_core.cpp`, `game_instance.cpp` 등 (GS는 "신 모드", 권한이 AI보다 높음) |
| 스크립트 API 클래스 | `src/script/api/script_*.hpp/.cpp` (`script_company`, `script_map`, `script_vehicle`, `script_industry`, `script_town`, `script_game`, `script_admin` 등, 총 60+ 클래스) |
| API 자동 등록 | `src/script/api/CMakeLists.txt` -- `file(GLOB script_*.hpp)`가 자동으로 `ai_*.sq.hpp`/`gs_*.sq.hpp` 바인딩 생성; `.cpp`는 소스 목록에 추가 필요 (235행부터) |
| Squirrel 바인딩 매크로 | `src/script/squirrel_class.hpp` -- `DefSQClass` / `DefSQStaticMethod`; `ai/ai_controller.sq.hpp`는 AI 컨트롤러 바인딩 |
| 인스턴스 스케줄 | `src/script/script_instance.cpp`, `src/script/script_suspend.hpp` (일시 중단/재개), 이벤트 `script_event*` |
| 회사 접근 제어 | `src/script/api/script_object.hpp:318` -- `ScriptObject::GetCompany()`; `ScriptCompanyMode` (`IsDeity()`)로 GS 신 모드 구분; 회사 API 매개변수 검증 `ResolveCompanyID`, `EnforceCompanyModeValid` |
| 스크립트 설정 | `src/table/settings/script_settings.ini` (`game.script.*`); AI 인스턴스 설정 `ai_config.cpp` |

**현재 상태 결론**: 현대 스크립트 API에서 경쟁사의 일부 정보 (예: `GetBankBalance`)는 이미 제한이 없지만, **전판 인식을 위한 집계 API는 없음** ("모든 회사 열거/전역 경제/전역 맵 통계"의 통일된 진입점 없음), 스위치로 제어되는 "전역 AI" 접근 지점도 없음. `ScriptGlobal` API 클래스 (GLOB 자동 등록) + "AI 전역 인식 허용" `game.script` 설정을 추가하면 구현 가능, GS는 항상 사용 가능 (신 모드), AI는 스위치로 제어 -- 즉 "NoAI 유지, 전판 인식 AI 추가, 접근 제어 포함".

---

## 5. 수정 위험 개요

| 기능 | 주요 변경 파일 | 위험 |
|---|---|---|
| F1 다운로드 다중 소스/다중 스레드 | `src/table/settings/network_settings.ini`, `src/settings_type.h`, `src/network/core/config.cpp`, `src/network/network_content.h/.cpp` | 중 (네트워크 콜백 상태 머신 주의 필요) |
| F2 상한 확장 | `src/company_type.h`, `src/table/settings/network_settings.ini`, `src/network/core/network_game_info.cpp` (선택적 uint16) | 낮-중 (회사 252 저위험; 클라이언트 >255는 프로토콜 변경 필요) |
| F3 차량 자동 그룹화 | `src/group_cmd.h/.cpp`, `src/command_type.h`, `src/console_cmds.cpp`, `src/group_gui.cpp`, `src/lang/english.txt` | 낮음 |
| F4 건설 tooltip | 새 `src/construction_cost_tip.h/.cpp`, `src/rail_gui.cpp`/`road_gui.cpp` 훅, `src/lang/english.txt` | 낮-중 |
| F5 전판 인식 AI | 새 `src/script/api/script_global.hpp/.cpp`, `src/script/api/CMakeLists.txt`, `src/table/settings/script_settings.ini`, 예시 AI `bin/ai/GlobalAI/` | 낮음 (프레임워크 자동 등록) |

> 참고: 모든 변경은 `jgrpp` 브랜치 로컬 작업 영역 기반, 컴파일 검증되지 않음 (본 기기에 사용 가능한 빌드 도구 체인 없음); 모든 패치는 `git diff` 검토 후 `git apply` 가능.