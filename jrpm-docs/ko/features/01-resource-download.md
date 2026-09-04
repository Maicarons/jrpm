---
title: "리소스 다운로드: 다중 미러 + 병렬 다운로드"
---

## 현재 상태 (조사 결론)

| 항목 | 현재 상태 |
|---|---|
| 콘텐츠 서버 (메타데이터 프로토콜) | `src/network/core/config.cpp` `NetworkContentServerConnectionString()`: 환경 변수 `OTTD_CONTENT_SERVER_CS`, 기본값 `content.openttd.org` (TCP 3978 포트) |
| 다운로드 미러 | `NetworkContentMirrorUriString()`: 환경 변수 `OTTD_CONTENT_MIRROR_URI`, 기본값 `https://binaries.openttd.org/bananas` |
| 다운로드 방식 | `network_content.cpp` `DownloadSelectedContentHTTP()`: **단일** POST 요청으로 모든 content ID 전송 → 미러가 파일 헤더 목록 반환 (`id,type,filesize,url` 각 줄) → **하나씩 직렬**로 각 파일 다운로드 (각 파일당 하나의 GET 연결) → `AfterDownload()` gunzip + tar 압축 풀기 |
| 스레드 모델 | HTTP 계층 (WinHttp) 자체는 백그라운드 스레드에서 처리하지만, **파일 다운로드는 직렬 큐잉**됨; 미러 목록 없음, 게임 내 설정 없음, 병렬 없음 |

## 본 기능 구현

### 1. 새로운 설정 (`network_settings.ini` + `settings_type.h`)

- `network.content_server` (SLE_STR, 기본값 비움 = 공식 소스 사용)
- `network.content_mirrors` (SLE_STR, 쉼표로 구분된 여러 미러 URI, 기본값 비움 = 공식 미러)

우선순위: 환경 변수 > 게임 설정 > 공식 기본값.

### 2. 설정 파싱 (`src/network/core/config.cpp/h`)

- `NetworkContentServerConnectionString()` 설정 읽기;
- 새로운 `NetworkContentMirrorUris()` 쉼표 구분 목록 파싱 (공백 제거, 빈 항목 건너뛰기, 기본 공식 소스 사용);
- `NetworkContentMirrorUriString()`은 목록의 첫 번째 항목 반환.

### 3. 병렬 다운로드 (`src/network/network_content.h/.cpp`)

- 새로운 `ContentFileDownload` (단일 대기 다운로드 파일: id/type/filesize/url/filename);
- 새로운 `ContentDownloadSession : HTTPCallback` (각 파일의 독립적인 다운로드 상태 및 콜백; `IsCancelled` 핸들러와 연동);
- `DownloadSelectedContentHTTP()`: POST를 `mirrors[mirror_index]`로 전송 → `ParseResponseHeaders()`에서 한 번에 모든 파일 헤더 파싱 → `StartDownloadSessions()`에서 최대 **4개의 병렬 세션** 시작 (`CONTENT_DOWNLOAD_PARALLEL`), 각 세션 완료 후 자동으로 다음 대기 파일 수령;
- 실패 재시도 체인: 세션 실패/미러 요청 실패 → 다음 미러로 재요청 → 모든 미러 실패 → 구버전 TCP 프로토콜로 폴백 (`DownloadSelectedContentFallback`);
- 우아한 종료: 실패/취소 시 `download_cancelled` 설정, 진행 중인 세션이 모두 종료될 때까지 대기 (`OnAllSessionsDone`) 후 재시도 또는 폴백, 매달린 콜백 방지;
- `ResetMirrorIndex()`: 새 다운로드는 첫 번째 미러에서 시작 (GUI 트리거 시 호출).

## 관련 파일

- `src/table/settings/network_settings.ini` (2개 설정 항목 추가)
- `src/settings_type.h` (`NetworkSettings`에 2개 필드 추가)
- `src/network/core/config.cpp` / `config.h` (미러 목록 파싱)
- `src/network/network_content.h` / `.cpp` (병렬 세션)
- `src/network/network_content_gui.cpp` (다운로드 전 미러 인덱스 재설정)

## 검증 포인트

1. `network.content_mirrors`에 여러 URI (쉼표 구분) 입력 후 콘텐츠 다운로드가 작동하고 **병렬로** 여러 파일을 다운로드해야 함 (다운로드 진행 상황 관찰/패킷 캡처로 여러 동시 연결 확인);
2. 네트워크 단절/오류 미러 시 자동으로 다음 미러 시도, 마지막으로 구버전 프로토콜 폴백;
3. 다운로드 중 취소 시 `.tar.gz` 반제품이 남지 않아야 함;
4. 실제 기계에서 컴파일 검증 필요 (이 저장소에는 빌드 환경 없음, 코드 미컴파일).

## 알려진 제한

- 병렬도가 하드코딩 4 (`CONTENT_DOWNLOAD_PARALLEL`), 추후 설정으로 만들 수 있음;
- 미러 프로토콜은 공식 bananas API 형식에 의존 (POST id 목록으로 파일 헤더 목록 반환).