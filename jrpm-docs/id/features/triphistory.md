---
title: Riwayat Perjalanan Kendaraan (Trip History)
---

# Riwayat Perjalanan Kendaraan (Trip History)

Berasal dari **embeddedt/OpenTTD-modded** (porting batch pertama), telah diadaptasi ke API string dan tanggal baru jrpm.

## Fitur

Setiap kendaraan akan mengingat data **10 perjalanan terakhir**, dapat dilihat di tombol **History (Riwayat)** baru di jendela detail kendaraan:

| Kolom | Arti |
|---|---|
| Received (Tiba) | Tanggal penerimaan barang perjalanan ini |
| Profit (Laba) | Laba perjalanan ini (negatif ditampilkan kuning) |
| % Change | Persentase perubahan laba dibandingkan perjalanan sebelumnya (hijau + / merah -) |
| TBT | Interval waktu sejak perjalanan sebelumnya (hari) |
| Change | Perubahan waktu dibandingkan perjalanan sebelumnya (hari) |
| Occupancy | Rata-rata tingkat okupansi penumpang/kargo perjalanan ini (%) |

Di bagian bawah jendela juga terdapat statistik ringkasan:

- **Total income for the last N trips**: Total laba N perjalanan + laba rata-rata per hari per perjalanan
- **Average trip length**: Rata-rata interval perjalanan (hari)
- **Improvement over last N trips**: Persentase perubahan laba secara keseluruhan

## Mekanisme Perekaman

- **AddValue**: Saat kendaraan menyelesaikan satu pengiriman (`CargoPayment` destruktor), catat laba, tanggal, okupansi, jarak antar stasiun
- **NewRound**: Saat kendaraan mencapai tujuan pertama sesuai jadwal, mulailah perjalanan baru
- **Okupansi**: Dikumpulkan saat kendaraan meninggalkan stasiun (menggunakan mekanisme `trip_occupancy` yang sudah ada di jrpm)

## Catatan Implementasi

- Data adalah **NOSAVE** (hanya runtime, tidak ditulis ke arsip), sehingga tidak perlu menaikkan versi arsip, arsip lama sepenuhnya kompatibel
- Kelas jendela baru `WindowClass::VehicleTripHistory`, ID jendela adalah ID kendaraan
- Jendela detail kendaraan (kereta/non-kereta) semuanya menambahkan tombol History di bilah judul
- Saat kendaraan dihapus, jendela riwayat otomatis ditutup

## File Terkait

- `src/triphistory.h` / `src/triphistory_cmd.cpp` / `src/triphistory_gui.cpp` (baru)
- `src/vehicle_base.h` (Vehicle baru bidang `trip_history`)
- `src/economy.cpp` (CargoPayment destruktor mencatat perjalanan)
- `src/timetable_cmd.cpp` (mencapai stasiun pertama memulai perjalanan baru)
- `src/vehicle_gui.cpp` / `src/widgets/vehicle_widget.h` (Tombol History)
- `src/lang/english.txt` (string STR_TRIP_HISTORY_*)