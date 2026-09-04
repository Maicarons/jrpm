---
title: Panduan Penyetelan Kinerja Server
---

> Berdasarkan cabang jrpm setelah penggabungan (berisi fitur pulsexlb dan optimasi eksklusif jrpm).
> Nilai default parameter diambil dari `src/table/settings/network_settings.ini`, disarankan mengacu pada hasil pengujian tekanan aktual.

## I. Optimasi Kinerja yang Telah Diterapkan (Saat Ini)

| Optimasi | Komit | Deskripsi |
|---|---|---|
| Konkurensi unduhan paralel dapat dikonfigurasi | 4716b925 | `network.content_download_parallel` (default 4, 1-8): Jumlah file yang diunduh secara bersamaan untuk unduhan konten, menggantikan hardcoded 4 |
| Multi-cermin + unduhan paralel file | d4c45740 (F1) | `network.content_mirrors` multi-cermin dipisahkan koma, coba ulang per cermin saat gagal, akhirnya kembali ke protokol lama |
| Kompatibilitas klien multi-versi server | cb9848b7 | Server dapat secara bersamaan menerima klien jrpm / jgrpp asli / pulsexlb |

## II. Parameter Kritis Kinerja Server (Sistem jgrpp)

| Pengaturan | Default | Rentang | Arti | Saran Penyetelan |
|---|---|---|---|---|
| `network.sync_freq` | 100 | 0-100 | Setiap berapa frame melakukan sinkronisasi frame (deteksi desync). Semakin besar semakin hemat bandwidth, semakin kecil semakin cepat menemukan ketidakcocokan | Jika stabilitas online prioritas, turunkan ke 20-50; jika sering desync, turunkan |
| `network.frame_freq` | 0 | 0-100 | Server mengirimkan frame perintah setiap berapa frame (0 = kirim setiap frame). Semakin besar semakin hemat bandwidth/CPU, tetapi latensi operasi naik | Normal 0-3; server dengan banyak pemain bisa 5, timbang setelah uji tekanan |
| `network.commands_per_frame` | 2 | 1-65535 | Batas atas jumlah perintah klien yang diproses per frame (anti-spam/anti-kejahatan) | Saat banyak pemain dan sering beroperasi, naikkan ke 4-8 |
| `network.commands_per_frame_server` | 16 | 1-65535 | Batas atas perintah server sendiri per frame | Umumnya tidak perlu diubah |
| `network.bytes_per_frame` | 8 | 1-65535 | Batas atas rata-rata byte yang diterima per frame jangka panjang (pembentuk bandwidth) | Jika bandwidth bagus, naikkan ke 16-32, tingkatkan kecepatan sinkronisasi peta besar |
| `network.bytes_per_frame_burst` | 256 | 1-65535 | Batas atas byte lonjakan (mengizinkan puncak jangka pendek) | Naikkan bersamaan dengan item di atas, mis. 512 |
| `network.max_init_time` | 60 | 0-32000 | Waktu habis inisialisasi klien (tick) | Longgarkan jika banyak pemain dengan jaringan lemah |
| `network.max_join_time` | 500 | 0-32000 | Waktu habis klien bergabung (unduh peta + sinkronisasi) (tick) | Longgarkan ke 1000+ untuk peta besar/bandwidth lambat |
| `network.max_download_time` | 1000 | 0-32000 | Waktu habis unduhan peta (tick) | Peta besar (4096+) disarankan 2000+ |
| `network.max_lag_time` | 800 | 0-32000 | Toleransi latensi maksimum klien (tick) | Longgarkan jika banyak pemain dengan latensi tinggi, tetapi akan meningkatkan risiko desync |

> Catatan: Satuan `max_*_time` adalah tick game (1/74 detik ≈ 13.5ms); `bytes_per_frame` mengacu pada rata-rata byte jendela sinkronisasi per frame.

## III. Konfigurasi Awal Server yang Direkomendasikan (Skenario Multiplayer)

```ini
[network]
max_clients = 32            ; atau sesuai kebutuhan
max_companies = 15
frame_freq = 3              ; 0=kirim perintah setiap frame (paling lancar); 3=kompromi hemat bandwidth
sync_freq = 50              ; temukan ketidakcocokan lebih awal
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
content_download_parallel = 4   ; konkurensi unduhan konten sisi klien
```

## IV. Arah Optimasi Kinerja yang Dapat Diterapkan Selanjutnya (Berdasarkan Prioritas)

1. **Thread pool lapisan transportasi HTTP + Unduhan chunk Range** (ide Openttd-Cluster 0007)
   Status saat ini: F1 sudah mengimplementasikan "paralel file" (beberapa file diunduh bersamaan). 0007 adalah thread pool di lapisan HTTP + chunk `CURLOPT_RANGE`, untuk **satu file besar** (skenario raksasa .tar.gz) masih bisa dipercepat. Keduanya saling melengkapi tetapi mengubah file yang sama, disarankan dilakukan secara inkremental setelah F1 stabil.

2. **Optimasi pengiriman peta**
   Verifikasi apakah jrpm mengaktifkan kompresi peta secara default (zstd/lzma); peta besar dapat membandingkan waktu berbagai algoritma kompresi.

3. **Runtime server Rust (referensi arsitektur jangka panjang)**
   otc-engine Openttd-Cluster (Admin/RCON, metrik Prometheus, cluster failover, panel Web, jembatan snapshot) bergantung pada integrasi FFI proyek Rust lengkap, termasuk perubahan tingkat "server generasi berikutnya", bukan penggabungan level patch; jrpm saat ini tetap mempertahankan biner tunggal C++ murni.

4. **Kinerja simulasi game**
   - Untuk peta besar + banyak kendaraan, perhatikan parameter `economy`/`linkgraph` (linkgraph_settings.ini);
   - Jika memerlukan kinerja ekstrem, dapat mempelajari kelas `settings_game.economy.` dan parameter batas pathfinder (pengaturan `pathfinding`).

## V. Saran Verifikasi

- Buka server `openttd-jrpm -D -c server.cfg`, gunakan beberapa klien jrpm untuk uji aktual: waktu bergabung, stabilitas sinkronisasi frame (tanpa petunjuk desync), penggunaan CPU/memori;
- Unduhan konten: atur `content_download_parallel=8` bandingkan waktu unduhan satu batch NewGRF;
- Uji tekanan jaringan lemah: uji kombinasi `max_lag_time` dan `sync_freq`, temukan batas stabil.