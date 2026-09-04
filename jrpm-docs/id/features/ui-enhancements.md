---
title: Peningkatan UI Multiplayer (Bookmark Lokasi / Detail Kargo / Tonton)
---

# Peningkatan UI Multiplayer

Porting batch kedua dari **citymania-org/cmclient**, diimplementasikan dalam bentuk **perintah konsol** (tidak bergantung pada infrastruktur hotkey/toolbar cmclient, stabil dan dapat di-script).

## Bookmark Lokasi (Viewport Locations)

Menyimpan/memulihkan posisi dan zoom viewport utama (9 slot), cocok untuk berpindah cepat antara pabrik, stasiun, dan area lawan saat online.

```
savelocation <1-9>    # Simpan posisi viewport+zoom saat ini
gotolocation <1-9>    # Lompat ke posisi yang disimpan
```

## Detail Kargo Perusahaan (Company Cargo Details)

Jendela statistik yang mencantumkan **volume pengiriman + pendapatan** perusahaan berdasarkan kargo, dapat beralih antara periode **Total / Bulan Lalu**.

```
company_cargo <company_id>    # Buka jendela detail kargo
```

- Jendela mencantumkan volume, pendapatan untuk setiap jenis kargo standar + total di bagian bawah (sama persis dengan cmclient)
- Klik header "Cargo" untuk beralih antara periode total/bulan lalu
- Pendapatan per kargo dilacak melalui `CompanyEconomyEntry::cargo_income` (ekstensi arsip `XSLFI_COMPANY_CARGO_INCOME`), arsip bolak-balik tanpa kehilangan

## Bantuan Tonton (Watch)

Pengamat dengan cepat menemukan area pembangunan perusahaan tertentu (lompat ke koordinat terakhir perusahaan dibangun).

```
watch <company_id>    # Viewport lompat ke lokasi perusahaan
```

## Item yang Dilewati dan Alasannya

| Fitur cmclient | Penanganan jrpm |
|---|---|
| Overlay daftar pemain (cm_client_list_gui) | jgrpp **sudah memiliki** jendela Online Players (`NetworkClientList`), tidak perlu implementasi ulang |
| Tooltip detail tanah (cm_tooltips) | LandInfoWindow jrpm **sudah mencakup** tampilan detail rumah/industri/stasiun |

## File Terkait

- `src/jrpm_locations.cpp/.h` (baru: perintah konsol bookmark lokasi + company_cargo + watch)
- `src/jrpm_cargo_table.cpp/.h` (baru: jendela detail kargo)
- `src/window_type.h` (baru `WindowClass::CompanyCargos`)
- `src/console_cmds.cpp` (registrasi perintah)
- `src/lang/english.txt` (string STR_JRPM_CARGOS_*)