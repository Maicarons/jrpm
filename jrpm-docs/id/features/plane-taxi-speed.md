---
title: Kecepatan Taksi Pesawat Dapat Diatur (Plane Taxi Speed)
---

# Kecepatan Taksi Pesawat Dapat Diatur

Berasal dari **embeddedt/OpenTTD-modded** (porting batch pertama).

## Fitur

Pengaturan game baru **`vehicle.plane_taxi_speed` (Kecepatan Taksi Pesawat)**, dapat menyesuaikan batas kecepatan pesawat saat meluncur di darat bandara secara independen:

- Rentang **1–8**, default **4** (= batas kecepatan taksi 50 unit versi asli)
- Semakin besar nilainya, semakin cepat meluncur (1 → 12.5, 8 → 100)
- **Berlaku independen** dari pengaturan kecepatan pesawat lainnya (`plane_speed` pengali global), tidak saling mengganggu

## Lokasi Pengaturan

Dalam game **Pengaturan → Pengaturan Ahli → Kendaraan (Vehicles)**, atau di file konfigurasi `openttd.cfg`:

```ini
[vehicle]
plane_taxi_speed = 4
```

## Catatan Implementasi

- Pengaturan dengan `SettingFlag::NoNetwork` (otoritas server, ditentukan oleh host saat online)
- Menggunakan gerbang fitur **XSLF** (`XSLFI_PLANE_TAXI_SPEED`, versi 1): Saat memuat arsip lama fitur ini tidak ada → pengaturan tetap pada nilai default 4, **tidak merusak arsip lama mana pun**
- Batas kecepatan taksi diterapkan saat status pesawat `AS_RUNNING` (meluncur di darat), mengambil `min(batas kecepatan tipe pesawat, batas kecepatan taksi)`

## File Terkait

- `src/aircraft_cmd.cpp` (konstanta baru `SPEED_LIMIT_TAXI` + logika batas kecepatan taksi)
- `src/table/settings/game_settings.ini` (definisi pengaturan)
- `src/settings_type.h` (`VehicleSettings::plane_taxi_speed`)
- `src/sl/extended_ver_sl.h/.cpp` (registrasi fitur XSLF)
- `src/lang/english.txt` (STR_CONFIG_SETTING_PLANE_TAXI_SPEED*)