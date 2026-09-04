---
title: 기능 개요
---

# 기능 개요

jrpm = jgrpp 전체 기능 + pulsexlb (기관차 분리 + 모듈식 공항) + modded (운행 이력 + 활주 속도) + cmclient 차용 (멀티플레이 UI 강화) + jrpm 전용 기능.

## jrpm 전용 기능

| # | 기능 | 문서 | 상태 |
|---|---|---|---|
| 1 | 리소스 다운로드: 다중 미러 + 병렬 | [리소스 다운로드](./01-resource-download) | ✅ 구현됨 (동시 실행 수 구성 가능) |
| 2 | 서버 상한 연구 | [서버 상한 연구](./02-server-caps) | 📖 조사 결론 (구조적 제한) |
| 3 | 차량 자동 그룹화 | [차량 자동 그룹화](./03-vehicle-autogroup) | ✅ 구현됨 |
| 4 | 전판 인식 AI | [전판 인식 AI](./05-global-ai) | ✅ 구현됨 |

> 설명: 원래 F4 "건설 비용 Tooltip"은 요구에 따라 제거됨 (커밋 `96ebfb75`).

## modded 이식 (첫 번째 배치)

| 기능 | 문서 | 상태 |
|---|---|---|
| 차량 운행 이력 | [운행 이력](./triphistory) | ✅ 구현됨 |
| 비행기 지상 활주 속도 조절 가능 | [활주 속도](./plane-taxi-speed) | ✅ 구현됨 |

## cmclient 차용 (두 번째~다섯 번째 배치)

| 기능 | 문서 | 상태 |
|---|---|---|
| 위치 북마크 / 화물 상세 / 관전 | [멀티플레이 UI 강화](./ui-enhancements) | ✅ 구현됨 (두 번째 배치) |
| 객체 수준 강조 시스템 | [강조 + 청사진](./highlight-blueprint-plan) | ✅ 구현됨 (세 번째 배치, 6 커밋) |
| 청사진 시스템 (복사/회전/슬롯/재건) | [강조 + 청사진](./highlight-blueprint-plan) | ✅ 구현됨 (세 번째 배치) |
| 도시 구역 + growth_tiles 저장 | [도시 구역](./town-zoning) | ✅ 구현됨 (네 번째 배치) |
| 명령 기록 및 재생 | [명령 재생](./command-replay) | ✅ 구현됨 (다섯 번째 배치) |

> **② 명령 객체 계층** (cmclient의 2251줄 생성 코드): 이식 실증 결과 **전체 우회 가능** -- 강조는 `CMD_ERROR`로 비용 추정 대체, 청사진은 명령 클로저 사용, 재생은 jrpm 네이티브 명령 직렬화 사용, 따라서 이식하지 않음.

## 병합 기능 (pulsexlb에서)

| 기능 | 문서 | 설명 |
|---|---|---|
| 기관차 분리 (decouple) | [기관차 분리](./decouple) | 열차 분리/결합 전체 체계 |
| 모듈식 공항 (multitile-airport) | [모듈식 공항](./multitile-airport) | 다중 타일 공항 시스템 재구성 |

## 완전 상속

- **jgrpp 모든 기능**: 신호 강화 (다중 신호/프로그램 가능 신호/슬롯 및 카운터), scheduled dispatch, tracerestrict, 템플릿 교체, 기관차/객차 분리 구매 창, 현실 제동, 일방통행 도로 강화, 건널목 안전 등;
- **OpenTTD 16.0 커널**: NewGRF/스크립트/저장 생태계 완전 호환.

## 설정 빠른 찾기

| 기능 | 설정 |
|---|---|
| 병렬 다운로드 동시 실행 수 | `network.content_download_parallel` (1-8) |
| 다운로드 미러 목록 | `network.content_mirrors` |
| 콘텐츠 서버 | `network.content_server` |
| 공항 레이아웃 개조 | `station.allow_modify_airports` |
| 기본 항공 유형 | `gui.default_air_type` |
| AI 전판 인식 스위치 | `game.script.allow_global_ai_access` |
| 비행기 활주 속도 | `vehicle.plane_taxi_speed` (1-8, 기본 4) |

## 콘솔 명령 빠른 찾기

| 기능 | 명령 |
|---|---|
| 차량 자동 그룹화 | `autogroup train\|road\|ship\|aircraft` |
| 전판 인식 AI 추가 | `start_ai GlobalAI` |
| 위치 북마크 | `savelocation <1-9>` / `gotolocation <1-9>` |
| 회사 화물 상세 | `company_cargo <company_id>` |
| 회사 관전 | `watch <company_id>` |
| 청사진 복사/회전/저장/로드/재건 | `blueprint_copy` / `blueprint_rotate` / `blueprint_save <0-15>` / `blueprint_load <0-15>` / `blueprint_build` |
| 명령 기록/재생 | `cmdrecord [start [file]]` / `cmdrecord stop` / `cmdreplay <file>` |
| 게임 속도/통계/내보내기/나무 심기 | `cmgamespeed [n]` / `cmgamestats` / `cmexport` / `cmtreemap <file>` |