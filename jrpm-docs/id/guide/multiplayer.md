---
title: Panduan Multiplayer
---

# Panduan Multiplayer

## Strategi Kompatibilitas Versi

jrpm adalah **versi tagged** (`jrpm-0.1.0`), jabat tangan multiplayer memerlukan string revisi **cocok persis**:

| Skenario | Perilaku |
|---|---|
| Klien jrpm ↔ Server jrpm | ✅ Normal (versi sama) |
| Klien jrpm ↔ Server jgrpp / pulsexlb asli | ❌ Ditolak (isolasi versi) |
| Klien jgrpp asli → Server jrpm | ✅ Diizinkan (server melonggarkan penerimaan revisi `jgrpp-*`) |
| Klien pulsexlb → Server jrpm | ✅ Diizinkan (menerima revisi `pxp`) |

::: warning Versi NewGRF
Terlepas dari versi klien, **nomor versi NewGRF harus sama persis dengan server** (validasi ketat `_openttd_newgrf_version`), ini adalah batas bawah simulasi deterministik.
:::

## Membuka Server

```bash
# Server khusus (tanpa GUI)
openttd-jrpm -D -c server.cfg
```

Konfigurasi `server.cfg` yang disarankan (lihat [Penyetelan Kinerja Server](../performance/server-tuning)):

```ini
[network]
server_name = My JRPM Server
server_port = 3979
max_clients = 32
max_companies = 15
frame_freq = 3
sync_freq = 50
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
```

## Bergabung dengan Server

- Dalam game "Multiplayer → Bergabung dengan Server Internet/Tambahkan Server", atau
- Baris perintah: `openttd-jrpm -n <host>:<port>`

## Unduhan Konten (NewGRF/Skenario)

jrpm mendukung **multi-cermin + unduhan paralel**:

```ini
[network]
content_server = content.openttd.org        ; Server metadata
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; Jumlah file unduhan paralel (1-8)
```

- Daftar cermin dipisahkan koma, digunakan secara berurutan; jika satu cermin gagal, beralih ke berikutnya secara otomatis;
- Variabel lingkungan `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS` memiliki prioritas lebih tinggi dari pengaturan;
- Jika semua cermin gagal, secara otomatis kembali ke protokol unduhan TCP lama.

## Manajemen Server

- Perintah konsol: `status`, `clients`, `kick`, `ban`, `save`, `reset_company`, `autogroup` (pengelompokan kendaraan otomatis), dll.;
- RCON: Setelah server mengatur `rcon_password`, perintah konsol dapat dikirim dari jarak jauh.