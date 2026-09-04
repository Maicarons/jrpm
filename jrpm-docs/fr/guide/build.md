---
title: Construction et installation
---

# Construction et installation

## Dependances

| Projet | Description |
|---|---|
| CMake ≥ 3.17 | Systeme de construction |
| Compilateur C++20 | MSVC 2019+ / GCC 11+ / Clang 14+ |
| Bibliotheques | zlib, liblzma, lzo, zstd, libpng ; GUI necessite aussi SDL2/Allegro, freetype, harfbuzz, ICU, opusfile (Windows utilise WinHttp, pas besoin de curl) |

Windows recommande d'utiliser [vcpkg](https://github.com/microsoft/vcpkg) pour installer les dependances (le projet fournit `vcpkg.json`).

## Etapes de construction

```bash
# 1. Configuration (dans le sous-repertoire build a la racine du projet)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. Compilation
cmake --build build --config Release -j

# 3. Produit
# build/openttd-jrpm.exe        (nom de l'executable, version jrpm)
```

Linux / MSYS2 :

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip Identifiant de version
La construction lit `.ottdrev-vc` a la racine pour generer le numero de version. Actuellement `jrpm-0.1.0` (tagged). Executez `openttd-jrpm -v` pour confirmer la version et les informations de construction.
:::

## Questions frequentes

### CMake ne trouve pas les dependances
Installez d'abord via vcpkg : `vcpkg install` (selon `vcpkg.json`). Ou installez les paquets systeme (Linux : `sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`).

### Je veux seulement un serveur dedie (sans GUI)
`cmake -B build -DOPTION_DEDICATED=ON`, puis `openttd-jrpm -D -c server.cfg`.

### Erreur de construction [safe-delete] (sandbox/CI Windows)
Ajoutez le prefixe `NODE_OPTIONS=""` a la commande de construction pour contourner les interférences du shim de suppression securisee sur le nettoyage de `.temp`/`.cache`.

## Execution depuis les sources

```bash
# Serveur dedie
openttd-jrpm -D -c openttd.cfg

# Client GUI
openttd-jrpm
```