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