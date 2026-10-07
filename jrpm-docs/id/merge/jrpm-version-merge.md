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

---

## Log penggabungan: 2026-09-28 (jgrpp-0.73.3 + px-patch 2609.x)

> Commit penggabungan: jgrpp 94 commit (sampai `jgrpp-0.73.3`) + px-patch 113 commit (sampai setelah `pxp-2609.10`).

**Baru:** dari pulsexlb — transportasi kendaraan jalan (RoRo, lihat Fitur), 128 jenis kargo (CargoTypes berbasis Uint128, via XSLFI_CARGO_TYPES_128), batch perbaikan decouple/couple dan peningkatan jadwal; dari jgrpp — seret-letakkan order/klik ganda, kapal ujung ganda (XSLFI_DOUBLE_ENDED_SHIPS), serta banyak perbaikan umum.

**Penanganan konflik utama:** lapisan save pulsexlb berbasis sistem makro SLE_ lama sementara jrpm/jgrpp 0.73.3 memakai VarFileType/VarMemType + VarTypes; semua konflik saveload/ dan sl/ ditulis ulang dengan sistem modern dan dukungan U128 ditambahkan (VarFileType::U128=13, VarMemType::U128, SLE_UINT128); versi 367/368 milik jrpm tetap, versi baru upstream bergeser ke 369/370; kompatibilitas savegame upstream tetap lewat sub-chunk XSLFI_UPSTREAM_VERSION; CT_VEHICLES sebagai string, perbandingan grfid via GrfID, blok konsol autogroup + RORO_DEBUG_COMMANDS dipertahankan.

**Status:** build MinGW ninja lulus, artefak `build/openttd-jrpm.exe`; uji coba game baru lulus.

---

## Catatan penggabungan: 2026-10-07 (jgrpp 0.73.3+89 + px-patch 2610.3)

> Commit penggabungan: 23 commit jgrpp (setelah `jgrpp-0.73.3`, sampai `6318727b02`) + 55 commit px-patch (sampai `pxp-2610.3`, `4a4d0724b5`).

**Baru:** dari pulsexlb — feri kereta (kapal dapat diubah untuk mengangkut satu trainset penuh, kargo khusus RAIL/CT_RAILVEHICLES, pemuatan per gerbong), sistem hujan (weather.cpp: periode hujan acak, secara bertahap, raindrop overlay yang mengikuti zoom; opsi difficulty.rain; cheat sandbox memaksa cuaca; status cuaca disimpan bersama WTHR/XSLFI_WEATHER), keluar dengan arah sama setelah decouple sambil menunggu bagian lain, tampilan kapasitas tiap kargo hold dan tiap gerbong, pilihan papan kereta/kendaraan di opsi RoRo, hilangnya lag pathfinding saat coupling (validasi kargo diubah menjadi pre-check dengan backoff saat gagal), serta build Android dengan rilis APK; dari jgrpp — StringID menjadi strong type, penolakan nama perusahaan kembar yang benar, perhitungan ukuran pilihan widget, dan banyak refactoring serta perbaikan.

**Penanganan konflik utama:** hambatan terbesar dalam build adalah strong typing Label/StringID: semua label literal 4 karakter pada kode milik jrpm sendiri dikonversi ke konstruksi string (CT_RAILVEHICLES{"RAIL"}); perbandingan label tipe jalan di afterload.cpp menjadi bentuk RoadTypeLabel{"ROAD"}; bidang string pada AirTypeInfo kosong di airport.cpp memakai STR_NULL; cabang switch STR_CHEAT_RAIN di cheat_gui.cpp memakai .base(); tipe blok lama CH_TABLE pada blok cuaca baru WTHR menjadi ChunkType::Table; tabel cheat mempertahankan tipe VarMemType dan sentinel InflationCheat milik jrpm sekaligus menambahkan baris cheat hujan baru; SetStringTip(SPR_GOTO_LOCATION, …) di jrpm_watch_gui.cpp menjadi SetSpriteTip; train_cmd.cpp mempertahankan gate enable_decouple dan keluaran debug desync, sekaligus mengadopsi perbaikan upstream yang hanya membalik saat kepala kereta melewati percabangan (if (v->IsMovingFront())) beserta flag keluar arah sama; order_cmd.cpp mengadopsi semantik baru DrivingBackwards berbasis TCF_NO_DRIVING_CAB dengan mempertahankan variabel DecouplePart; deploy-docs.yml mempertahankan build VitePress dan deployment GitHub Pages, begitu juga README/.gitignore/.ottdrev-vc mempertahankan identitas jrpm; SL_UPSTREAM_VERSION tetap dipinned ke 368 (DoubleEndedShips upstream), static_assert di src/saveload/engine_sl.cpp lolos, dan versi 367/368 milik jrpm tidak berubah.

**Status:** build MinGW ninja lulus (-j3), artefak `build/openttd-jrpm.exe`; uji asap server khusus `-D` lulus — pembuatan peta → simpan → muat → simpan lagi → keluar tanpa assertion maupun crash SetupEngines, dan gate versi blok upstream berperilaku benar.
