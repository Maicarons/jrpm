---
title: 버전 역사
---

# 버전 역사

## jrpm-0.1.0 (2026-08-14) cmclient 이식 완료

첫 번째 0.1.0 기반으로 cmclient 5개 배치 기능 이식 완료 (modded + cmclient 전 계열):

### 새로운 기능 (cmclient / modded 이식)

- **modded 첫 번째 배치**: 차량 운행 이력 (`b89f93f9`) + 비행기 지상 활주 속도 조절 가능 (`b89f93f9`);
- **cmclient 두 번째 배치**: 위치 북마크 / 화물 상세 / 관전 (`089480b3`);
- **cmclient 영번째 배치**: `cmgamespeed` / `cmgamestats` / `cmexport` / `cmtreemap` 콘솔 명령 (`1fd94d12`);
- **cmclient 세 번째 배치 ① 강조**: 객체 수준 건설 미리보기 (역/철로/창고/공항), viewport 렌더링 파이프라인 및 건설 도구 연결 (6 커밋, `d97aa38a` → `1957bf45`);
- **cmclient 세 번째 배치 ③ 청사진**: 영역 선택 복사/회전/16 슬롯/재건 (`8e08ca6b`);
- **cmclient 네 번째 배치 ④ 도시 구역**: Tz 구역 + growth_tiles 색상 + GRWT 저장 청크 (`9e3f95a2`);
- **cmclient 다섯 번째 배치 ⑤ 명령 재생**: `cmdrecord` / `cmdreplay` (`f113acce28`);
- **콘솔 매개변수 규칙 수정**: 모든 jrpm 명령이 argv[1]부터 시작하는 매개변수 사용 (argv[0]은 명령 이름).

### 아키텍처 결론

- **② 명령 객체 계층 미이식**: 강조/청사진/재생이 각각 `CMD_ERROR`, 명령 클로저, jrpm 네이티브 명령 직렬화를 사용하여 우회, cmclient의 2251줄 생성 코드 불필요.

### 저장 호환성

- growth_tiles 데이터는 독립적인 `GRWT` 청크에 저장, 이전 저장 (해당 청크 없음) 로드 시 완전 호환;
- 명령 기록 파일 (`.jrcm`)은 jrpm 비공개 형식, 버전 간 호환 미보장.

## jrpm-0.1.0 (2026-08-14)

첫 번째 jrpm 버전, jgrpp 0.73.1 + pulsexlb px-patch 병합 기반, modded / cmclient 기능 이식.

### 변경 내용

- **pulsexlb px-patch 병합 (152 커밋)**:
  - 기관차 분리 (decouple): 분리/결합 명령, 노선표 이전, 결합 길이/속도 제한, 이중 기관차, NewGRF 결합, 결합 경로 탐색, 분리 후 독립 운행;
  - 모듈식 공항 (multitile-airport): air 유형 체계, PBS 항공 관제, YAPF 항공 경로 탐색, 공항 레이아웃 개조 (`allow_modify_airports`);
  - 저장 버전에 `SLV_MULTITILE_AIRPORTS` / `SLV_ORDER_DECOUPLE` 추가.
- **버전명 변경**: `openttd-jrpm` / `jrpm-0.1.0` (실행 파일명 및 수정 문자열).
- **서버 다중 버전 호환**: jrpm 서버가 jrpm / 원본 jgrpp / pulsexlb 클라이언트 수용.
- **jrpm 전용 기능**:
  - 리소스 다운로드 다중 미러 + 파일 수준 병렬 (동시 실행 수 구성 가능);
  - 차량 공유 명령별 자동 그룹화 (창 버튼 + `autogroup` 명령);
  - 전판 인식 AI (`AIGlobal` API + GlobalAI 예시, 설정 스위치 제어);
  - 건설 비용 Tooltip (요구에 따라 제거됨, 커밋 `96ebfb75`).
- **modded 이식 (첫 번째 배치)**:
  - 차량 운행 이력 (최근 10회 수익/점유율/시간, 차량 상세 창 History 버튼);
  - 비행기 지상 활주 속도 조절 가능 (`vehicle.plane_taxi_speed`, XSLF 기능 게이트, 이전 저장 호환).
- **cmclient 차용 (두 번째 배치)**:
  - 위치 북마크 (`savelocation` / `gotolocation`, 9 슬롯);
  - 회사 화물 상세 창 (`company_cargo`);
  - 관전 지원 (`watch <company_id>`).

### 저장 호환성

- jgrpp 관례 상속: trunk 저장 로드 가능 (최근 병합된 버전까지);
- jrpm 저장 (다중 타일 공항/분리 명령 데이터 포함) **미보장** 이전 jgrpp 저장과 상호 읽기;
- 저장 버전 번호 `SAVEGAME_VERSION`은 pulsexlb와 일치 (`SLV_CUSTOM_SUBSIDY_DURATION`);
- 새로운 기능은 모두 XSLF 확장 기능 게이트 또는 NOSAVE 사용, 이전 저장 손상 없음.

## 상위 버전

- **jgrpp 0.73.1**: 본 프로젝트 베이스 (OpenTTD 16.0 커널 + JGR 전체 기능).
- **pulsexlb px-patch 2608.3**: 기관차 분리 및 모듈식 공항 출처.
- **embeddedt/OpenTTD-modded (0.59.1 시대)**: 운행 이력 및 활주 속도 출처.
- **citymania-org/cmclient (vanilla 15.3)**: 위치 북마크/화물 상세/관전 차용 출처.

## 로드맵

- [x] 실제 기계 빌드 검증 및 첫 번째 컴파일 오류 수정
- [x] 운행 이력 + 활주 속도 (modded 첫 번째 배치)
- [x] 위치 북마크 + 화물 상세 + 관전 (cmclient 두 번째 배치)
- [ ] 강조 시스템 + 청사진 시스템 (cmclient 세 번째 배치, [로드맵](../features/highlight-blueprint-plan) 참조)
- [ ] HTTP 전송 계층 스레드 풀 + Range 분할 다운로드 (단일 대용량 파일 가속)
- [ ] 맵 전송 압축 비교 최적화
- [ ] 화물별 수익 통계 (저장 형식 확장 필요)
- [ ] (장기) Rust 서버 런타임 통합 (Openttd-Cluster 참조)