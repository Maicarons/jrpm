---
title: 비행기 지상 활주 속도 조절 가능 (Plane Taxi Speed)
---

# 비행기 지상 활주 속도 조절 가능

**embeddedt/OpenTTD-modded** (첫 번째 배치 이식)에서 유래.

## 기능

새 게임 설정 **`vehicle.plane_taxi_speed` (비행기 활주 속도)** 추가, 비행기가 공항 지상에서 활주할 때의 속도 제한을 독립적으로 조절 가능:

- 범위 **1–8**, 기본값 **4** (= 원본 50 단위의 활주 속도 제한)
- 값이 클수록 활주 속도가 빠름 (1 → 12.5, 8 → 100)
- 다른 비행기 속도 설정 (`plane_speed` 글로벌 배율)과 **독립적으로 적용**, 서로 간섭 없음

## 설정 위치

게임 내 **설정 → 전문가 설정 → 차량 (Vehicles)**, 또는 설정 파일 `openttd.cfg`:

```ini
[vehicle]
plane_taxi_speed = 4
```

## 구현 설명

- 설정에 `SettingFlag::NoNetwork` 플래그 (서버 권위, 멀티플레이 시 방장이 결정)
- **XSLF 확장 기능 게이트** 사용 (`XSLFI_PLANE_TAXI_SPEED`, 버전 1): 이전 저장 로드 시 해당 기능이 없으면 설정이 기본값 4로 유지되어 **이전 저장을 전혀 손상시키지 않음**
- 활주 속도 제한은 비행기 상태가 `AS_RUNNING` (지상 활주 중)일 때 적용되며, `min(기종 제한속도, 활주 제한속도)`를 사용

## 관련 파일

- `src/aircraft_cmd.cpp` (새 `SPEED_LIMIT_TAXI` 상수 + 활주 속도 제한 로직)
- `src/table/settings/game_settings.ini` (설정 정의)
- `src/settings_type.h` (`VehicleSettings::plane_taxi_speed`)
- `src/sl/extended_ver_sl.h/.cpp` (XSLF 기능 등록)
- `src/lang/english.txt` (STR_CONFIG_SETTING_PLANE_TAXI_SPEED*)