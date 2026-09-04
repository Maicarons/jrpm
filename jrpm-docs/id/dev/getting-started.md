---
title: Panduan Pengembangan
---

# Panduan Pengembangan

Dokumen ini menjelaskan cara melanjutkan pengembangan di jrpm. Semua pola berasal dari kode yang benar-benar diimplementasikan dalam proyek ini (dapat dibandingkan dengan komit di `git log`).

## Ikhtisar Organisasi Kode

| Direktori | Tanggung Jawab |
|---|---|
| `src/` | Semua kode sumber C++ (tingkat atas dinamai berdasarkan sistem: `rail_cmd.cpp`, `group_gui.cpp`…) |
| `src/network/` | Jaringan (server/klien/UDP/HTTP/unduhan konten) |
| `src/script/` | Kerangka script (Squirrel); `api/` untuk kelas API AI/GS |
| `src/table/settings/*.ini` | **Sumber definisi pengaturan** (settingsgen menghasilkan kode) |
| `src/lang/english.txt` | Definisi string (strgen menghasilkan) |
| `src/sl/saveload_common.h` | Enum versi arsip (SLV) |
| `bin/ai/` | Script AI (`GlobalAI` sebagai contoh) |

## Pola Pengembangan Umum

- [Menambah perintah game baru](./add-command): Enum perintah + `DEF_CMD_TUPLE_NT` + handler + GUI/entri konsol
- [Menambah API script baru](./add-script-api): `script_*.hpp/.cpp` (registrasi otomatis binding Squirrel)
- [Menambah item pengaturan baru](./add-setting): `.ini` + bidang `settings_type.h` + string

## Build

```bash
cmake -B build ..
cmake --build build -j
```

## Catatan Pengembangan

1. **Encoding**: Kode sumber/dokumen yang mengandung bahasa Mandarin setelah perubahan perhatikan UTF-8 (alat Write/Edit mesin ini pernah menghasilkan encoding GBK, dapat diperbaiki dengan `jgrpp-features/_fix_utf8.py`);
2. **Pesan komit**: Disarankan bahasa Inggris (menghindari masalah encoding terminal);
3. **Penyisipan di tengah enum perintah** akan menggeser ID perintah berikutnya——versi biner lama dan baru tidak akan konsisten saat online, ini normal dalam fork, upgrade harus sinkron;
4. **Versi arsip**: Saat mengubah struktur arsip, tambah entri `SLV_*` baru di `src/sl/saveload_common.h` dan perbarui `SAVEGAME_VERSION`;
5. **File baru**: `.cpp` harus ditambahkan ke daftar sumber CMakeLists.txt yang sesuai (`src/CMakeLists.txt` atau subdirektori/`script/api/CMakeLists.txt`); `script_*.hpp` ditemukan secara otomatis oleh `file(GLOB)`, tidak perlu registrasi;
6. **String baru**: Tambahkan di `src/lang/english.txt` (bahasa lain default kembali ke Inggris);
7. **Kompatibilitas NewGRF/arsip**: Ubah `_openttd_content_version` / nomor versi NewGRF dengan hati-hati (mempengaruhi kompatibilitas online dan konten).

## Saran Regresi (Penting)

Cabang saat ini **belum dikompilasi di mesin nyata**, setelah build pertama disarankan regresi dalam urutan ini:
1. Dasar: `openttd-jrpm -v` menampilkan `jrpm-0.1.0`;
2. Arsip: Buka game single player jalankan 1-2 tahun;
3. Online: Klien jrpm saling terhubung; klien jgrpp/pxp bergabung;
4. Fitur baru: Kopling lokomotif (decouple/couple/mundur), Bandara modular (modifikasi tata letak), unduhan paralel, pengelompokan otomatis, tooltip bangunan, GlobalAI;
5. Regresi fitur lama: Sinyal, tracerestrict, scheduled dispatch, penggantian templat.