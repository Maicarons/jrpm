---
title: Transportasi Kendaraan Jalan (RoRo)
---

# Transportasi Kendaraan Jalan (RoRo: Road-vehicle on Road-vehicle)

> Sumber: pulsexlb/OpenTTD-patches `px-patch` (batch September 2026), digabungkan ke jrpm melalui git merge.

## Ikhtisar

"Transportasi kendaraan jalan" memungkinkan kereta, kapal, dan pesawat **mengangkut kendaraan jalan secara langsung**:

- Kendaraan jalan tidak perlu lagi menyetel ke mana-mana sendiri: mereka bisa "menumpang" — pengangkut membawa mereka ke stasiun yang jauh, lalu mereka turun ke jalan dan melanjutkan sendiri;
- Pengangkut (kereta/kapal/pesawat) mendapat kemampuan membawa kendaraan jalan setelah **direfit ke kargo "Vehicles (Road)"** (kargo khusus `VEHC`, yang memakai ruang kargo);
- Di sisi kendaraan jalan, penanda order "**Menunggu diangkut**" dan "**Turun di sini**" berpasangan dengan opsi "memuat kendaraan jalan" / "membongkar kendaraan jalan" milik pengangkut.

## Cara pakai

### Sisi pengangkut (kereta/kapal/pesawat)

1. Refit kereta di depot **secara manual** ke kargo "Vehicles (Road)" — menjadi pengangkut hanya bisa lewat refit manual; refit lewat order tidak berlaku;
2. Aktifkan "**Muat kendaraan jalan**" pada order stasiun: kereta mengambil kendaraan yang menunggu di stasiun tersebut;
3. Opsi pasangan tambahan:
   - "Menunggu muatan" (berangkat hanya saat penuh dimuat);
   - "Cocokkan tujuan": hanya memuat kendaraan yang stasiun turunnya sama dengan perhentian berikutnya si pengangkut;
   - "Turunkan semua kendaraan di sini": turunkan semuanya, abaikan stasiun yang dinyatakan masing-masing.

### Sisi kendaraan jalan

1. Atur "**Menunggu diangkut**" pada order stasiun: kendaraan berhenti di sana menunggu pengangkut;
2. Atur "**Turun di sini**": kendaraan turun dari pengangkut di stasiun ini;
3. Kedua opsi saling eksklusif (satu per order);
4. Saat turun, kendaraan menjalankan pencarian jalur untuk memilih peron terbaik.

## Detail dan aturan

- **Slot kargo khusus**: transportasi kendaraan jalan memakai slot kargo 128 (`NUM_CARGO - 1`), di luar 64 slot yang bisa didefinisikan NewGRF; total jenis kargo diperluas dari 64 menjadi **128**;
- **Deteksi pengangkut khusus**: bila semua bagian kendaraan direfit ke "Vehicles (Road)", tombol order default menampilkan transportasi kendaraan jalan; kendaraan dengan kargo normal menampilkan kargo normal;
- **Pengaturan bagian pengangkut**: `vehicle.rv_transport_carrier_parts` menentukan bagian mana yang bisa membawa kendaraan jalan;
- **Pemuatan lintas perusahaan**: opsional mengizinkan muat/bongkar kendaraan perusahaan lain dengan penyelesaian biaya otomatis;
- **Peringatan "diangkut terlalu lama"**: peringatan sekali saat kendaraan diangkut terlalu lama;
- **Daftar order buatan pemain**: daftar order bersama/mandiri juga mendukung penanda transportasi kendaraan jalan.

## Kompatibilitas savegame

- Status menunggu/diangkut dan penanda order disimpan;
- Savegame lama (tanpa penanda XSLFI_CARGO_TYPES_128) dibaca dengan 64 slot kargo dan tetap kompatibel.

## Perintah konsol debug (mati secara default)

Keluarga perintah `rvtransport` hanya dikompilasi dengan opsi CMake `RORO_DEBUG_COMMANDS=ON`, untuk uji regresi.

## Kode terkait

- Inti: `src/roadveh_transport.h`, `src/cargo_type.h` (kargo VEHC)
- Order: `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_base.h` (OrderExtraInfo)
- Pemuatan kereta: `src/train_cmd.cpp`, `src/station_cmd.cpp`
- Perluasan kargo: `src/sl/station_sl.cpp`, `src/sl/company_sl.cpp` (XSLFI_CARGO_TYPES_128)
