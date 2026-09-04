---
title: 빌드 및 설치
---

# 빌드 및 설치

## 의존성

| 항목 | 설명 |
|---|---|
| CMake ≥ 3.17 | 빌드 시스템 |
| C++20 컴파일러 | MSVC 2019+ / GCC 11+ / Clang 14+ |
| 라이브러리 | zlib, liblzma, lzo, zstd, libpng; GUI는 SDL2/Allegro, freetype, harfbuzz, ICU, opusfile도 필요 (Windows는 WinHttp 사용, curl 불필요) |

Windows는 [vcpkg](https://github.com/microsoft/vcpkg)를 사용하여 의존성 설치를 권장합니다 (프로젝트에 `vcpkg.json` 포함).

## 빌드 단계

```bash
# 1. 설정 (프로젝트 루트의 build 하위 디렉토리)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. 컴파일
cmake --build build --config Release -j

# 3. 산출물
# build/openttd-jrpm.exe        (실행 파일명, jrpm 버전)
```

Linux / MSYS2:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip 버전 식별자
빌드 시 루트 디렉토리의 `.ottdrev-vc`를 읽어 버전 번호를 생성합니다. 현재는 `jrpm-0.1.0` (태그됨)입니다. `openttd-jrpm -v`를 실행하여 버전과 빌드 정보를 확인할 수 있습니다.
:::

## 자주 묻는 질문

### CMake가 의존성을 찾지 못함
먼저 vcpkg로 설치: `vcpkg install` (`vcpkg.json` 기준). 또는 시스템 패키지 설치 (Linux: `sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`).

### 전용 서버만 실행 (GUI 없음)
`cmake -B build -DOPTION_DEDICATED=ON`, 그런 다음 `openttd-jrpm -D -c server.cfg`.

### [safe-delete] 작업 실패 (Windows 샌드박스/CI)
빌드 명령 앞에 `NODE_OPTIONS=""` 접두사를 추가하여 `.temp`/`.cache` 정리에 대한 안전 삭제 shim 간섭을 우회할 수 있습니다.

## 소스에서 실행

```bash
# 전용 서버
openttd-jrpm -D -c openttd.cfg

# GUI 클라이언트
openttd-jrpm
```