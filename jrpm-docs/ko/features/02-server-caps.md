---
title: 서버 상한 연구
---

## 조사 결론: 두 상한 모두 구조적

### 온라인 플레이어 상한 = 255 (프로토콜 비트폭 하드 제약)

| 위치 | 내용 |
|---|---|
| `src/network/network_type.h:21` | `static const uint MAX_CLIENTS = 255;` |
| `src/network/network_type.h:54` | `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>` |
| `src/network/core/network_game_info.cpp` | `clients_max` / `companies_max`가 **`Send_uint8` / `Recv_uint8`** 로 `SerializeNetworkGameInfo` (251-296행)와 역직렬화 (422-432행)에서 전송 |
| `src/table/settings/network_settings.ini:241` | `network.max_clients` (SLE_UINT8, max = MAX_CLIENTS, 기본 25) |
| `src/network/network_server.cpp:360` | 수락 판단 `_network_clients_connected < MAX_CLIENTS` |

**결론**: 255는 이미 uint8 프로토콜 필드의 상한입니다. 돌파하려면 `clients_max` 등 관련 필드를 uint16으로 변경해야 합니다 -- 이는 client↔server 온라인 프로토콜, UDP 브로드캐스트 및 **게임 코디네이터 (Game Coordinator)** 프로토콜을 변경하는 것으로, 외부 프로토콜 변경에 해당합니다 (fork 내에서 양쪽이 함께 업그레이드하면 플레이 가능하지만, 공용 서버 목록 호환성에 영향).

### 회사 수 상한 = 15 (타일 owner 저장 형식 하드 제약)

| 위치 | 내용 |
|---|---|
| `src/company_type.h:25` | `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15` |
| `src/tile_map.h:195` | `SetTileOwner`: `SB(_m[tile].m1, 0, 5, owner.base())` -- **타일 owner는 5비트만 저장** (`_m[].m1`의 하위 5비트) |
| `src/company_type.h:30-33` | `OWNER_TOWN{0x0F}`, `OWNER_NONE{0x10}`, `OWNER_WATER{0x11}`, `OWNER_DEITY{0x12}`, `OWNER_END{0x13}`가 회사 ID와 동일한 바이트 공간 공유 |

**결론**: `Owner`와 `CompanyID`는 동일한 유형이며, 타일 소유권 필드는 5비트 (값 0-31)뿐이며, 그중 15-18은 특수 owner가 점유합니다. 따라서 실제 회사 상한 15는 **맵 배열 형식**에 의해 결정됩니다 -- 돌파하려면 타일 owner 저장을 확장 (`m1` 전체 8비트 또는 `_m` 구조 변경)하고 OWNER_* 상수를 마이그레이션해야 하며, **전체 저장 변환**과 맵 메모리 증가가 필요하므로 대규모 형식 리팩토링에 해당합니다 (OpenTTD 상위에서 여러 해 동안 하지 않은 이유).

## 선택 가능한 방안

| 방안 | 변경 | 호환성 | 제안 |
|---|---|---|---|
| A. 현재 상태 유지 | 없음 | 저장/프로토콜/코디네이터 완전 호환 | ✅ 권장: 255 클라이언트 / 15 회사는 멀티플레이에 이미 원본을 훨씬 초과 |
| B. 클라이언트 → uint16 프로토콜 | `network_game_info.cpp` 직렬화/역직렬화를 `Send_uint16/Recv_uint16`으로 변경; `MAX_CLIENTS`를 4095로 상향; `network_settings.ini`의 `max_clients`를 SLE_UINT16으로 변경; `settings_type.h` 필드를 uint16으로 변경; `console_cmds.cpp:1056` 표시 너비 | fork 내 양쪽 플레이 가능; 공용 코디네이터/구버전 클라이언트와 호환 불가 | 선택 가능, 실제로 >255 클라이언트가 필요하다면 |
| C. 회사 → 타일 형식 리팩토링 | 타일 owner 필드 확장 + OWNER_* 상수 마이그레이션 + 전체 저장 변환 | 저장 형식 변경 (일회성 변환) | 단기 구현 비권장 |

## 이미 제공된 내용

- 본 설계 문서 (정확한 파일/줄 번호 포함);
- 선택적 패치 `option-clients-uint16.diff` (방안 B의 전체 변경 사항, **미적용**, `git apply` 검토용).

> 결론 우선: **두 상한의 "확장"은 본질적으로 형식/프로토콜 업그레이드이지 설정 항목 조정이 아닙니다**; 저장 및 공용 네트워크 호환성을 유지하는 한, 현재 255/15가 상한입니다. 호환성 비용을 수용한다면 방안 B/C의 변경 방식은 위와 같습니다.