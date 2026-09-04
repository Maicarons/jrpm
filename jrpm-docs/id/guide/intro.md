---
title: Pengenalan Proyek
---

# Pengenalan Proyek OpenTTD-JRPM

## Apa Itu

**OpenTTD-JRPM (jrpm)** adalah cabang pengembangan turunan berdasarkan [JGR's Patchpack](https://github.com/JGRennison/OpenTTD-patches) (jgrpp), yang menggabungkan fitur "Kopling Lokomotif" dan "Bandara Modular" dari [pulsexlb/OpenTTD-patches](https://github.com/pulsexlb/OpenTTD-patches), serta menambahkan fungsi eksklusif jrpm. Versi saat ini: **jrpm-0.1.0**.

Ini adalah versi baru yang dapat diinstal secara independen dan bermain online secara independen: Klien/server jrpm menggunakan identifikasi versi independen (`jrpm-0.1.0`) untuk berjabat tangan, sepenuhnya terisolasi dari versi jgrpp asli dan pulsexlb, menghindari kekacauan versi.

## Ikhtisar Fitur

### Dari pulsexlb (152 komit, digabungkan melalui git merge)
- **Kopling Lokomotif (decouple)**：Sistem lengkap pemisahan/penggandengan kereta——perintah decouple, transfer tanda, batas panjang kopling dan kecepatan, dukungan dua lokomotif, kopling NewGRF, navigasi kopling (YAPF/NPF), penjadwalan independen setelah decouple;
- **Bandara Modular (multitile-airport)**：Restrukturisasi sistem bandara multi-ubin——sistem tipe air (`air.h`/`air_type.h`/`newgrf_airtype.*`), penjadwalan udara PBS (`pbs_air.*`), navigasi udara YAPF, `station.allow_modify_airports` untuk memodifikasi tata letak bandara, `gui.default_air_type` untuk tipe penerbangan default.

### Eksklusif jrpm
| Fitur | Deskripsi | Akses |
|---|---|---|
| Unduhan Sumber Daya: Multi-cermin + Paralel | Banyak sumber cermin dipisahkan koma, unduhan file paralel (konkurensi dapat dikonfigurasi), pengalihan cermin otomatis saat gagal | Pengaturan → `network.content_mirrors` / `network.content_download_parallel` |
| Pengelompokan Kendaraan Otomatis | Kelompokkan otomatis berdasarkan pesanan/jadwal bersama, nama grup diambil dari nama rute | Tombol jendela grup / Konsol `autogroup` |
| Tooltip Harga Bangunan | Tampilkan perkiraan biaya real-time di atas kursor saat membangun rel/jalan/bentuk tanah | Toolbar rel/jalan/bentuk tanah |
| AI Sadar Permainan Penuh | Pertahankan NoAI, tambahkan API global `AIGlobal` + contoh AI GlobalAI, dengan kontrol akses pengaturan game | `game.script.allow_global_ai_access` |
| Kompatibilitas Multi-versi Server | Server jrpm menerima klien jrpm / jgrpp asli / pulsexlb secara bersamaan | Berlaku otomatis saat bergabung |

### Warisan Lengkap
- Semua fitur jgrpp (peningkatan sinyal, scheduled dispatch, tracerestrict, penggantian templat, dll.);
- Semua fungsi OpenTTD hulu dan kompatibilitas ekosistem NewGRF/script.

## Hubungan Versi

```
                 jgrpp-0.73.1 (leluhur bersama)
                 /                 \
 cabang jgrpp (63 komit)        pulsexlb px-patch (152 komit)
 ├ Pembaruan terbaru jgrpp       ├ Kopling Lokomotif (cabang decouple)
 ├ Fitur eksklusif jrpm          └ Bandara Modular (cabang multitile-airport)
 └ Ganti nama versi jrpm-0.1.0
                 \                 /
                  cabang jrpm (git merge)
```

## Cabang & Komit

- Cabang: `jrpm` (jalur pengembangan utama)
- Komit Kunci:
  - `d4c45740` 5 fitur eksklusif jrpm
  - `71fe214c` merge pulsexlb (kopling + bandara modular)
  - `425e7207` Ganti nama versi openttd-jrpm / jrpm-0.1.0
  - `cb9848b7` Kompatibilitas klien multi-versi server
  - `4716b925` Konkurensi unduhan paralel dapat dikonfigurasi

## Lisensi

Sama dengan OpenTTD: **GPL-2.0**.