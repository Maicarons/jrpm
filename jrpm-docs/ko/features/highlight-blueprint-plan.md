---
title: "세 번째 배치 계획: 강조 시스템 + 청사진 시스템"
---

# 세 번째 배치 계획: 강조 시스템 + 청사진 시스템

cmclient의 두 가지 고가치 기능을 심층 연구했으며, 이 페이지에 이식 방안과 작업량 평가를 기록합니다.

## 一、객체 수준 강조 시스템 (cm_highlight, 2888줄)

**기능**: 건설 도구 활성화 시, 건설될 객체를 실시간으로 강조 표시 -- 철로 전체 구간, 역 전체 구역, 도로/주차장, 신호, 교량, 터널, 부두, 공항, 산업 등 **15가지 객체**의 정확한 미리보기.

**이식 의존성 (vanilla API → jrpm API)**:

| cmclient 의존성 | jrpm 대응 |
|---|---|
| `DrawSelectionSprite` / `SetSelectionTilesDirty` / `DrawTileSelectionRect` / `DrawAutorailSelection` (viewport.cpp 내부) | 존재하지만 시그니처가 다름 (jgrpp가 리팩토링함), 하나씩 맞춰야 함 |
| `TileZoning` (도시 구역 색상) | jrpm 없음 → 새로 생성 필요 |
| `_fn_mod` 등 전역 상태 | cmclient 전용 → 재설계 필요 |
| `ObjectTileHighlight::make_rail/road_stop/...` 각 객체 생성자 | NewGRF 역/도로 API에 의존, jrpm과 vanilla에 차이 있음 |

**작업량**: 약 2-3회 (회당 4-6시간), 핵심 난이도는 viewport 렌더링 파이프라인 정렬.

## 二、청사진 시스템 (cm_blueprint, 660줄)

**기능**: 영역 선택 → 건설 명령 시퀀스 기록 (철로/역/터널/교량/신호) → 16개 슬롯 저장 → 원클릭 재생 및 회전.

**이식 의존성**:

| cmclient 의존성 | jrpm 대응 |
|---|---|
| `cm_commands.hpp` **명령 객체 계층** (as_company / with_callback / set_auto / no_estimate + 100+ 생성 명령 클래스, 2251줄) | jrpm에 이 추상화 없음 → **먼저 이식/재작성 필요** |
| `cm_station_gui.hpp`의 `_station_gui` 상태 | jrpm 역 GUI 상태 구조가 다름 |
| `sp<Blueprint>` 스마트 포인터 | jrpm에서 `std::shared_ptr` 사용 가능 |
| `BlueprintCopyArea`의 타일 순회 | `TileIndexDiffC` 등 일반 API에 의존 (jrpm에 있음) |

**작업량**: 약 2-3회 (명령 객체 계층 포함).

## 三、권장 구현 순서

```
1단계: 명령 객체 계층 (cm_command_type 설계, jrpm의 Command<T>::Do/Post로 재구현)
         -- 청사진과 명령 재생 (load_commands)의 공통 기반
2단계: 객체 수준 강조 (viewport 파이프라인 정렬, 객체별 이식)
3단계: 청사진 (복사/회전/슬롯/재생)
4단계 (선택): 명령 재생 (lzma 전체판 재생) + 도시 구역 (growth_tiles, 저장 확장 필요)
```

## 전제 설명

이 세 가지 기능은 합계 약 **5000+줄**이며, 명령 시스템/저장/뷰포트 렌더링 세 가지 핵심 계층을 가로지르고 jrpm에 없는 중간 계층 (명령 객체, 이벤트 버스, 비트스트림 직렬화)에 의존합니다. 독립적인 특별 프로젝트로 추진하는 것이 더 안정적입니다 -- 배치별로 실행하고, 각 배치마다 컴파일 + 멀티플레이 검증 후 커밋하여 한 번에 대규모 변경으로 인한 회귀 찾기 어려움을 방지하는 것이 좋습니다.