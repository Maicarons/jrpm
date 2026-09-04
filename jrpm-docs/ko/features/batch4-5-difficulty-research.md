# 네 번째 배치 / 다섯 번째 배치 / CM 서버 명령 -- 이식 난이도 조사 보고서

> 조사 대상: citymania-org/cmclient (vanilla 15.3 브랜치) 소스 코드 정독
> 조사 일자: 2026-08-14
> 결론: 세 작업의 난이도 차이가 크며, 항목별 평가와 권장 순서를 제시합니다.

---

## 一、네 번째 배치: 도시 구역 (growth_tiles 저장 포함)

### 기능 구성

| 구성 요소 | 파일 | 규모 | 설명 |
|---|---|---|---|
| 구역 모드 열거형 | cm_zoning.hpp | 37줄 | 12가지 평가 모드 (CHECKOPINION/CHECKBUILD/CHECKSTACATCH/CHECKACTIVESTATIONS/CHECKBULUNSER/CHECKINDUNSER/CHECKTOWNZONES/CHECKCBACCEPTANCE/CHECKCBTOWNLIMIT/CHECKTOWNADZONES/CHECKTOWNGROWTHTILES) |
| 평가 로직 | cm_zoning_cmd.cpp | 413줄 | 각 모드별 하나의 쿼리 함수 (팔레트 SpriteID 반환), TownZone/Tz 반경 알고리즘, StationFinder 커버리지 검사, 미서비스 건물/산업 검사 포함 |
| 구역 도구 모음 GUI | cm_zoning_gui.cpp | 204줄 | 내/외부 두 계층 구역 모드 선택 창 (`_zoning.inner/outer` 전역 상태) |
| growth_tiles 저장 | cm_saveload.cpp/.hpp | 90줄 | Town 확장 필드 `growth_tiles` + `growth_tiles_last_month` (`std::map<TileIndex, uint8_t>`), vanilla SaveLoad handler로 저장 |
| 데이터 확장 | extensions/cmext_town.hpp | ~80줄 | `ext::Town`: growth_tiles ×2 + **대량 CM 서버 플레이 필드** (CBTownInfo 화물 통계, 광고/자금 추적, hs/cs/hr 도시 성장 카운트) |
| 월간 순환 + 트리거 | cm_game.cpp / town_cmd.cpp | -- | NewMonth 순환 growth_tiles; 건물 건설/철거/재건 시 상태 기록 |

### 이식 난이도 항목별 평가

| 난이도 | 심각도 | 설명 |
|---|---|---|
| **저장 시스템 재작성** | 🟠 중 | growth_tiles는 vanilla `DefaultSaveLoadHandler` + `SlSetStructListLength`/`SlObject` 사용, jrpm은 `sl/` 새 시스템 (SlTableHeader/SlObjectSaveFiltered). jrpm의 NSL/SLE + **XSLF 기능 게이트**로 재작성 필요 (새 `XSLFI_TOWN_GROWTH_TILES`, 버전 1), 이전 저장 영향 없음 |
| **Town 구조 확장** | 🟢 낮 | growth_tiles 두 개의 map만 가져옴 (**CBTownInfo/광고/자금 필드 건너뜀** -- 그것들은 CM 서버 CityBuilder 플레이 전용); jrpm Town에 필드 추가 + town_sl.cpp 저장 테이블 마운트 |
| **평가 로직** | 🟢 낮-중 | 대부분 순수 쿼리 (GetTileType/StationFinder/Town 캐시/건물 검사), jrpm API 존재; TownZone 반경 알고리즘 (`squared_town_zone_radius`)은 jrpm 필드명 확인 필요 |
| **렌더링 파이프라인** | 🟠 중-고 | `DrawTileZoning`을 viewport 렌더링 파이프라인에 연결 필요 -- **세 번째 배치 강조와 TileHighlight 렌더링 메커니즘 공유**. 강조 파이프라인이 없으면 별도로 구축해야 함 (**강조 먼저, 구역 나중에 권장**) |
| **스프라이트 리소스** | 🟠 중 | 12가지 팔레트가 `CM_SPR_PALETTE_ZONING_*` **사용자 정의 스프라이트** 사용, jrpm에 없음 → 기존 팔레트 스프라이트로 대체하거나 새 리소스 추가 필요 |
| **이벤트 훅** | 🟠 중 | growth_tiles 기록은 cmclient의 **이벤트 버스**에 의존 (event::HouseBuilt/HouseCleared/..., cm_main.cpp의 Emit). jrpm에 이 메커니즘 없음 → town_cmd.cpp의 건물 건설/철거 부분에 직접 훅 추가 + NewMonth 순환 (IntervalTimer) |

### 난이도 결론: 🟠 중고 (약 1.5-2회 특별 작업, 회당 4-6시간)

- **전제**: 세 번째 배치 **강조 시스템** 완료 후 수행을 강력 권장 (렌더링 파이프라인 공유)
- "12가지 구역 색상, growth_tiles 저장 제외"만 할 경우: 난이도 🟡 중으로 하락 (저장 확장 생략, 약 1회)

---

## 二、다섯 번째 배치: 명령 재생 + 내보내기/녹화

### 2.1 명령 재생 (cm_command_log + cm_commands + generated)

| 구성 요소 | 규모 | 설명 |
|---|---|---|
| 명령 객체 계층 | cm_command_type.hpp + generated/cm_gen_commands (2251+1418줄) | 각 vanilla 명령을 프로그래밍 가능한 객체로 래핑 (as_company/with_callback/set_auto), **명령→비트스트림 직렬화** 포함 |
| 명령 로그 로드 | cm_command_log.cpp (203줄) | lzma 압축 해제 + BitOStream 비트스트림 파싱 → `_fake_commands` 큐 (tick counter/예상 결과/랜덤 시드/CommandPacket) |
| 명령 실행 | ExecuteFakeCommands | tick counter 순서로 실행: `ExecuteCommand(&cp)` (vanilla 내부 API) + **랜덤 시드/결과 검증** (안티치트), 멀티플레이 시 모든 클라이언트에 전달 |

**주요 차이 (난이도 결정)**:

| cmclient | jrpm | 영향 |
|---|---|---|
| `ExecuteCommand(CommandPacket*)` | 이 함수 없음, 명령 핵심은 `DoCommandPInternal(Commands, TileIndex, CommandPayloadBase&, ...)` | CommandPacket → DoCommandPInternal의 payload 변환 계층 작성 필요 🟠 |
| CommandPacket 필드 (vanilla) | CommandPacket은 존재하지만 구조가 다름 (GeneralCommandPacket\<DynBaseCommandContainer\>) | 필드 매핑 적응 필요 🟠 |
| `GetCommandName` | ✅ 존재 (command_func.h:166) | 🟢 |
| `outgoing_queue` (멀티플레이 전달) | jrpm 네트워크 계층이 다름 (OutgoingCommandPacket/ServerNetworkGameSocketHandler::SendCommand) | 멀티플레이 재생 시 전달 로직 재작성 필요 🟠 |
| 레코더 (어떻게 .cmd 파일 생성) | **cmclient의 기록 훅은 명령 객체 계층의 post() 가로채기에 의존** | jrpm 명령은 템플릿화된 Post → 명령 분배 지점에 기록 훅 추가 필요 🔴 핵심 작업 |
| 파일 형식 | 비공개 (vanilla 명령 ID + 비트스트림) | jrpm 명령 ID가 vanilla와 완전히 다름 → 재생 파일 호환 불가, **형식 재설계 필요** 🟠 |

### 2.2 내보내기 / 녹화 (cm_export.cpp, 536줄)

| 기능 | 설명 | 난이도 |
|---|---|---|
| ExportOpenttdData | JSON으로 건물 사양/화물 사양/팔레트/엔진 정보 내보내기 (JsonWriter) | 🟢 낮-중 (독립적, Spec 구조 읽고 JSON 쓰기) |
| ViewportExport / ExportFrameSprites | 프레임별 뷰포트 스프라이트 내보내기 (viewport 내부 렌더링 벡터 TileSpriteToDrawVector/ParentSpriteToSortVector에 의존) | 🟠 중-고 (jgrpp viewport 파이프라인이 다르므로 벡터 인터페이스 정렬 필요) |

### 난이도 결론: 🟠 중고 (약 2회 특별 작업)

- **명령 재생이 가장 큰 부분**: 핵심 작업 = ①명령 분배 지점에 **기록 훅** 추가 (jrpm 템플릿화된 명령 시스템 적응) ②CommandPacket → DoCommandPInternal 실행 계층 ③새 파일 형식 설계. **명령 객체 계층 먼저 이식 권장** (세 번째 배치 청사진의 기반이자 재생의 기반)
- 내보내기: JSON 데이터 내보내기는 독립적으로 먼저 수행 가능 (낮-중); 프레임 녹화는 viewport 정렬에 의존 (강조와 동일 배치 처리)

---

## 三、CM 서버 명령 (cm_console_cmds.cpp, 289줄)

### 명령 목록 및 항목별 난이도

| 명령 | 기능 | jrpm 난이도 | 비고 |
|---|---|---|---|
| `cmgamespeed [n]` | 게임 속도 변경 | 🟢 **매우 낮음** (~20줄) | jrpm에 이미 `_game_speed` 전역 (gfx.cpp:52), 명령 셸만 부족 |
| `cmstep [n]` | n tick 스텝 | ⏭️ **jrpm에 이미 있음** | `step` 명령 (ConStepGame) 기능 동일, 건너뜀 |
| `cmexport` | openttd.json 내보내기 | 🟢 낮-중 | ExportOpenttdData에 의존 (다섯 번째 배치 참조) |
| `cmtreemap <file>` | 높이 맵에 나무 심기 | 🟡 중 | 높이 맵 읽기 + 나무 심기 명령, 독립적 |
| `cmreset_town_growth` | 도시 성장 기록 초기화 | 🟢 낮 | 도시 구역 필드에 의존 |
| `cmload_commands` | 명령 재생 로드 | 🟠 중고 | 명령 재생 인프라에 의존 |
| `cmstart_record` / `cmstop_record` | 프레임 녹화 | 🟠 중고 | 녹화 인프라에 의존 |
| `cmgamestats` | 게임 세션 통계 | 🟢 낮 | 독립적 |
| `cmgfxdebug` | 그래픽 디버그 | 🟢 낮 | 독립적 |

### 난이도 결론: 🟢 전체적으로 낮음 (단일 명령 0.5-2시간)

**권장 순서**: `cmgamespeed` (10분) → `cmgamestats`/`cmgfxdebug` (각 ~1h) → `cmexport` (다섯 번째 배치 내보내기와 함께) → `cmtreemap` (~2h) → `cmreset_town_growth` (네 번째 배치와 함께) → `cmload_commands`/`cmstart_record` (다섯 번째 배치 재생/녹화와 함께).

---

## 四、전체 권장 로드맵

```
① 강조 시스템 (세 번째 배치 핵심, ~2-3회)      ← 청사진/구역의 렌더링 기반
② 명령 객체 계층 (세 번째 배치 청사진 + 다섯 번째 배치 재생의 기반, ~1-2회)
③ 청사진 (~1회)
④ 도시 구역 (~1.5회, ① 렌더링 파이프라인에 의존; growth_tiles는 XSLF 게이트 사용)
⑤ 명령 재생 + 내보내기 (~2회, ②에 의존; JSON 내보내기는 미리 독립적으로 수행 가능)
⑥ CM 서버 명령 (각 배치 사이에 분산 삽입, gamespeed/step은 즉시 수행 가능)
```

## 五、즉시 저비용으로 구현 가능한 항목 (대규모 작업에 의존하지 않음)

1. `cmgamespeed` 명령 (~20줄, `_game_speed` 변경)
2. `step` 이미 있음 → 불필요
3. `cmgamestats` / `cmgfxdebug` (각 ~1h, 독립적)
4. `cmexport`의 JSON 데이터 내보내기 (~2h, 독립적)
5. `cmtreemap` (~2h, 독립적)

이 5개 항목은 약 **반나절 작업량**으로, "영번째 배치"로 먼저 구현할 수 있으며 三/四/五 배치와 병행해도 충돌하지 않습니다.