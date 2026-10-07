---
title: Feri Kereta dan Hujan
---

# Feri Kereta dan Hujan

> Sumber: pulsexlb/OpenTTD-patches `px-patch` (batch 2026-10, `pxp-2610.1` – `pxp-2610.3`), digabung ke jrpm melalui git merge.

Batch ini menghadirkan dua fitur besar baru: **feri kereta** (kapal mengangkut satu trainset penuh) dan **sistem hujan** (dengan penggelapan dunia dan lapisan tetesan air).

## Feri kereta

Selain Transportasi Kendaraan Jalan (RoRo) yang sudah ada, tempat mobil dimuat di kereta, kapal, dan pesawat, kapal sekarang dapat mengangkut **kereta penuh**.

### Hubungan dengan RoRo

| | Transportasi kendaraan jalan (VEHC) | Transportasi kereta (RAIL) |
|---|---|---|
| Pengangkut | kereta, kapal, pesawat | kapal |
| Benda yang diangkut | kendaraan jalan | kereta (lokomotif dan gerbong) |
| Label kargo | `VEHC` | `RAIL` |
| Slot kargo | `NUM_CARGO - 1` | `NUM_CARGO - 2` |

Kedua slot kargo berada di luar 64 slot NewGRF, sehingga tidak pernah saling bertabrakan.

### Pemuatan

- Kapal memperoleh kemampuan mengangkut kereta penuh dengan **dimodifikasi secara manual** menjadi kargo "Kendaraan (Kereta)";
- **Pemuatan per gerbong**: sebuah kereta boleh tersebar di beberapa kargo hold, tidak harus seluruhnya muat dalam satu hold. Ini menyelesaikan masalah kereta panjang yang melebihi kapasitas satu hold;
- **Pemilih papan** pada opsi transportasi kini menawarkan papan kereta dan kendaraan secara terpisah, sehingga tiap jenis transportasi dapat mendeklarasikan tujuannya sendiri;
- Bilah status tidak lagi merinci kendaraan yang diangkut, sehingga ruang yang dibebaskan dapat dipakai untuk informasi operasi yang lebih penting.

### Tampilan kapasitas

- **Jendela informasi kapal** menampilkan berat kargo yang dapat ditampung setiap kargo hold;
- **Jendela informasi kereta** menampilkan kapasitas setiap gerbong.

### Perbaikan terkait

Batch ini juga memperbaiki sejumlah cacat pemuatan, pembongkaran, dan reservasi peron:

- kapal salah menilai tipe rel sebagai tidak kompatibel saat membongkar kereta, sehingga kereta tidak bisa turun;
- reservasi peron tidak dibersihkan saat kereta naik, disertai kesalahan struktur dan reservasi saat turun;
- sisa reservasi peron membuat kereta tidak bisa membongkar ke peron lagi;
- beberapa orientasi peron menolak pembongkaran kereta;
- batas jumlah gerbong tidak ditegakkan pada kendaraan yang dimodifikasi menjadi pengangkut.

## Sistem hujan

Simulasi cuaca yang murni kosmetik: hanya memengaruhi tampilan dan tidak menyentuh logika permainan maupun nilai ekonomi.

### Tampilan cuaca

- **Periode hujan acak**: status cuaca dikendalikan generator angka acak deterministik yang di-seed dari seed pembuatan peta, sehingga semua pemain dan server melihat cuaca yang persis sama;
- **Penggelapan dunia bertahap**: dunia menjadi gelap perlahan saat hujan dan kembali terang ketika berhenti. Penggelapan melewati `RAIN_SHADE_LEVELS` tingkat dan dibaca saat menggambar viewport;
- **Lapisan tetesan air**: lapisan hujan layar penuh yang **menyesua diri dengan zoom** dan memiliki **getaran acak** agar tetesannya tidak terlihat seperti tekstur diam.

### Opsi dan cheat

- **Opsi kesulitan** `difficulty.rain`: menentukan saat membuat permainan baru apakah penggelapan dunia saat hujan diaktifkan;
- **Cheat sandbox** "Cuaca": siklus tiga keadaan — otomatis / paksa hujan / paksa cerah.

### Simpanan permainan

Status cuaca (apakah sedang hujan, seed generator, awal periode hujan saat ini) dan cheat cuaca sandbox **disimpan bersama permainan**; setelah dimuat, penggelapan langsung melompat ke cuaca saat ini alih-alih muncul kembali secara bertahap.

- Chunk: `WTHR`;
- Flag fitur: `XSLFI_WEATHER`;
- Kode: `src/weather.cpp`, `src/weather.h`, `src/sl/weather_sl.cpp`.

## Kode terkait

- Feri kereta dan kapasitas: `src/roadveh_transport.cpp`, `src/cargo_type.h` (`CT_RAILVEHICLES`), `src/table/cargo_const.h`
- Efek visual hujan: `src/weather.cpp`, `src/blitter/32bpp_anim.cpp`, `src/blitter/40bpp_anim.cpp`, `src/viewport.cpp`
- Simpanan cuaca: `src/sl/weather_sl.cpp`, `src/sl/extended_ver_sl.cpp` (`XSLFI_WEATHER`)
- Setelan dan cheat: `src/table/settings/difficulty_settings.ini` (`difficulty.rain`), `src/cheat_gui.cpp`, `src/cheat_type.h`
