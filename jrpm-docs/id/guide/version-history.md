---
title: Riwayat Versi
---

# Riwayat Versi

## jrpm-0.1.0 (2026-08-14) Porting cmclient Selesai

Berdasarkan 0.1.0 pertama, menyelesaikan porting lima batch fitur cmclient (seluruh seri modded + cmclient):

### Fitur Baru (Porting cmclient / modded)

- **Batch pertama modded**: Riwayat perjalanan kendaraan (`b89f93f9`) + Kecepatan taksi pesawat dapat diatur (`b89f93f9`);
- **Batch kedua cmclient**: Bookmark lokasi / Detail kargo / Tonton (`089480b3`);
- **Batch nol cmclient**: Perintah konsol `cmgamespeed` / `cmgamestats` / `cmexport` / `cmtreemap` (`1fd94d12`);
- **Batch ketiga cmclient ① Sorotan**: Pratinjau bangunan objek (stasiun/rel/gudang/bandara), terhubung ke pipeline rendering viewport dan alat bangun (6 komit, `d97aa38a` → `1957bf45`);
- **Batch ketiga cmclient ③ Cetak Biru**: Pilih salin/putar/16 slot/bangun ulang (`8e08ca6b`);
- **Batch keempat cmclient ④ Zonasi Kota**: Zona Tz + pewarnaan growth_tiles + blok arsip GRWT (`9e3f95a2`);
- **Batch kelima cmclient ⑤ Pemutaran Ulang Perintah**: `cmdrecord` / `cmdreplay` (`f113acce28`);
- **Perbaikan konvensi parameter konsol**: Semua perintah jrpm menggunakan parameter mulai argv[1] (argv[0] adalah nama perintah).

### Kesimpulan Arsitektur

- **② Lapisan objek perintah tidak di-porting**: Sorotan/cetak biru/pemutaran ulang masing-masing menggunakan `CMD_ERROR`, penutupan perintah, serialisasi perintah asli jrpm untuk dilewati, tanpa memerlukan 2251 baris kode generasi cmclient.

### Kompatibilitas Arsip

- Data growth_tiles disimpan di chunk `GRWT` independen, arsip lama (tanpa chunk ini) dimuat dengan kompatibilitas penuh;
- File rekaman perintah (`.jrcm`) adalah format privat jrpm, kompatibilitas antar versi tidak dijamin.

## jrpm-0.1.0 (2026-08-14)

Versi jrpm pertama, berdasarkan jgrpp 0.73.1 + penggabungan pulsexlb px-patch, dan porting fitur modded / cmclient.

### Perubahan

- **Menggabungkan pulsexlb px-patch (152 komit)**:
  - Kopling Lokomotif (decouple): Perintah pemisahan/penggandengan, transfer tanda, batas panjang kopling/kecepatan, dua lokomotif, kopling NewGRF, navigasi kopling, penjadwalan independen setelah decouple;
  - Bandara Modular (multitile-airport): Sistem tipe air, penjadwalan udara PBS, navigasi udara YAPF, modifikasi tata letak bandara (`allow_modify_airports`);
  - Versi arsip baru `SLV_MULTITILE_AIRPORTS` / `SLV_ORDER_DECOUPLE`.
- **Ganti nama versi**: `openttd-jrpm` / `jrpm-0.1.0` (nama file eksekusi dan string revisi).
- **Kompatibilitas multi-versi server**: Server jrpm menerima klien jrpm / jgrpp asli / pulsexlb.
- **Fitur eksklusif jrpm**:
  - Unduhan sumber daya multi-cermin + paralel file (konkurensi dapat dikonfigurasi);
  - Pengelompokan kendaraan otomatis berdasarkan pesanan bersama (tombol jendela + perintah `autogroup`);
  - AI sadar permainan penuh (API `AIGlobal` + contoh GlobalAI, dikontrol sakelar pengaturan);
  - Tooltip Harga Bangunan (telah dihapus sesuai permintaan, komit `96ebfb75`).
- **Porting modded (Batch Pertama)**:
  - Riwayat perjalanan kendaraan (10 perjalanan terakhir: laba/tingkat okupansi/durasi, tombol History di jendela detail kendaraan);
  - Kecepatan taksi pesawat dapat diatur (`vehicle.plane_taxi_speed`, gerbang fitur XSLF, kompatibel dengan arsip lama).
- **Pinjaman cmclient (Batch Kedua)**:
  - Bookmark lokasi (`savelocation` / `gotolocation`, 9 slot);
  - Jendela detail kargo perusahaan (`company_cargo`);
  - Bantuan tonton (`watch <company_id>`).

### Kompatibilitas Arsip

- Mewarisi konvensi jgrpp: Dapat memuat arsip trunk (hingga versi yang baru digabungkan);
- Arsip jrpm (berisi data bandara multi-ubin/perintah decouple) **tidak dijamin** dapat dibaca bersama dengan arsip jgrpp lama;
- Nomor versi arsip `SAVEGAME_VERSION` konsisten dengan pulsexlb (`SLV_CUSTOM_SUBSIDY_DURATION`);
- Fitur baru semuanya menggunakan gerbang fitur ekstensi XSLF atau NOSAVE, tidak merusak arsip lama.

## Versi Hulu

- **jgrpp 0.73.1**: Basis proyek ini (kernel OpenTTD 16.0 + semua fitur JGR).
- **pulsexlb px-patch 2608.3**: Sumber kopling lokomotif dan bandara modular.
- **embeddedt/OpenTTD-modded (era 0.59.1)**: Sumber riwayat perjalanan dan kecepatan taksi.
- **citymania-org/cmclient (vanilla 15.3)**: Sumber pinjaman bookmark lokasi/detail kargo/tonton.

## Peta Jalan

- [x] Verifikasi build nyata dan perbaikan kesalahan kompilasi pertama
- [x] Riwayat perjalanan + Kecepatan taksi (batch pertama modded)
- [x] Bookmark lokasi + Detail kargo + Tonton (batch kedua cmclient)
- [ ] Sistem sorotan + Sistem cetak biru (batch ketiga cmclient, lihat [Peta Jalan](../features/highlight-blueprint-plan))
- [ ] Thread pool lapisan transportasi HTTP + Unduhan chunk Range (percepatan file besar tunggal)
- [ ] Optimasi kompresi perbandingan pengiriman peta
- [ ] Statistik pendapatan per kargo (memerlukan ekstensi format arsip)
- [ ] (Jangka panjang) Integrasi runtime server Rust (referensi Openttd-Cluster)