---
title: Construcao e Instalacao
---

# Construcao e Instalacao

## Dependencias

| Item | Descricao |
|---|---|
| CMake >= 3.17 | Sistema de construcao |
| Compilador C++20 | MSVC 2019+ / GCC 11+ / Clang 14+ |
| Bibliotecas | zlib, liblzma, lzo, zstd, libpng; GUI tambem requer SDL2/Allegro, freetype, harfbuzz, ICU, opusfile (Windows usa WinHttp, sem necessidade de curl) |

Windows recomenda usar [vcpkg](https://github.com/microsoft/vcpkg) para instalar dependencias (o projeto inclui `vcpkg.json`).

## Passos de Construcao

```bash
# 1. Configurar (no subdiretorio build da raiz do projeto)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. Compilar
cmake --build build --config Release -j

# 3. Artefato
# build/openttd-jrpm.exe        (nome do executavel, versao jrpm)
```

Linux / MSYS2:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip Identificador de Versao
Durante a construcao, o arquivo `.ottdrev-vc` na raiz e lido para gerar o numero da versao. Atualmente e `jrpm-0.1.0` (tagged). Execute `openttd-jrpm -v` para confirmar a versao e informacoes de construcao.
:::

## Perguntas Frequentes

### CMake nao encontra dependencias
Instale primeiro via vcpkg: `vcpkg install` (conforme `vcpkg.json`). Ou instale pacotes do sistema (Linux: `sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`).

### Quero apenas executar um servidor dedicado (sem GUI)
`cmake -B build -DOPTION_DEDICATED=ON`, entao `openttd-jrpm -D -c server.cfg`.

### Erro de construcao [safe-delete] (Windows sandbox/CI)
Adicione o prefixo `NODE_OPTIONS=""` ao comando de construcao para contornar a interferencia do shim de exclusao segura na limpeza de `.temp`/`.cache`.

## Executando a partir do codigo fonte

```bash
# Servidor dedicado
openttd-jrpm -D -c openttd.cfg

# Cliente GUI
openttd-jrpm
```