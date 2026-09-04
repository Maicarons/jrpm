---
title: Kopling Lokomotif (decouple)
---

# Kopling Lokomotif (decouple)

> Sumber: Cabang fitur `jgrpp-decouple` dari pulsexlb/OpenTTD-patches (inti dari 152 komit), digabungkan ke jrpm melalui git merge.

## Pengenalan Fitur

"Kopling Lokomotif" memungkinkan kereta untuk **memisahkan dan menggandeng** gerbong atau lokomotif di stasiun/selama penjadwalan, mewujudkan:

- **Decouple (Pemisahan)**: Kereta meninggalkan sebagian gerbong (atau gerbong+kelompok lokomotif) di stasiun tertentu sesuai "perintah decouple", sisanya terus berjalan;
- **Couple (Penggandengan)**: Kereta menunggu di stasiun, secara otomatis terhubung dengan kereta lain (atau kelompok gerbong yang diparkir);
- **Dua kereta menjalankan penjadwalan independen setelah decouple**: Melalui mekanisme seperti "loncatan perintah bersyarat", dua kereta setelah decouple menjalankan rencana penjadwalan yang berbeda;
- **Transfer tanda**: Saat decouple/couple, tanda (label pembatasan pelacakan) dipindahkan dengan benar atau diduplikasi mengikuti kereta.

## Kemampuan Inti

| Kemampuan | Deskripsi |
|---|---|
| Perintah decouple/couple | Tipe perintah baru, mendukung batasan "muat/jangan muat", "tunggu gandeng", "decouple" |
| Batas panjang kopling dan kecepatan | Membatasi operasi kopling berdasarkan panjang kereta; operasi kopling memiliki batas kecepatan |
| Dukungan dua lokomotif | Dua lokomotif depan-belakang (termasuk lokomotif dua kepala NewGRF) menggandeng/decouple dengan benar |
| Navigasi kopling | YAPF/NPF keduanya mendukung perencanaan jalur kopling; hanya kopling di dalam stasiun |
| Mundur | Dapat mundur setelah kopling, mendukung teknik penjadwalan seperti "mundur setelah decouple" |
| Kopling NewGRF | Mendukung atribut kopling yang ditentukan NewGRF (atribut 0xC6/0xF2, dll.) |
| Arsip | Versi arsip `SLV_ORDER_DECOUPLE`; penghitungan `num_decouple` dipersistensikan |

## Cara Penggunaan

1. Di stasiun, gunakan perintah "decouple" (`decouple`) untuk memisahkan kereta;
2. Atur perintah couple untuk membuat kereta menunggu koneksi;
3. Gunakan "loncatan perintah bersyarat" (conditional order skip) untuk membuat dua kereta setelah decouple mengambil rute berbeda;
4. Seret untuk menyesuaikan urutan gerbong di stasiun, atau gunakan alat bantu seperti "filter berdasarkan panjang".

## Kode Terkait

- Perintah: `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_type.h`
- Kereta: `src/train_cmd.cpp`, `src/train.h`
- Navigasi: `src/pathfinder/yapf`, `src/pathfinder/npf`
- Arsip: `src/sl/saveload_common.h` (`SLV_ORDER_DECOUPLE`)

## Catatan

- Fitur ini melibatkan perubahan mendalam pada urutan fisik kereta/logika reservasi sinyal, **disarankan untuk regresi utama setelah kompilasi di mesin nyata**: skenario decouple, couple, mundur, perbaikan tabrakan, penggantian otomatis (autoreplace), dll.;
- String terkait decouple dan GUI telah digabungkan (`STR_DECOUPLE*` dll.).