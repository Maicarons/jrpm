---
title: Zonasi Kota (Town Zoning)
---

# Zonasi Kota (Town Zoning)

Porting zonasi kota cmclient (batch keempat, komit `9e3f95a2`). Basis jrpm (pulsexlb) sendiri sudah memiliki satu set sistem zoning (toolbar + pipeline rendering + menu), batch ini menambahkan mode evaluasi unik cmclient dan **arsip growth_tiles** di atasnya.

## Mode Baru (dropdown toolbar zoning)

| Mode | Deskripsi | Pewarnaan |
|---|---|---|
| **Town zones (Tz)** | Zona konsentris kota, menggunakan kembali `squared_town_zone_radius` | Pinggiran Tz0=biru muda / Tz1=merah / Tz2=kuning / Tz3=hijau / Tz4 pusat kota=putih |
| **Town growth tiles** | Jejak pembangunan dan penghancuran rumah bulan ini/bulan lalu | Rumah baru=hijau / Penghancuran=biru muda / Bangun ulang=putih / Lewati pertumbuhan=oranye / Lewati bangun rumah=kuning / Penghancuran rumah server=merah |

## Lapisan Data growth_tiles (`cm_town_growth.cpp/.h`)

- Dua peta bulan bergulir: `TileIndex → TownGrowthTileState` (bulan saat ini / bulan lalu)
- Kait peristiwa:
  - `BuildTownHouse` → `NEW_HOUSE` (jika bulan lalu dihancurkan, tingkatkan menjadi `RH_REBUILT`)
  - `ClearTownHouse` → `RH_REMOVED`
  - `TownsMonthlyLoop` → rotasi bulanan (bulan lalu = bulan ini, bulan ini dikosongkan)
- **Persistensi arsip**: Chunk savegame `GRWT` baru (`misc_sl.cpp`), diserialisasi sebagai daftar pasangan `{tile, state}`; arsip lama tidak mengandung chunk ini, pemuatan sepenuhnya kompatibel (telah diverifikasi arsip→baca bolak-balik).

## Cara Membuka

Menu toolbar → Peta (Zoning) → buka toolbar zoning, pilih mode evaluasi di dropdown lingkaran dalam dan luar.

## Catatan Pemotongan

Mode khusus server CityBuilder cmclient (Area penerimaan CB / Batas kota CB) dan bidang ekstensi `ext::Town` (iklan, dana, statistik kargo) adalah khusus untuk permainan server, tidak di-porting.