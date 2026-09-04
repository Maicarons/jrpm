---
title: Ikhtisar Fitur
---

# Ikhtisar Fitur

jrpm = semua fitur jgrpp + pulsexlb (kopling lokomotif + bandara modular) + modded (riwayat perjalanan + kecepatan taksi) + pinjaman cmclient (peningkatan UI multiplayer) + fitur eksklusif jrpm.

## Fitur Eksklusif jrpm

| # | Fitur | Dokumentasi | Status |
|---|---|---|---|
| 1 | Unduhan Sumber Daya: Multi-cermin + Paralel | [Unduhan Sumber Daya](./01-resource-download) | ✅ Sudah diimplementasikan (konkurensi dapat dikonfigurasi) |
| 2 | Riset Batas Server | [Riset Batas Server](./02-server-caps) | 📖 Kesimpulan riset (batasan struktural) |
| 3 | Pengelompokan Kendaraan Otomatis | [Pengelompokan Kendaraan Otomatis](./03-vehicle-autogroup) | ✅ Sudah diimplementasikan |
| 4 | AI Sadar Permainan Penuh | [AI Sadar Permainan Penuh](./05-global-ai) | ✅ Sudah diimplementasikan |

> Catatan: F4 asli "Tooltip Harga Bangunan" telah dihapus sesuai permintaan (komit `96ebfb75`).

## Porting modded (Batch Pertama)

| Fitur | Dokumentasi | Status |
|---|---|---|
| Riwayat Perjalanan Kendaraan | [Riwayat Perjalanan](./triphistory) | ✅ Sudah diimplementasikan |
| Kecepatan Taksi Pesawat Dapat Diatur | [Kecepatan Taksi](./plane-taxi-speed) | ✅ Sudah diimplementasikan |

## Pinjaman cmclient (Batch Kedua~Kelima)

| Fitur | Dokumentasi | Status |
|---|---|---|
| Bookmark Lokasi / Detail Kargo / Tonton | [Peningkatan UI Multiplayer](./ui-enhancements) | ✅ Sudah diimplementasikan (Batch Kedua) |
| Sistem Sorotan Objek | [Sorotan + Cetak Biru](./highlight-blueprint-plan) | ✅ Sudah diimplementasikan (Batch Ketiga, 6 komit) |
| Sistem Cetak Biru (salin/putar/slot/bangun ulang) | [Sorotan + Cetak Biru](./highlight-blueprint-plan) | ✅ Sudah diimplementasikan (Batch Ketiga) |
| Zonasi Kota + growth_tiles arsip | [Zonasi Kota](./town-zoning) | ✅ Sudah diimplementasikan (Batch Keempat) |
| Perekaman & Pemutaran Ulang Perintah | [Pemutaran Ulang Perintah](./command-replay) | ✅ Sudah diimplementasikan (Batch Kelima) |

> **② Lapisan objek perintah** (2251 baris kode generasi cmclient): Terbukti dari praktik porting **dapat dilewati secara keseluruhan**——sorotan menggunakan `CMD_ERROR` untuk estimasi biaya, cetak biru menggunakan penutupan perintah, pemutaran ulang menggunakan serialisasi perintah asli jrpm, oleh karena itu tidak di-porting.

## Fitur Gabungan (dari pulsexlb)

| Fitur | Dokumentasi | Deskripsi |
|---|---|---|
| Kopling Lokomotif (decouple) | [Kopling Lokomotif](./decouple) | Sistem lengkap pemisahan/penggandengan kereta |
| Bandara Modular (multitile-airport) | [Bandara Modular](./multitile-airport) | Restrukturisasi sistem bandara multi-ubin |

## Warisan Lengkap

- **Semua fitur jgrpp**: Peningkatan sinyal (multi-sinyal/sinyal terprogram/slot dan penghitung), scheduled dispatch, tracerestrict, penggantian templat, jendela pembelian terpisah lokomotif/gerbong, pengereman realistis, peningkatan jalan satu arah, keselamatan perlintasan sebidang, dll.;
- **Kernel OpenTTD 16.0**: Kompatibilitas penuh ekosistem NewGRF/script/arsip.

## Temukan Pengaturan dengan Cepat

| Fitur | Pengaturan |
|---|---|
| Konkurensi unduhan paralel | `network.content_download_parallel` (1-8) |
| Daftar cermin unduhan | `network.content_mirrors` |
| Server konten | `network.content_server` |
| Modifikasi tata letak bandara | `station.allow_modify_airports` |
| Tipe penerbangan default | `gui.default_air_type` |
| Sakelar kesadaran AI permainan | `game.script.allow_global_ai_access` |
| Kecepatan taksi pesawat | `vehicle.plane_taxi_speed` (1-8, default 4) |

## Temukan Perintah Konsol dengan Cepat

| Fitur | Perintah |
|---|---|
| Pengelompokan kendaraan otomatis | `autogroup train\|road\|ship\|aircraft` |
| Tambah AI sadar permainan penuh | `start_ai GlobalAI` |
| Bookmark lokasi | `savelocation <1-9>` / `gotolocation <1-9>` |
| Detail kargo perusahaan | `company_cargo <company_id>` |
| Tonton perusahaan | `watch <company_id>` |
| Salin/putar/simpan/muat/bangun cetak biru | `blueprint_copy` / `blueprint_rotate` / `blueprint_save <0-15>` / `blueprint_load <0-15>` / `blueprint_build` |
| Rekam/putar ulang perintah | `cmdrecord [start [file]]` / `cmdrecord stop` / `cmdreplay <file>` |
| Kecepatan game/statistik/ekspor/tanam pohon | `cmgamespeed [n]` / `cmgamestats` / `cmexport` / `cmtreemap <file>`