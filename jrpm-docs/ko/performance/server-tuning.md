---
title: 서버 성능 튜닝 가이드
---

> merge 후 jrpm 브랜치 기준 (pulsexlb 기능 및 jrpm 전용 최적화 포함).
> 매개변수 기본값은 `src/table/settings/network_settings.ini`에서 가져왔으며, 실제 부하 테스트 결과를 기준으로 조정하는 것이 좋습니다.

## 一、이미 적용된 성능 최적화 (이번)

| 최적화 | 커밋 | 설명 |
|---|---|---|
| 병렬 다운로드 동시 실행 수 구성 가능 | 4716b925 | `network.content_download_parallel` (기본 4, 1-8): 콘텐츠 다운로드 시 동시에 진행할 파일 수, 하드코딩 4 대체 |
| 다중 미러 + 파일 수준 병렬 다운로드 | d4c45740 (F1) | `network.content_mirrors` 쉼표 구분 다중 미러, 실패 시 미러별 재시도, 최종적으로 구버전 프로토콜 폴백 |
| 서버 다중 버전 클라이언트 호환 | cb9848b7 | 서버가 jrpm / 원본 jgrpp / pulsexlb 클라이언트를 동시에 수용 가능 |

## 二、서버 성능 핵심 매개변수 (jgrpp 체계)

| 설정 | 기본값 | 범위 | 의미 | 튜닝 제안 |
|---|---|---|---|---|
| `network.sync_freq` | 100 | 0-100 | 몇 프레임마다 한 번씩 프레임 동기화 검사 (desync 감지). 클수록 대역폭 절약, 작을수록 더 빨리 불일치 발견 | 멀티플레이 안정성 우선 시 20-50으로 낮춤; desync 자주 발생하면 낮춤 |
| `network.frame_freq` | 0 | 0-100 | 서버가 몇 프레임마다 한 번씩 명령 프레임을 패킹하여 전송 (0 = 매 프레임 전송). 클수록 대역폭/CPU 절약, 하지만 조작 지연 증가 | 일반 0-3; 플레이어 많은 서버는 5까지 가능, 부하 테스트 후 결정 |
| `network.commands_per_frame` | 2 | 1-65535 | 프레임당 처리할 클라이언트 명령 수 상한 (도배/악성 방지) | 플레이어 많고 조작 빈번할 때 4-8로 상향 |
| `network.commands_per_frame_server` | 16 | 1-65535 | 서버 자체의 프레임당 명령 수 상한 | 일반적으로 변경 불필요 |
| `network.bytes_per_frame` | 8 | 1-65535 | 장기 평균 프레임당 수신 바이트 수 상한 (대역폭 성형) | 대역폭 좋으면 16-32로 상향, 대형 맵 동기화 속도 향상 |
| `network.bytes_per_frame_burst` | 256 | 1-65535 | 버스트 바이트 상한 (단기 피크 허용) | 위 항목과 함께 상향, 예: 512 |
| `network.max_init_time` | 60 | 0-32000 | 클라이언트 초기화 시간 초과 (tick) | 약한 네트워크 플레이어 많으면 완화 |
| `network.max_join_time` | 500 | 0-32000 | 클라이언트 가입 (맵 다운로드+동기화) 시간 초과 (tick) | 대형 맵/느린 대역폭에서 1000+로 완화 |
| `network.max_download_time` | 1000 | 0-32000 | 맵 다운로드 시간 초과 (tick) | 대형 맵 (4096+)은 2000+ 권장 |
| `network.max_lag_time` | 800 | 0-32000 | 클라이언트 최대 지연 허용 (tick) | 높은 지연 플레이어 많을 때 완화, 하지만 desync 위험 증가 |

> 참고: `max_*_time` 단위는 게임 tick (1/74초≈13.5ms); `bytes_per_frame`는 프레임 동기화 윈도우의 평균 바이트를 의미.

## 三、권장 서버 시작 설정 (멀티플레이 시나리오)

```ini
[network]
max_clients = 32            ; 또는 필요에 따라
max_companies = 15
frame_freq = 3              ; 0=매 프레임 명령 전송 (가장 부드러움); 3=절충, 대역폭 절약
sync_freq = 50              ; 더 빨리 불일치 발견
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
content_download_parallel = 4   ; 클라이언트 측 콘텐츠 다운로드 동시 실행
```

## 四、추후 구현 가능한 성능 최적화 방향 (우선순위별)

1. **HTTP 전송 계층 스레드 풀 + Range 분할 다운로드** (Openttd-Cluster 0007 아이디어)
   현재 상태: F1에서 이미 "파일 수준 병렬" (여러 파일 동시 다운로드) 구현. 0007은 HTTP 계층에서 스레드 풀 + `CURLOPT_RANGE` 분할을 통해 **단일 대용량 파일** (거대 시나리오 .tar.gz)도 추가 가속 가능. 둘은 상호 보완적이지만 동일한 파일을 변경하므로 F1 안정화 후 증분 작업 권장.

2. **맵 전송 최적화**
   jrpm이 기본적으로 맵 압축 (zstd/lzma)을 활성화하는지 확인; 대형 맵에서 다양한 압축 알고리즘의 소요 시간 비교 가능.

3. **Rust 서버 런타임 (장기 아키텍처 참조)**
   Openttd-Cluster의 otc-engine (Admin/RCON, Prometheus 메트릭, 클러스터 failover, 웹 패널, 스냅샷 브리지)은 완전한 Rust 엔지니어링 FFI 통합에 의존하므로, "차세대 서버"급 개조이며 패치 수준 병합이 아님; jrpm은 현재 순수 C++ 단일 바이너리 유지.

4. **게임 시뮬레이션 성능**
   - 대형 맵 + 많은 차량 시 `economy`/`linkgraph` 매개변수 확인 (linkgraph_settings.ini);
   - 극한 성능 필요 시 `settings_game.economy.` 클래스와 pathfinder 상한 매개변수 (`pathfinding` 설정) 연구 가능.

## 五、검증 제안

- 서버 오픈 `openttd-jrpm -D -c server.cfg`, 여러 jrpm 클라이언트로 실제 측정: 가입 시간, 프레임 동기화 안정성 (desync 메시지 없음), CPU/메모리 사용량;
- 콘텐츠 다운로드: `content_download_parallel=8` 설정 후 NewGRF 배치 다운로드 시간 비교;
- 약한 네트워크 부하 테스트: `max_lag_time`와 `sync_freq` 조합 테스트로 안정적인 경계 찾기.