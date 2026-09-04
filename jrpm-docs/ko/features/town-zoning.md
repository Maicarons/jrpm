---
title: 도시 구역 (Town Zoning)
---

# 도시 구역 (Town Zoning)

cmclient 도시 구역 이식 (네 번째 배치, 커밋 `9e3f95a2`). jrpm 베이스 (pulsexlb) 자체에 이미 zoning 시스템 (도구 모음 + 렌더링 파이프라인 + 메뉴)이 있으며, 이번 배치에서 그 위에 cmclient 고유의 평가 모드와 **growth_tiles 저장**을 추가했습니다.

## 새 모드 (zoning 도구 모음 드롭다운)

| 모드 | 설명 | 색상 |
|---|---|---|
| **Town zones (Tz)** | 도시 동심원 구역, `squared_town_zone_radius` 재사용 | Tz0 가장자리=연한 파랑 / Tz1=빨강 / Tz2=노랑 / Tz3=초록 / Tz4 도심=흰색 |
| **Town growth tiles** | 이번 달/지난 달 건물 건설 및 철거 궤적 | 새 건물=초록 / 철거=연한 파랑 / 재건=흰색 / 성장 건너뜀=주황 / 건설 건너뜀=노랑 / 서버 철거=빨강 |

## growth_tiles 데이터 계층 (`cm_town_growth.cpp/.h`)

- 두 개의 롤링 월 맵: `TileIndex → TownGrowthTileState` (현재 월 / 지난 월)
- 이벤트 훅:
  - `BuildTownHouse` → `NEW_HOUSE` (지난 월이 철거면 `RH_REBUILT`로 업그레이드)
  - `ClearTownHouse` → `RH_REMOVED`
  - `TownsMonthlyLoop` → 월간 순환 (지난 월=현재 월, 현재 월 초기화)
- **저장 영속화**: 새로운 `GRWT` savegame 청크 (`misc_sl.cpp`), `{tile, state}` 쌍 목록으로 직렬화; 이전 저장에는 이 청크가 없으므로 로드 시 완전 호환 (저장→읽기 왕복 검증 완료).

## 열기 방법

도구 모음 메뉴 → 맵 (Zoning) → zoning 도구 모음 열기, 안쪽/바깥쪽 드롭다운에서 각각 평가 모드 선택.

## 생략 설명

cmclient의 CityBuilder 서버 전용 모드 (CB 수용 구역 / CB 도시 상한) 및 `ext::Town` 확장 필드 (광고, 자금, 화물 통계)는 서버 플레이 전용이므로 이식하지 않았습니다.