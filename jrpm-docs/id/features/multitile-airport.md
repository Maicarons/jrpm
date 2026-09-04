---
title: Bandara Modular (multitile-airport)
---

# Bandara Modular (multitile-airport)

> Sumber: Cabang fitur `jgrpp-multitile-airport` dari pulsexlb/OpenTTD-patches, digabungkan ke jrpm melalui git merge.

## Pengenalan Fitur

Restrukturisasi bandara modular multi-ubin (multitile): Mengubah bandara dari "tipe bandara tetap" menjadi **sistem multi-ubin dengan tata letak yang dapat dimodifikasi secara bebas**:

- **Bandara multi-ubin**: Bandara terdiri dari beberapa ubin fungsional (landasan pacu, taxiway, apron, terminal, helipad), dapat dikombinasikan menjadi tata letak apa pun;
- **Modifikasi tata letak bandara**: Setelah mengaktifkan `station.allow_modify_airports`, ubin dapat ditambah/dihapus/disesuaikan di bandara yang sudah ada;
- **Sistem tipe air**: Baru `air.h`/`air_type.h`/`newgrf_airtype.*`——Mengabstraksi tipe penerbangan (sayap tetap/heli, dll.) menjadi sistem air type yang dapat diperluas, NewGRF dapat mendefinisikan tipe penerbangan baru dan sprite;
- **Penjadwalan udara PBS**: `pbs_air.*`——Versi udara dari okupansi landasan pacu/taxiway dan reservasi sinyal, mendukung beberapa pesawat meluncur bersamaan;
- **Navigasi udara YAPF**: Perencanaan jalur pesawat di darat (meluncur/menunggu) dan di udara menggunakan sistem YAPF.

## Kemampuan Inti

| Kemampuan | Deskripsi |
|---|---|
| Modifikasi tata letak bandara | `station.allow_modify_airports` (**default aktif**; setelah aktif, bandara yang sudah ada dapat diubah) |
| Tipe penerbangan default | `gui.default_air_type` |
| Sprite bandara multi-ubin | openttd.grf yang dibangun ulang (sprite air type), perbaikan sprite transparan |
| Perilaku pesawat | Okupasi landasan pacu, belok saat meluncur, antrian lepas landas/mendarat, helipad, penggambaran pesawat luar angkasa |
| Kompatibilitas NewGRF | Pemuatan sprite airtype, callback NewGRF bandara |
| Arsip | Versi arsip `SLV_MULTITILE_AIRPORTS` |

## Cara Penggunaan

1. Aktifkan `station.allow_modify_airports` di pengaturan game;
2. Setelah membangun bandara, gunakan alat modifikasi bandara untuk menyesuaikan tata letak landasan pacu/tempat parkir/terminal;
3. Pilih tipe penerbangan default di `gui.default_air_type`;
4. Gunakan air type kustom dengan NewGRF penerbangan.

## Kode Terkait

- Tipe penerbangan: `src/air.h`, `src/air_type.h`, `src/newgrf_airtype.*`
- Penjadwalan udara: `src/pbs_air.*`
- Perintah pesawat/bandara: `src/aircraft_cmd.cpp` (restrukturisasi 3600 baris), `src/airport_cmd.cpp`, `src/airport_gui.cpp`
- Navigasi: `src/pathfinder/yapf` (bagian penerbangan)
- Arsip: `src/sl/saveload_common.h` (`SLV_MULTITILE_AIRPORTS`)

## Catatan

- Fitur ini adalah restrukturisasi skala besar dari sistem penerbangan (restrukturisasi aircraft_cmd.cpp 3600+ baris), **disarankan untuk regresi utama setelah kompilasi di mesin nyata**: pembelian/lepas landas/pendaratan pesawat, okupasi landasan pacu, GUI bandara, pemuatan arsip;
- Tipe lama yang tidak memiliki referensi di workspace telah dihapus saat penggabungan (`VehicleAirFlags`, `AirportMovingDataFlag`), dikonfirmasi tidak ada file lain yang mereferensi;
- Untuk bandara yang sudah ada di arsip yang perlu dimodifikasi, cadangkan arsip terlebih dahulu.