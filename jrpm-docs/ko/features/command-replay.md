---
title: 명령 기록 및 재생
---

# 명령 기록 및 재생 (Command Record / Replay)

cmclient 명령 재생 이식 (다섯 번째 배치, 커밋 `f113acce28`). **cmclient의 명령 객체 계층을 이식하지 않음** -- jrpm 자체의 명령 직렬화 인프라 (`DynBaseCommandContainer`)를 직접 사용하여 "명령 객체 계층"을 완전히 우회.

## 콘솔 명령

```
cmdrecord [start [file]]    # 기록 시작 (기본 파일 cmdrecord.jrcm, 개인 디렉토리에 저장)
cmdrecord stop              # 중지 (10 tick 지연 후 명령 큐가 비워지면 디스크에 기록)
cmdreplay <file>            # 재생: 역직렬화 후 각 명령을 즉시 실행
```

## 구현 포인트

- **기록 훅**: `CommandRecordLog`가 `DoCommandPInternal`의 명령 실행 이후에 연결 -- 이것은 로컬, 네트워크, 재생 명령의 **유일한 실제 실행 지점**이므로 중복 기록되지 않음.
- **직렬화**: `cmd / tile / error_msg / payload / company`를 `DynBaseCommandContainer::Serialise`로 무손실 직렬화; 파일 형식 `JRCM` magic + version + count + 항목.
- **지연 중지**: `cmdrecord stop`은 10 tick 지연 플래그 설정, `StateGameLoop`가 매 프레임 확인 (`CommandRecordTick`)하여 큐에 이미 들어간 명령도 캡처된 후에 flush되도록 보장.
- **재생 실행**: 각 명령이 서버 명령 경로 (`DCIF_NETWORK_COMMAND`)로 즉시 실행, 재생 파일이 count 헤더를 업데이트하지 않았어도 실제 데이터를 기준으로 읽을 수 있음.

## 검증

전용 서버 E2E: `pause` 기록 → 26바이트 기록 파일 → 새 맵 재생 → `Game paused (manual)` + `Replay finished: 1 executed, 0 failed`.

## 일반적인 사용법

```
cmdrecord start build1     # 기록 시작
# ... 게임에서 철로 깔기, 역 건설 ...
cmdrecord stop             # 중지 및 디스크 쓰기
# 맵 교체 또는 크래시 복구 후:
cmdreplay build1           # 원클릭으로 모든 작업 재건
```