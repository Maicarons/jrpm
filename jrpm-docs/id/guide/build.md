---
title: Membangun & Menginstal
---

# Membangun & Menginstal

## Dependensi

| Proyek | Deskripsi |
|---|---|
| CMake ≥ 3.17 | Sistem build |
| Kompiler C++20 | MSVC 2019+ / GCC 11+ / Clang 14+ |
| Pustaka | zlib, liblzma, lzo, zstd, libpng; GUI juga membutuhkan SDL2/Allegro, freetype, harfbuzz, ICU, opusfile (Windows menggunakan WinHttp, tidak perlu curl) |

Windows disarankan menggunakan [vcpkg](https://github.com/microsoft/vcpkg) untuk menginstal dependensi (proyek menyertakan `vcpkg.json`).

## Langkah Membangun

```bash
# 1. Konfigurasi (di subdirektori build di root proyek)
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

# 2. Kompilasi
cmake --build build --config Release -j

# 3. Hasil
# build/openttd-jrpm.exe        (nama file eksekusi, versi jrpm)
```

Linux / MSYS2：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# build/openttd-jrpm
```

::: tip Identifikasi Versi
Saat membangun, file `.ottdrev-vc` di root akan dibaca untuk menghasilkan nomor versi. Saat ini `jrpm-0.1.0` (tagged). Jalankan `openttd-jrpm -v` untuk mengonfirmasi versi dan informasi build.
:::

## Pertanyaan Umum

### CMake tidak menemukan dependensi
Instal terlebih dahulu melalui vcpkg: `vcpkg install` (sesuai `vcpkg.json`). Atau instal paket sistem (Linux：`sudo apt install zlib1g-dev liblzma-dev liblzo2-dev libzstd-dev libpng-dev libsdl2-dev libfreetype6-dev libharfbuzz-dev libicu-dev libopusfile-dev`).

### Hanya ingin menjalankan server khusus (tanpa GUI)
`cmake -B build -DOPTION_DEDICATED=ON`, lalu `openttd-jrpm -D -c server.cfg`.

### Build gagal dengan operasi [safe-delete] (Sandbox/CI Windows)
Tambahkan prefiks `NODE_OPTIONS=""` pada perintah build untuk melewati gangguan shim penghapusan aman pada pembersihan `.temp`/`.cache`.

## Menjalankan dari Sumber

```bash
# Server khusus
openttd-jrpm -D -c openttd.cfg

# Klien GUI
openttd-jrpm
```