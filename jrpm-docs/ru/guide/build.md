---
title: Сборка и установка
---

# Сборка и установка

## Зависимости

| Компонент | Описание |
|---|---|
| CMake ≥ 3.17 | Система сборки |
| Компилятор C++20 | MSVC 2019+ / GCC 11+ / Clang 14+ |
| Библиотеки | zlib, liblzma, lzo, zstd, libpng; для GUI также SDL2/Allegro, freetype, harfbuzz, ICU, opusfile (Windows использует WinHttp, curl не требуется) |

На Windows рекомендуется использовать [vcpkg](https://github.com/microsoft/vcpkg) для установки зависимостей (проект включает `vcpkg.json`).

## Шаги сборки

```bash
# 1. Конфигурация (в подкаталоге build корневой директории проекта)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. Компиляция
cmake --build build --config Release -j

# 3. Результат
# build/openttd-jrpm.exe        (имя исполняемого файла, версия jrpm)
```

Linux / MSYS2:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip Идентификатор версии
При сборке читается `.ottdrev-vc` из корня для генерации номера версии. Текущая версия: `jrpm-0.1.0` (tagged). Запустите `openttd-jrpm -v` для проверки версии и информации о сборке.
:::

## Часто задаваемые вопросы

### CMake не находит зависимости
Сначала установите через vcpkg: `vcpkg install` (согласно `vcpkg.json`). Или установите системные пакеты (Linux: `sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`).

### Только выделенный сервер (без GUI)
`cmake -B build -DOPTION_DEDICATED=ON`, затем `openttd-jrpm -D -c server.cfg`.

### Ошибка сборки [safe-delete] (Windows sandbox/CI)
Добавьте префикс `NODE_OPTIONS=""` к команде сборки, чтобы обойти干扰 shim при очистке `.temp`/`.cache`.

## Запуск из исходников

```bash
# Выделенный сервер
openttd-jrpm -D -c openttd.cfg

# GUI клиент
openttd-jrpm
```