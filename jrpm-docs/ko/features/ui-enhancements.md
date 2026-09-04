---
title: 멀티플레이 UI 강화 (위치 북마크 / 화물 상세 / 관전)
---

# 멀티플레이 UI 강화

두 번째 배치로 **citymania-org/cmclient**에서 이식, **콘솔 명령** 형태로 구현 (cmclient의 단축키/도구 모음 인프라에 의존하지 않으며, 안정적이고 스크립트화 가능).

## 위치 북마크 (Viewport Locations)

주 뷰포트 위치와 확대/축소를 저장/복원 (9개 슬롯), 멀티플레이 시 자신의 공장, 역 및 상대방 지역으로 빠르게 이동하기에 적합.

```
savelocation <1-9>    # 현재 뷰포트 위치+확대축소 저장
gotolocation <1-9>    # 저장된 위치로 이동
```

## 회사 화물 상세 (Company Cargo Details)

화물별로 회사의 **배송 완료 운송량 + 수익** 통계 창 표시, **총계 / 지난달** 두 기간 전환 가능.

```
company_cargo <company_id>    # 화물 상세 창 열기
```

- 창에는 각 표준 화물의 운송량, 수익 + 하단 총계 표시 (cmclient와 완전히 동일)
- "Cargo" 열 헤더 클릭으로 총계/지난달 기간 전환
- 화물별 수익은 `CompanyEconomyEntry::cargo_income`으로 추적 (`XSLFI_COMPANY_CARGO_INCOME` 저장 확장), 저장 왕복 무손실

## 관전 지원 (Watch)

관전자가 특정 회사의 건설 지역으로 빠르게 이동 (해당 회사의 마지막 건설 좌표로 이동).

```
watch <company_id>    # 뷰포트가 회사 위치로 이동
```

## 건너뛴 항목과 이유

| cmclient 기능 | jrpm 처리 |
|---|---|
| 플레이어 목록 오버레이 (cm_client_list_gui) | jgrpp **이미** Online Players 창 있음 (`NetworkClientList`), 중복 구현 안 함 |
| 지면 상세 툴팁 (cm_tooltips) | jrpm의 LandInfoWindow **이미** 건물/산업/역 상세 표시 포함 |

## 관련 파일

- `src/jrpm_locations.cpp/.h` (신규: 위치 북마크 + company_cargo + watch 콘솔 명령)
- `src/jrpm_cargo_table.cpp/.h` (신규: 화물 상세 창)
- `src/window_type.h` (새 `WindowClass::CompanyCargos` 추가)
- `src/console_cmds.cpp` (명령 등록)
- `src/lang/english.txt` (STR_JRPM_CARGOS_* 문자열)