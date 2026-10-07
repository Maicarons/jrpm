---
title: jrpm 버전 병합 개요
---

> 브랜치: `jrpm` ｜ 버전: jrpm-0.1.0 (태그됨, 2026-08-14)
> 빌드 산출물: `openttd-jrpm` (실행 파일명)

## 버전 관계

```
                        jgrpp-0.73.1 (공통 조상)
                        /                 \
        jgrpp 브랜치(63 커밋)          pulsexlb px-patch (152 커밋)
        ├ tracerestrict 등 최근 업데이트      ├ jgrpp-decouple (기관차 분리)
        ├ 내 5 기능 (d4c45740)                └ jgrpp-multitile-airport (모듈식 공항)
        └ 버전명 변경 jrpm-0.1.0 (425e7207)
                        \                 /
                        jrpm 브랜치 (merge 71fe214c + 호환 cb9848b)
```

## 병합 내용

### 1. pulsexlb/OpenTTD-patches (px-patch 전체 152 커밋) → 병합 완료

| 기능 | 설명 | 주요 파일 |
|---|---|---|
| **기관차 분리 (decouple)** | 열차 분리/결합: 분리 명령, 노선표 이전, 결합 길이/속도 제한, 이중 기관차, NewGRF 결합, 결합 경로 탐색 (YAPF/NPF), 분리 후 두 열차 독립 운행 | train_cmd.cpp, order_cmd.cpp, order_gui.cpp, train.h, yapf/npf |
| **모듈식 공항 (multitile-airport)** | 다중 타일 공항 시스템 재구성: air 유형 체계 (air.h/air_type.h/newgrf_airtype.*), PBS 항공 관제 (pbs_air.*), YAPF 항공 경로 탐색, `station.allow_modify_airports` (공항 레이아웃 개조), `gui.default_air_type`, 다중 타일 공항 스프라이트 | air.*, pbs_air.*, aircraft_cmd.cpp (3600줄 리팩토링), airport_cmd/gui, station_cmd |

충돌 처리: 헤더 파일 2개만 충돌 (aircraft.h / airport.h) -- pulsexlb 항공 리팩토링이 작업 영역에서 **참조되지 않는** 죽은 유형 (`VehicleAirFlags` bitset, `AirportMovingDataFlag`)을 삭제, pulsexlb 측 삭제를 채택, 다른 파일 참조 없음 확인 완료.

### 2. Openttd-Cluster (사용자 Rust 클러스터 프로젝트) → 선택적 차용

| 패치 | 처리 | 설명 |
|---|---|---|
| 0006 vanilla-native-server (다중 버전 클라이언트 호환) | ✅ **병합 완료** (cb9848b7) | jrpm 서버가 jrpm / 원본 jgrpp (`jgrpp-`) / pulsexlb (`pxp`) 클라이언트를 동시에 수용; NewGRF 버전은 여전히 엄격 검증 |
| 0001 revision-handshake / 0005 version-metadata | ✅ 아이디어 채택 | jrpm이 독립적인 태그된 수정 문자열 `jrpm-0.1.0` 사용, 멀티플레이 핸드셰이크가 jgrpp/pxp와 격리되어 "새 버전으로 멀티플레이 편리" 구현 |
| 0007 parallel-download (HTTP 스레드 풀 + Range 분할, 30KB) | 📝 참조, 병합 안 함 | 본 프로젝트 F1 "파일 수준 병렬 + 다중 미러"와 동일 주제 및 동일 파일 변경; 0007의 **전송 계층 스레드 풀/분할 다운로드**를 F1 추후 강화 방향으로 기록 |
| 0002-0004 snapshot/command/FFI 브리지 | 📝 아키텍처 참조 | 전체 otc-engine Rust 런타임에 의존 (FFI 정적 링크), "장기 전체 통합"에 해당하며 패치 수준 병합이 아님; jrpm은 현재 순수 C++ 단일 바이너리 유지 |

### 3. jrpm 전용 (이전 5 기능, d4c45740) → 이미 jrpm 브랜치 내

병렬 다운로드 (다중 미러+4 동시 세션), 자동 그룹화, 건설 tooltip, 전판 인식 AI (ScriptGlobal + GlobalAI), 미러/콘텐츠 서버 설정.

## 멀티플레이 전략 ("새 버전으로 멀티플레이 편리")

- jrpm은 **태그된 버전**: `IsNetworkCompatibleVersion`가 수정 문자열의 완전 일치 요구 → **jrpm 클라이언트는 jrpm 서버와만 멀티플레이**, jgrpp 0.73.x / pxp와 완전 격리;
- **서버 완화**: jrpm 서버가 추가로 `jgrpp-*` 및 `pxp*` 클라이언트의 가입을 허용 (`IsJgrppNativeNetworkRevision` / `IsPxpNetworkRevision`, NewGRF 버전은 일치해야 함);
- 따라서: jrpm 서버 주인이 서버 열기 = jrpm 플레이어만 접속 (기본값); 기존 클라이언트와 호환 필요 시 설정 변경 없이 jgrpp/pxp 플레이어 접속 가능.

## 빌드 및 검증 (사용자 본 기기에서 실행)

```bash
# 처음 (CMake + 의존성 필요, COMPILING.md 참조)
cmake -B build ..
cmake --build build -j
# 산출물: build/openttd-jrpm.exe
```

검증 우선순위:
1. `openttd-jrpm -v`가 `jrpm-0.1.0` 표시;
2. 싱글 플레이로 1-2년 실행 (병합에 train/airport 대규모 변경 + allow_modify_airports 등 추가로 저장 버전이 올라갈 수 있음);
3. 서버 개설 후: jrpm 클라이언트 가입 ✓; 원본 jgrpp 0.73.x 클라이언트 가입 시도 (진입 가능 예상, NewGRF 일치 시);
4. 기관차 분리: 열차에 분리/결합 명령 추가, 분리 후 두 열차 독립 운행 검증; 모듈식 공항: `station.allow_modify_airports` 활성화 후 공항 레이아웃 개조;
5. 이전 5 기능 회귀 (병렬 다운로드, 자동 그룹화, 건설 tooltip, GlobalAI).

## 알려진 위험

- **컴파일 검증 안 됨**: 병합+개조 코드가 본 기기에서 컴파일되지 않음 (도구 체인 없음), 실제 기계 첫 번째 컴파일에서 누락된 인터페이스 변경이 있을 수 있음 (특히 aircraft/airport/train 3계열 대규모 변경);
- 저장 버전: px-patch가 SLV를 올릴 수 있음 (M9에서 savegame version gate 언급), jrpm 저장과 jgrpp 0.73.x 저장이 상호 읽기 불가능할 수 있음 (jgrpp 관례와 동일, trunk 저장과 하위 호환);
- merge로 pulsexlb 전체 역사가 포함됨, 기능 귀속 추적이 필요하면 `git log --oneline pulsexlb/px-patch` 사용.

---

## 병합 로그: 2026-09-28 (jgrpp-0.73.3 + px-patch 2609.x)

> 병합 커밋: jgrpp 94개 (`jgrpp-0.73.3`까지) + px-patch 113개 (`pxp-2609.10` 이후까지).

**신규:** pulsexlb에서 — 차량 운송 (RoRo, 기능 문서 참조), 128 화물 종류 (Uint128 기반 CargoTypes, XSLFI_CARGO_TYPES_128 게이트), 분리/연결 수정 배치와 일정 개선; jgrpp에서 — 주문 드래그 앤 드롭/더블클릭, 양방향 선박 (XSLFI_DOUBLE_ENDED_SHIPS), 그리고 다수의 일반 수정.

**주요 충돌 처리:** pulsexlb의 세이브 계층은 구형 SLE_ 매크로 시스템 기반이고 jrpm/jgrpp 0.73.3은 현대적 VarFileType/VarMemType + VarTypes를 사용 — saveload/ 및 sl/의 모든 충돌을 현대 시스템으로 재작성하고 U128 지원을 추가했습니다 (VarFileType::U128=13, VarMemType::U128, SLE_UINT128); jrpm이 예약한 367/368은 유지되고 업스트림의 신규 버전은 369/370으로 이동; 업스트림 세이브 호환은 XSLFI_UPSTREAM_VERSION 서브 청크로 처리; CT_VEHICLES 문자열 생성, grfid Label 비교, autogroup + RORO_DEBUG_COMMANDS 콘솔 블록 모두 보존.

**상태:** MinGW ninja 빌드 통과, 산출물 `build/openttd-jrpm.exe`; 신규 게임 스모크 테스트 통과.

---

## 병합 기록: 2026-10-07 (jgrpp 0.73.3+89 + px-patch 2610.3)

> 병합 커밋: jgrpp 23개 커밋 (`jgrpp-0.73.3` 이후, `6318727b02`까지) + px-patch 55개 커밋 (`pxp-2610.3`, `4a4d0724b5`까지).

**신규:** pulsexlb에서 — 기차 페리(선박을 열차 전체 수송용으로 개조, 전용 RAIL 화물 CT_RAILVEHICLES, 객차별 적재), 비 시스템(weather.cpp: 무작위 강우 기간, 점진적 세계 어두워짐, 줌에 맞춰 확장되는 빗방울 오버레이; difficulty.rain 옵션; 샌드박스 치트로 강제 날씨; 날씨 상태는 WTHR/XSLFI_WEATHER로 저장), 분리 후 동일 방향으로 대기하며 출발, 선창별/객차별 적재량 표시, RoRo 옵션에서 열차·자동차 표지판 선택 가능, 결합 경로 탐색 지연 제거(화물 검증을 사전 검사로 바꾸고 실패 시 백오프), Android 빌드 및 APK 배포; jgrpp에서 — StringID 강한 타입화, 중복 회사명 거부, 위젯 기본 크기 계산과 그 밖의 리팩터링·수정.

**주요 충돌 처리:** 빌드의 최대 장애물은 Label/StringID 강한 타입화였습니다 — jrpm 자체 코드의 4문자 리터럴 라벨을 모두 문자열 생성으로 전환했습니다(CT_RAILVEHICLES{"RAIL"}). afterload.cpp의 도로·전차 종류 라벨 비교는 RoadTypeLabel{"ROAD"} 형식이 되었고, airport.cpp의 빈 AirTypeInfo 문자열 필드는 STR_NULL을 사용하며, cheat_gui.cpp의 STR_CHEAT_RAIN switch 분기는 .base()를 씁니다. 새 날씨 청크 WTHR의 구식 청크 타입 CH_TABLE은 ChunkType::Table로 교체했습니다. 치트 표에는 jrpm의 VarMemType와 InflationCheat 센티널을 유지한 채 비 치트 행을 추가했고, jrpm_watch_gui.cpp의 SetStringTip(SPR_GOTO_LOCATION, …)은 SetSpriteTip으로 바꿨습니다. train_cmd.cpp는 enable_decouple 게이트와 desync 디버그 출력을 유지하면서 업스트림의 "열차 앞부분이 분기점을 지날 때만 후진" 수정(if (v->IsMovingFront()))과 동일 방향 출발 플래그를 채택했습니다. order_cmd.cpp는 DecouplePart 변수를 유지한 채 DrivingBackwards를 TCF_NO_DRIVING_CAB 기반 새 의미로 바꿨습니다. deploy-docs.yml은 VitePress 빌드와 GitHub Pages 배포를 유지했고, README/.gitignore/.ottdrev-vc는 jrpm 브랜딩을 보존했습니다. SL_UPSTREAM_VERSION은 여전히 368(업스트림 DoubleEndedShips)에 고정되어 있고 src/saveload/engine_sl.cpp의 static_assert가 통과하며, jrpm 예약 367/368은 그대로입니다.

**상태:** MinGW ninja 빌드 통과(-j3), 산출물 `build/openttd-jrpm.exe`; `-D` 전용 서버 스모크 테스트 통과 — 새 지도 생성 → 저장 → 불러오기 → 다시 저장 → 종료까지 어서션이나 SetupEngines 크래시 없이 완료되었고 업스트림 청크 버전 게이트가 정상 동작합니다.
