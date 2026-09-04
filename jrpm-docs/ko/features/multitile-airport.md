---
title: 모듈식 공항 (multitile-airport)
---

# 모듈식 공항 (multitile-airport)

> 출처: pulsexlb/OpenTTD-patches의 `jgrpp-multitile-airport` 기능 브랜치, git merge를 통해 jrpm에 통합.

## 기능 소개

다중 타일 (multitile) 모듈식 공항 재구성: 공항을 "고정된 공항 유형"에서 **자유롭게 레이아웃을 개조할 수 있는** 다중 타일 시스템으로 변경:

- **다중 타일 공항**: 공항은 여러 기능 타일 (활주로, 유도로, 계류장, 터미널, 헬리패드)로 구성되며, 임의의 레이아웃으로 조합 가능;
- **공항 레이아웃 개조**: `station.allow_modify_airports`를 활성화하면 기존 공항에 타일을 추가/삭제/조정 가능;
- **air 유형 체계**: 새로운 `air.h`/`air_type.h`/`newgrf_airtype.*` -- 항공 유형 (고정익/헬리콥터 등)을 확장 가능한 air type 체계로 추상화, NewGRF가 새 항공 유형과 스프라이트를 정의할 수 있음;
- **PBS 항공 관제**: `pbs_air.*` -- 활주로/유도로 점유 및 신호 예약의 항공 버전, 다중 항공기 동시 활주 지원;
- **YAPF 항공 경로 탐색**: 비행기가 지상 (활주/대기)과 공중에서의 경로 계획을 YAPF 체계로 수행.

## 핵심 능력

| 능력 | 설명 |
|---|---|
| 공항 레이아웃 개조 | `station.allow_modify_airports` (**기본 활성화**; 활성화 후 기존 공항 개조 가능) |
| 기본 항공 유형 | `gui.default_air_type` |
| 다중 타일 공항 스프라이트 | 재구성된 openttd.grf (air type 스프라이트), 투명 스프라이트 수정 |
| 비행기 동작 | 활주로 점유, 활주 방향 전환, 이륙/착륙 대기, 헬리패드, 우주선 렌더링 |
| NewGRF 호환 | airtype 스프라이트 로드, 공항 NewGRF 콜백 |
| 저장 | `SLV_MULTITILE_AIRPORTS` 저장 버전 |

## 사용 방법

1. 게임 설정에서 `station.allow_modify_airports` 활성화;
2. 공항 건설 후, 공항 개조 도구를 사용하여 활주로/계류장/터미널 레이아웃 조정;
3. `gui.default_air_type`에서 기본 항공 유형 선택;
4. 항공 NewGRF와 함께 사용자 정의 air type 사용.

## 관련 코드

- 항공 유형: `src/air.h`, `src/air_type.h`, `src/newgrf_airtype.*`
- 항공 관제: `src/pbs_air.*`
- 비행기/공항 명령: `src/aircraft_cmd.cpp` (3600줄 리팩토링), `src/airport_cmd.cpp`, `src/airport_gui.cpp`
- 경로 탐색: `src/pathfinder/yapf` (항공 부분)
- 저장: `src/sl/saveload_common.h` (`SLV_MULTITILE_AIRPORTS`)

## 주의

- 이 기능은 항공 시스템의 대규모 리팩토링 (aircraft_cmd.cpp 3600+줄 리팩토링)이므로, **실제 기계에서 컴파일 후 다음 시나리오를 중점적으로 회귀 테스트**해야 함: 비행기 구매/이륙/착륙, 활주로 점유, 공항 GUI, 저장 로드;
- 병합 시 작업 영역에서 참조되지 않는 이전 유형 (`VehicleAirFlags`, `AirportMovingDataFlag`)을 삭제했으며, 다른 파일에서 참조되지 않음을 확인함;
- 저장에 있는 기존 공항을 개조해야 하는 경우 먼저 저장을 백업하세요.