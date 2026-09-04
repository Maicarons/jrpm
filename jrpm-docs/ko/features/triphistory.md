---
title: 차량 운행 이력 (Trip History)
---

# 차량 운행 이력 (Trip History)

**embeddedt/OpenTTD-modded** (첫 번째 배치 이식)에서 유래, jrpm의 새로운 문자열 및 날짜 API에 맞게 조정됨.

## 기능

각 차량은 최근 **10회 운행**의 데이터를 기억하며, 차량 상세 창에 새로 추가된 **History (기록)** 버튼에서 확인 가능:

| 열 | 의미 |
|---|---|
| Received (도착) | 이번 운행에서 화물이 도착한 날짜 |
| Profit (수익) | 이번 운행 수익 (음수는 노란색 표시) |
| % Change | 이전 운행 대비 수익 변화율 (녹색 + / 빨간색 -) |
| TBT | 이전 운행과의 시간 간격 (일) |
| Change | 이전 운행 대비 시간 변화 (일) |
| Occupancy | 이번 운행 평균 승객/화물 적재율 (%) |

창 하단에는 요약 통계도 표시:

- **Total income for the last N trips**: N회 총 수익 + 회당 일일 평균 수익
- **Average trip length**: 평균 운행 간격 (일)
- **Improvement over last N trips**: 수익 종합 변화율

## 기록 메커니즘

- **AddValue**: 차량이 한 번의 운송을 완료 (`CargoPayment` 소멸)할 때 수익, 날짜, 점유율, 역 간 거리 기록
- **NewRound**: 차량이 시간표에 따라 첫 번째 목적지에 도착하면 새 운행 시작
- **점유율**: 차량이 역을 떠날 때 수집 (jrpm의 기존 `trip_occupancy` 메커니즘 재사용)

## 구현 설명

- 데이터는 **NOSAVE** (런타임 전용, 저장에 기록 안 함)이므로 저장 버전을 올릴 필요 없음, 이전 저장과 완전 호환
- 새 창 클래스 `WindowClass::VehicleTripHistory`, 창 ID는 차량 ID
- 차량 상세 창 (기차/비기차) 제목 표시줄에 History 버튼 추가
- 차량 삭제 시 기록 창이 자동으로 닫힘

## 관련 파일

- `src/triphistory.h` / `src/triphistory_cmd.cpp` / `src/triphistory_gui.cpp` (신규)
- `src/vehicle_base.h` (Vehicle에 `trip_history` 필드 추가)
- `src/economy.cpp` (CargoPayment 소멸 시 운행 기록)
- `src/timetable_cmd.cpp` (첫 번째 역 도착 시 새 운행 시작)
- `src/vehicle_gui.cpp` / `src/widgets/vehicle_widget.h` (History 버튼)
- `src/lang/english.txt` (STR_TRIP_HISTORY_* 문자열)