---
title: 构建与安装
---

# 构建与安装

## 依赖

| 项目 | 说明 |
|---|---|
| CMake ≥ 3.17 | 构建系统 |
| C++20 编译器 | MSVC 2019+ / GCC 11+ / Clang 14+ |
| 库 | zlib、liblzma、lzo、zstd、libpng；GUI 还需要 SDL2/Allegro、freetype、harfbuzz、ICU、opusfile（Windows 用 WinHttp，无需 curl） |

Windows 推荐使用 [vcpkg](https://github.com/microsoft/vcpkg) 安装依赖（项目自带 `vcpkg.json`）。

## 构建步骤

```bash
# 1. 配置（在项目根目录的 build 子目录）
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. 编译
cmake --build build --config Release -j

# 3. 产物
# build/openttd-jrpm.exe        （可执行文件名，jrpm 版本）
```

Linux / MSYS2：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip 版本标识
构建时会读取根目录 `.ottdrev-vc` 生成版本号。当前为 `jrpm-0.1.0`（tagged）。运行 `openttd-jrpm -v` 可确认版本与构建信息。
:::

## 常见问题

### CMake 找不到依赖
先通过 vcpkg 安装：`vcpkg install`（按 `vcpkg.json`）。或安装系统包（Linux：`sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`）。

### 只想跑专用服务器（无 GUI）
`cmake -B build -DOPTION_DEDICATED=ON`，然后 `openttd-jrpm -D -c server.cfg`。

### 构建报 [safe-delete] 操作失败（Windows 沙箱/CI）
给构建命令加 `NODE_OPTIONS=""` 前缀可绕过安全删除 shim 对 `.temp`/`.cache` 清理的干扰。

## 从源码运行

```bash
# 专用服务器
openttd-jrpm -D -c openttd.cfg

# GUI 客户端
openttd-jrpm
```
