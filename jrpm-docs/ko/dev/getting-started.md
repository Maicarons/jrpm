---
title: 개발 가이드
---

# 개발 가이드

이 문서는 jrpm에서 계속 개발하는 방법을 설명합니다. 모든 패턴은 본 프로젝트에서 실제 구현된 코드를 기반으로 합니다 (`git log`의 커밋에서 확인 가능).

## 코드 조직 개요

| 디렉토리 | 역할 |
|---|---|
| `src/` | 모든 C++ 소스 코드 (최상위는 시스템별로 이름 지정: `rail_cmd.cpp`, `group_gui.cpp`…) |
| `src/network/` | 네트워킹 (서버/클라이언트/UDP/HTTP/콘텐츠 다운로드) |
| `src/script/` | 스크립트 프레임워크 (Squirrel); `api/`는 AI/GS API 클래스 |
| `src/table/settings/*.ini` | **설정 정의 소스** (settingsgen이 코드 생성) |
| `src/lang/english.txt` | 문자열 정의 (strgen이 생성) |
| `src/sl/saveload_common.h` | 저장 버전 (SLV) 열거형 |
| `bin/ai/` | AI 스크립트 (`GlobalAI`가 예시) |

## 일반적인 개발 패턴

- [새 게임 명령 추가](./add-command): 명령 열거형 + `DEF_CMD_TUPLE_NT` + 핸들러 + GUI/콘솔 진입점
- [새 스크립트 API 추가](./add-script-api): `script_*.hpp/.cpp` (자동 Squirrel 바인딩 등록)
- [새 설정 항목 추가](./add-setting): `.ini` + `settings_type.h` 필드 + 문자열

## 빌드

```bash
cmake -B build ..
cmake --build build -j
```

## 개발 주의사항

1. **인코딩**: 중국어가 포함된 소스 코드/문서 변경 후 UTF-8 주의 (본 기기의 Write/Edit 도구가 GBK 인코딩을 생성할 수 있음, `jgrpp-features/_fix_utf8.py`로 수정 가능);
2. **커밋 메시지**: 영어 권장 (터미널 인코딩 문제 회피);
3. **명령 열거형 중간 삽입**은 이후 명령 ID를 순서대로 이동시킴 -- 구버전 바이너리와 신버전 바이너리가 멀티플레이에서 불일치하지만, fork 내에서는 정상 현상이며 업그레이드 시 동기화 필요;
4. **저장 버전**: 저장 구조 변경 시 `src/sl/saveload_common.h`에 새 `SLV_*` 항목 추가 및 `SAVEGAME_VERSION` 업데이트;
5. **새 파일**: `.cpp`는 반드시 해당 `CMakeLists.txt` 소스 목록에 추가해야 함 (`src/CMakeLists.txt` 또는 하위 디렉토리/`script/api/CMakeLists.txt`); `script_*.hpp`는 `file(GLOB)`로 자동 발견되므로 등록 불필요;
6. **새 문자열**: `src/lang/english.txt`에 추가 (다른 언어는 기본적으로 영어로 폴백);
7. **NewGRF/저장 호환성**: `_openttd_content_version` / NewGRF 버전 번호 변경 시 주의 (멀티플레이 및 콘텐츠 호환성에 영향).

## 회귀 테스트 제안 (중요)

현재 브랜치는 **실제 기계에서 컴파일되지 않았음**, 첫 번째 빌드 후 다음 순서로 회귀 테스트 권장:
1. 기본: `openttd-jrpm -v`가 `jrpm-0.1.0` 표시;
2. 저장: 싱글 플레이로 1-2년 실행;
3. 멀티플레이: jrpm 클라이언트 상호 연결; jgrpp/pxp 클라이언트 가입;
4. 새 기능: 기관차 분리 (분리/결합/역방향), 모듈식 공항 (레이아웃 개조), 병렬 다운로드, 자동 그룹화, 건설 tooltip, GlobalAI;
5. 기존 기능 회귀: 신호, tracerestrict, scheduled dispatch, 템플릿 교체.