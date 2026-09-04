---
title: 프로젝트 소개
---

# OpenTTD-JRPM 프로젝트 소개

## 소개

**OpenTTD-JRPM (jrpm)** 은 [JGR의 Patchpack](https://github.com/JGRennison/OpenTTD-patches) (jgrpp) 기반의 2차 개발 브랜치로, [pulsexlb/OpenTTD-patches](https://github.com/pulsexlb/OpenTTD-patches)의 "기관차 분리" 및 "모듈식 공항" 기능을 융합하고 jrpm 전용 기능을 추가했습니다. 현재 버전 **jrpm-0.1.0**입니다.

독립 설치 및 독립 멀티플레이가 가능한 완전히 새로운 버전입니다: jrpm 클라이언트/서버는 독립적인 버전 식별자 (`jrpm-0.1.0`)로 핸드셰이크하여 원본 jgrpp, pulsexlb 버전과 완전히 격리되어 버전 혼동을 방지합니다.

## 기능 개요

### pulsexlb에서 가져옴 (152개 커밋, git merge로 통합)
- **기관차 분리 (decouple)**: 열차 분리/결합 전체 체계 -- 분리 명령, 노선표 이전, 결합 길이와 속도 제한, 이중 기관차 지원, NewGRF 결합, 결합 경로 탐색 (YAPF/NPF), 분리 후 두 열차 독립 운행;
- **모듈식 공항 (multitile-airport)**: 다중 타일 공항 시스템 재구성 -- air 유형 체계 (`air.h`/`air_type.h`/`newgrf_airtype.*`), PBS 항공 관제 (`pbs_air.*`), YAPF 항공 경로 탐색, `station.allow_modify_airports` 공항 레이아웃 개조, `gui.default_air_type` 기본 항공 유형.

### jrpm 전용
| 기능 | 설명 | 진입점 |
|---|---|---|
| 리소스 다운로드: 다중 미러 + 병렬 | 쉼표 구분 다중 미러, 파일 수준 병렬 다운로드 (동시 실행 수 구성 가능), 실패 시 자동 미러 전환 | 설정 → `network.content_mirrors` / `network.content_download_parallel` |
| 차량 자동 그룹화 | 공유 명령/일정에 따라 원클릭 자동 그룹 생성 및 배정, 그룹명은 자동으로 노선명 사용 | 그룹화 창 버튼 / 콘솔 `autogroup` |
| 건설 비용 Tooltip | 철로/도로/지형 건설 시 마우스 위에 실시간 예상 비용 표시 | 철로/도로/지형 도구 모음 |
| 전판 인식 AI | NoAI 유지, 새로운 `AIGlobal` 전역 API + 예시 AI GlobalAI, 게임 설정 접근 제어 | `game.script.allow_global_ai_access` |
| 서버 다중 버전 호환 | jrpm 서버가 jrpm / 원본 jgrpp / pulsexlb 클라이언트를 동시에 수용 | 서버 가입 검사 자동 적용 |

### 완전 상속
- jgrpp 모든 기능 (신호 강화, scheduled dispatch, tracerestrict, 템플릿 교체 등);
- 상위 OpenTTD 모든 기능과 NewGRF/스크립트 생태계 호환.

## 버전 관계

```
                 jgrpp-0.73.1 (공통 조상)
                 /                 \
 jgrpp 브랜치 (63 커밋)        pulsexlb px-patch (152 커밋)
 ├ jgrpp 최근 업데이트             ├ 기관차 분리 (decouple 브랜치)
 ├ jrpm 전용 기능                  └ 모듈식 공항 (multitile-airport 브랜치)
 └ 버전명 변경 jrpm-0.1.0
                 \                 /
                  jrpm 브랜치 (git merge)
```

## 브랜치와 커밋

- 브랜치: `jrpm` (개발 메인라인)
- 주요 커밋:
  - `d4c45740` jrpm 전용 5 기능
  - `71fe214c` merge pulsexlb (분리 + 모듈식 공항)
  - `425e7207` 버전명 변경 openttd-jrpm / jrpm-0.1.0
  - `cb9848b7` 서버 다중 버전 클라이언트 호환
  - `4716b925` 병렬 다운로드 동시 실행 수 구성 가능

## 라이선스

OpenTTD와 동일: **GPL-2.0**.