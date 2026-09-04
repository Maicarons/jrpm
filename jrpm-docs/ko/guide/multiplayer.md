---
title: 멀티플레이 가이드
---

# 멀티플레이 가이드

## 버전 호환 전략

jrpm은 **태그된 버전** (`jrpm-0.1.0`)이며, 멀티플레이 핸드셰이크는 수정 문자열이 **완전히 일치**해야 합니다:

| 시나리오 | 동작 |
|---|---|
| jrpm 클라이언트 ↔ jrpm 서버 | ✅ 정상 멀티플레이 (버전 일치) |
| jrpm 클라이언트 ↔ 원본 jgrpp / pulsexlb 서버 | ❌ 거부 (버전 격리) |
| 원본 jgrpp 클라이언트 → jrpm 서버 | ✅ 허용 (서버 측에서 `jgrpp-*` 수정 완화) |
| pulsexlb 클라이언트 → jrpm 서버 | ✅ 허용 (`pxp` 수정 허용) |

::: warning NewGRF 버전
클라이언트가 어떤 버전에서 왔든, **NewGRF 버전 번호는 서버와 완전히 일치해야 합니다** (`_openttd_newgrf_version` 엄격 검증). 이는 결정론적 시뮬레이션의 최소 기준입니다.
:::

## 서버 열기

```bash
# 전용 서버 (GUI 없음)
openttd-jrpm -D -c server.cfg
```

`server.cfg` 권장 설정 (자세한 내용은 [서버 성능 튜닝](../performance/server-tuning) 참조):

```ini
[network]
server_name = My JRPM Server
server_port = 3979
max_clients = 32
max_companies = 15
frame_freq = 3
sync_freq = 50
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
```

## 서버 가입

- 게임 내 "멀티플레이 → 인터넷 서버 가입/서버 추가", 또는
- 명령줄: `openttd-jrpm -n <host>:<port>`

## 콘텐츠 다운로드 (NewGRF/시나리오)

jrpm은 **다중 미러 + 병렬 다운로드**를 지원합니다:

```ini
[network]
content_server = content.openttd.org        ; 메타데이터 서버
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; 병렬 다운로드 파일 수 (1-8)
```

- 미러 목록은 쉼표로 구분하며 순서대로 사용; 특정 미러 실패 시 자동으로 다음 미러로 전환;
- 환경 변수 `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS`가 설정 항목보다 우선순위 높음;
- 모든 미러 실패 시 자동으로 구버전 TCP 다운로드 프로토콜로 폴백.

## 서버 관리

- 콘솔 명령: `status`, `clients`, `kick`, `ban`, `save`, `reset_company`, `autogroup` (차량 자동 그룹화) 등;
- RCON: 서버 설정 `rcon_password`를 설정하면 원격에서 콘솔 명령 전송 가능.