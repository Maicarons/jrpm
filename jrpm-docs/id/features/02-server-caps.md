---
title: Riset Batas Server
---

## Kesimpulan Riset: Kedua Batas Bersifat Struktural

### Batas Pemain Online = 255 (Kendala Kaku Lebar Bit Protokol)

| Lokasi | Konten |
|---|---|
| `src/network/network_type.h:21` | `static const uint MAX_CLIENTS = 255;` |
| `src/network/network_type.h:54` | `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>` |
| `src/network/core/network_game_info.cpp` | `clients_max` / `companies_max` ditransmisikan dengan **`Send_uint8` / `Recv_uint8`** di `SerializeNetworkGameInfo` (baris 251-296) dan deserialisasi (baris 422-432) |
| `src/table/settings/network_settings.ini:241` | `network.max_clients` (SLE_UINT8, max = MAX_CLIENTS, default 25) |
| `src/network/network_server.cpp:360` | Penerimaan memeriksa `_network_clients_connected < MAX_CLIENTS` |

**Kesimpulan**: 255 sudah merupakan batas atas bidang protokol uint8. Untuk menembusnya, bidang terkait seperti `clients_max` harus diubah menjadi uint16——ini akan mengubah protokol online client↔server, siaran UDP, dan protokol **Game Coordinator**, yang merupakan perubahan protokol eksternal (dalam fork, kedua ujung dapat dinaikkan untuk bermain, tetapi kompatibilitas daftar server publik akan terpengaruh).

### Batas Jumlah Perusahaan = 15 (Kendala Kaku Format Penyimpanan Pemilik Ubin)

| Lokasi | Konten |
|---|---|
| `src/company_type.h:25` | `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15` |
| `src/tile_map.h:195` | `SetTileOwner`：`SB(_m[tile].m1, 0, 5, owner.base())` —— **Pemilik ubin hanya menyimpan 5 bit** (5 bit rendah dari `_m[].m1`) |
| `src/company_type.h:30-33` | `OWNER_TOWN{0x0F}`、`OWNER_NONE{0x10}`、`OWNER_WATER{0x11}`、`OWNER_DEITY{0x12}`、`OWNER_END{0x13}` berbagi ruang byte yang sama dengan ID perusahaan |

**Kesimpulan**: `Owner` dan `CompanyID` adalah tipe yang sama, bidang kepemilikan ubin hanya 5 bit (nilai 0-31), di mana 15-18 ditempati oleh pemilik khusus. Oleh karena itu batas perusahaan sebenarnya 15 ditentukan oleh **format array peta**——untuk menembusnya perlu memperluas penyimpanan pemilik ubin (`m1` penuh 8 bit atau mengubah struktur `_m`) dan memigrasi konstanta OWNER_*, yang melibatkan **konversi arsip penuh** dan peningkatan memori peta, merupakan restrukturisasi format besar (alasan OpenTTD hulu tidak melakukannya selama bertahun-tahun).

## Opsi yang Tersedia

| Opsi | Perubahan | Kompatibilitas | Saran |
|---|---|---|---|
| A. Pertahankan status saat ini | Tidak ada | Arsip/protokol/koordinator sepenuhnya kompatibel | ✅ Direkomendasikan: 255 klien / 15 perusahaan sudah jauh melebihi versi asli untuk multiplayer |
| B. Klien → protokol uint16 | `network_game_info.cpp` serialisasi/deserialisasi diubah `Send_uint16/Recv_uint16`; `MAX_CLIENTS` dinaikkan ke 4095; `network_settings.ini` `max_clients` diubah SLE_UINT16; `settings_type.h` bidang diubah uint16; `console_cmds.cpp:1056` lebar tampilan | Dalam fork kedua ujung dapat bermain; koordinator publik/klien lama tidak kompatibel | Opsional, jika memang memerlukan >255 klien |
| C. Perusahaan → restrukturisasi format ubin | Bidang pemilik ubin diperlebar + migrasi konstanta OWNER_* + konversi arsip penuh | Perubahan format arsip (konversi satu kali) | Tidak disarankan untuk implementasi jangka pendek |

## Konten yang Telah Diserahkan

- Dokumen desain ini (dengan file/nomor baris yang tepat);
- Patch opsional `option-clients-uint16.diff` (perubahan lengkap untuk opsi B, **belum diterapkan**, untuk review `git apply`).

> Kesimpulan awal: **"Perluasan" kedua batas pada dasarnya adalah peningkatan format/protokol, bukan penyesuaian item konfigurasi**; dengan tetap menjaga kompatibilitas arsip dan jaringan publik, 255/15 saat ini adalah batasnya. Jika bersedia menerima biaya kompatibilitas, perubahan opsi B/C seperti di atas.