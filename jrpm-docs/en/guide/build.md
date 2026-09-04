---
title: Build & Install
---

# Build & Install

## Dependencies

| Item | Description |
|---|---|
| CMake ≥ 3.17 | Build system |
| C++20 Compiler | MSVC 2019+ / GCC 11+ / Clang 14+ |
| Libraries | zlib, liblzma, lzo, zstd, libpng; GUI also needs SDL2/Allegro, freetype, harfbuzz, ICU, opusfile (Windows uses WinHttp, no curl needed) |

Windows is recommended to use [vcpkg](https://github.com/microsoft/vcpkg) to install dependencies (the project includes a `vcpkg.json`).

## Build Steps

```bash
# 1. Configure (in the build subdirectory of the project root)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. Compile
cmake --build build --config Release -j

# 3. Output
# build/openttd-jrpm.exe        (executable name, jrpm version)
```

Linux / MSYS2:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip Version Identifier
The build reads `.ottdrev-vc` from the root directory to generate the version number. Currently `jrpm-0.1.0` (tagged). Run `openttd-jrpm -v` to confirm the version and build info.
:::

## Common Issues

### CMake cannot find dependencies
First install via vcpkg: `vcpkg install` (according to `vcpkg.json`). Or install system packages (Linux: `sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`).

### Only want to run a dedicated server (no GUI)
`cmake -B build -DOPTION_DEDICATED=ON`, then `openttd-jrpm -D -c server.cfg`.

### Build fails with [safe-delete] operation (Windows sandbox/CI)
Prefix the build command with `NODE_OPTIONS=""` to bypass the safe-delete shim interfering with `.temp`/`.cache` cleanup.

## Running from Source

```bash
# Dedicated server
openttd-jrpm -D -c openttd.cfg

# GUI client
openttd-jrpm
```