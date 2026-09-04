---
title: Ikhtisar Penggabungan Versi jrpm
---

> Cabang: `jrpm` ｜ Versi: jrpm-0.1.0 (tagged, 2026-08-14)
> Hasil build: `openttd-jrpm` (nama file eksekusi)

## Hubungan Versi

```
                        jgrpp-0.73.1 (leluhur bersama)
                        /                 \
        cabang jgrpp (63 komit)          pulsexlb px-patch (152 komit)
        ├ tracerestrict dll. pembaruan terbaru  ├ jgrpp-decouple (kopling lokomotif)
        ├ 5 fitur saya (d4c45740)                └ jgrpp-multitile-airport (bandara modular)
        └ ganti nama versi jrpm-0.1.0 (425e7207)
                        \                 /
                        cabang jrpm (merge 71fe214c + kompatibilitas cb9848b)
```

## Konten Penggabungan

### 1. pulsexlb/OpenTTD-patches (px-patch penuh 152 komit) → Sudah digabung

| Fitur | Deskripsi | File Utama |
|---|---|---|
| **Kopling Lokomotif (decouple)** | Pemisahan/penggandengan kereta: perintah decouple, transfer tanda, batas panjang kopling/kecepatan, dua lokomotif, kopling NewGRF, navigasi kopling (YAPF/NPF), penjadwalan independen setelah decouple | train_cmd.cpp, order_cmd.cpp, order_gui.cpp, train.h, yapf/npf |
| **Bandara Modular (multitile-airport)** | Restrukturisasi sistem bandara multi-ubin: sistem tipe air (air.h/air_type.h/newgrf_airtype.*), penjadwalan udara PBS (pbs_air.*), navigasi udara YAPF, `station.allow_modify_airports` (modifikasi tata letak bandara), `gui.default_air_type`, sprite bandara multi-ubin | air.*, pbs_air.*, aircraft_cmd.cpp (restrukturisasi 3600 baris), airport_cmd/gui, station_cmd |

Penanganan konflik: Hanya 2 file header konflik (aircraft.h / airport.h)——restrukturisasi penerbangan pulsexlb menghapus tipe mati yang **tidak memiliki referensi** di workspace (`VehicleAirFlags` bitset, `AirportMovingDataFlag`), ambil sisi penghapusan pulsexlb, telah dikonfirmasi tidak ada file lain yang mereferensi.

### 2. Openttd-Cluster (proyek cluster Rust pengguna) → Pinjaman selektif

| Patch | Penanganan | Deskripsi |
|---|---|---|
| 0006 vanilla-native-server (kompatibilitas klien multi-versi) | ✅ **Sudah digabung** (cb9848b7) | Server jrpm secara bersamaan menerima klien jrpm / jgrpp asli (`jgrpp-`) / pulsexlb (`pxp`); versi NewGRF tetap diverifikasi ketat |
| 0001 revision-handshake / 0005 version-metadata | ✅ Ide sudah diadopsi | jrpm menggunakan string revisi tagged independen `jrpm-0.1.0`, jabat tangan online terisolasi dari jgrpp/pxp, mewujudkan "versi baru yang mudah untuk online" |
| 0007 parallel-download (thread pool HTTP + Range chunk, 30KB) | 📝 Referensi, tidak digabung | Dengan F1 proyek ini "paralel file + multi-cermin" tema sama dan mengubah file yang sama; **thread pool lapisan transportasi/unduhan chunk** 0007 dicatat sebagai arah peningkatan F1 selanjutnya |
| 0002-0004 snapshot/command/FFI bridge | 📝 Referensi arsitektur | Bergantung pada seluruh runtime Rust otc-engine (tautan statis FFI), termasuk "integrasi keseluruhan jangka panjang" bukan penggabungan level patch; jrpm saat ini tetap mempertahankan biner tunggal C++ murni |

### 3. Eksklusif jrpm (5 fitur sebelumnya, d4c45740) → Sudah dalam cabang jrpm

Unduhan paralel (multi-cermin + 4 sesi bersamaan), pengelompokan otomatis, tooltip bangunan, AI sadar permainan penuh (ScriptGlobal + GlobalAI), pengaturan cermin/server konten.

## Strategi Online ("Versi baru yang mudah untuk online")

- jrpm adalah **versi tagged**: `IsNetworkCompatibleVersion` memerlukan string revisi cocok persis → **klien jrpm hanya online dengan server jrpm**, sepenuhnya terisolasi dari jgrpp 0.73.x / pxp;
- **Pelonggaran server**: Server jrpm juga menerima klien `jgrpp-*` dan `pxp*` untuk bergabung (`IsJgrppNativeNetworkRevision` / `IsPxpNetworkRevision`, versi NewGRF harus sama);
- Oleh karena itu: Pemilik server jrpm membuka server = hanya menerima pemain jrpm (default); saat perlu kompatibilitas dengan klien lama, tidak perlu mengubah konfigurasi untuk menerima pemain jgrpp/pxp.

## Build dan Verifikasi (dieksekusi oleh pengguna di mesin sendiri)

```bash
# Pertama kali (perlu CMake + dependensi, lihat COMPILING.md)
cmake -B build ..
cmake --build build -j
# Hasil: build/openttd-jrpm.exe
```

Prioritas verifikasi:
1. `openttd-jrpm -v` menampilkan `jrpm-0.1.0`;
2. Buka game single player jalankan 1-2 tahun (penggabungan melibatkan perubahan besar train/airport + versi arsip mungkin naik karena allow_modify_airports dll. yang baru);
3. Setelah buka server: klien jrpm bergabung ✓; klien jgrpp 0.73.x asli mencoba bergabung (diharapkan bisa masuk, saat NewGRF sama);
4. Kopling lokomotif: tambahkan perintah decouple/couple ke kereta, verifikasi dua kereta berjalan independen setelah decouple; Bandara modular: aktifkan `station.allow_modify_airports` lalu modifikasi tata letak bandara;
5. Regresi 5 fitur sebelumnya (unduhan paralel, pengelompokan otomatis, tooltip bangunan, GlobalAI).

## Risiko yang Diketahui

- **Belum diverifikasi kompilasi**: Kode yang digabung+diubah belum dikompilasi di mesin ini (tanpa toolchain), kompilasi pertama di mesin nyata mungkin ada perubahan antarmuka yang terlewat (terutama tiga sistem besar aircraft/airport/train);
- Versi arsip: px-patch mungkin menaikkan SLV (M9 menyebutkan savegame version gate), arsip jrpm dan arsip jgrpp 0.73.x mungkin tidak dapat dibaca bersama (sama dengan konvensi jgrpp, kompatibel ke bawah dengan arsip trunk);
- Merge membawa seluruh sejarah pulsexlb, jika perlu melacak kepemilikan fitur gunakan `git log --oneline pulsexlb/px-patch`.