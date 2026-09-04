---
title: Construccion e instalacion
---

# Construccion e instalacion

## Dependencias

| Proyecto | Descripcion |
|---|---|
| CMake ≥ 3.17 | Sistema de construccion |
| Compilador C++20 | MSVC 2019+ / GCC 11+ / Clang 14+ |
| Librerias | zlib, liblzma, lzo, zstd, libpng; GUI tambien necesita SDL2/Allegro, freetype, harfbuzz, ICU, opusfile (Windows usa WinHttp, no necesita curl) |

En Windows se recomienda usar [vcpkg](https://github.com/microsoft/vcpkg) para instalar las dependencias (el proyecto incluye `vcpkg.json`).

## Pasos de construccion

```bash
# 1. Configuracion (en el subdirectorio build de la raiz del proyecto)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. Compilacion
cmake --build build --config Release -j

# 3. Resultado
# build/openttd-jrpm.exe        (nombre del ejecutable, version jrpm)
```

Linux / MSYS2:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip Identificador de version
Durante la construccion se lee `.ottdrev-vc` de la raiz para generar el numero de version. Actualmente es `jrpm-0.1.0` (tagged). Ejecute `openttd-jrpm -v` para confirmar la version y la informacion de construccion.
:::

## Preguntas frecuentes

### CMake no encuentra las dependencias
Instale primero con vcpkg: `vcpkg install` (segun `vcpkg.json`). O instale los paquetes del sistema (Linux: `sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`).

### Solo quiero ejecutar un servidor dedicado (sin GUI)
`cmake -B build -DOPTION_DEDICATED=ON`, luego `openttd-jrpm -D -c server.cfg`.

### Error de compilacion [safe-delete] (Windows sandbox/CI)
Anada el prefijo `NODE_OPTIONS=""` al comando de construccion para evitar la interferencia del shim de eliminacion segura en la limpieza de `.temp`/`.cache`.

## Ejecucion desde codigo fuente

```bash
# Servidor dedicado
openttd-jrpm -D -c openttd.cfg

# Cliente GUI
openttd-jrpm
```