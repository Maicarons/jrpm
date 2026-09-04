---
title: "Unduhan Sumber Daya: Multi-cermin + Unduhan Paralel"
---

## Status Saat Ini (Kesimpulan Riset)

| Item | Status Saat Ini |
|---|---|
| Server Konten (protokol metadata) | `src/network/core/config.cpp` `NetworkContentServerConnectionString()`: Variabel lingkungan `OTTD_CONTENT_SERVER_CS`, default `content.openttd.org` (port TCP 3978) |
| Cermin Unduhan | `NetworkContentMirrorUriString()`: Variabel lingkungan `OTTD_CONTENT_MIRROR_URI`, default `https://binaries.openttd.org/bananas` |
| Metode Unduhan | `network_content.cpp` `DownloadSelectedContentHTTP()`: **Satu** permintaan POST untuk semua ID konten → cermin mengembalikan daftar header file (`id,type,filesize,url` per baris) → **serial satu per satu** mengunduh setiap file (satu koneksi GET per file) → `AfterDownload()` gunzip + tar ekstrak |
| Model Thread | Lapisan HTTP (WinHttp) diproses di thread latar belakang, tetapi **unduhan file diantrekan secara serial**; tidak ada daftar cermin, tidak ada pengaturan dalam game, tidak ada paralel |

## Implementasi Fitur Ini

### 1. Pengaturan Baru (`network_settings.ini` + `settings_type.h`)

- `network.content_server` (SLE_STR, default kosong = menggunakan sumber resmi)
- `network.content_mirrors` (SLE_STR, beberapa URI cermin dipisahkan koma, default kosong = cermin resmi)

Prioritas: Variabel lingkungan > Pengaturan game > Default resmi.

### 2. Parsing Konfigurasi (`src/network/core/config.cpp/h`)

- `NetworkContentServerConnectionString()` membaca pengaturan;
- Baru `NetworkContentMirrorUris()` mem-parsing daftar yang dipisahkan koma (hapus spasi, lewati item kosong, cadangan sumber resmi);
- `NetworkContentMirrorUriString()` diubah menjadi mengembalikan yang pertama dalam daftar.

### 3. Unduhan Paralel (`src/network/network_content.h/.cpp`)

- Baru `ContentFileDownload` (satu file yang akan diunduh: id/type/filesize/url/filename);
- Baru `ContentDownloadSession : HTTPCallback` (status unduhan independen dan callback untuk setiap file; `IsCancelled` menghubungkan handler);
- `DownloadSelectedContentHTTP()`: POST ke `mirrors[mirror_index]` → `ParseResponseHeaders()` mem-parsing semua header file sekaligus → `StartDownloadSessions()` memulai maksimal **4 sesi paralel** (`CONTENT_DOWNLOAD_PARALLEL`), setiap sesi setelah selesai secara otomatis mengambil file berikutnya yang akan diunduh;
- Rantai percobaan ulang gagal: Sesi gagal/permintaan cermin gagal → cermin berikutnya meminta ulang → semua cermin gagal → kembali ke protokol TCP lama (`DownloadSelectedContentFallback`);
- Penutupan yang rapi: Saat gagal/dibatalkan, set `download_cancelled`, tunggu semua sesi yang sedang berjalan selesai (`OnAllSessionsDone`) sebelum mencoba ulang atau kembali, hindari callback yang menggantung;
- `ResetMirrorIndex()`: Unduhan baru mulai dari cermin pertama (dipanggil saat dipicu GUI).

## File Terkait

- `src/table/settings/network_settings.ini` (2 pengaturan baru)
- `src/settings_type.h` (2 bidang baru di `NetworkSettings`)
- `src/network/core/config.cpp` / `config.h` (parsing daftar cermin)
- `src/network/network_content.h` / `.cpp` (sesi paralel)
- `src/network/network_content_gui.cpp` (reset indeks cermin sebelum unduhan)

## Poin Verifikasi

1. Setelah `network.content_mirrors` diisi beberapa URI (dipisahkan koma), unduhan konten harus berfungsi dan mengunduh beberapa file secara **paralel** (amati progres unduhan/tangkap beberapa koneksi bersamaan);
2. Jaringan terputus/cermin error harus secara otomatis mencoba cermin berikutnya, akhirnya kembali ke protokol lama;
3. Pembatalan saat unduhan tidak boleh meninggalkan `.tar.gz` setengah jadi;
4. Perlu diverifikasi kompilasi di mesin nyata (repo ini tidak memiliki lingkungan build, kode belum dikompilasi).

## Keterbatasan yang Diketahui

- Paralelisme dikodekan keras sebagai 4 (`CONTENT_DOWNLOAD_PARALLEL`), nantinya dapat dijadikan pengaturan;
- Protokol cermin bergantung pada format API bananas resmi (POST daftar id mengembalikan daftar header file).