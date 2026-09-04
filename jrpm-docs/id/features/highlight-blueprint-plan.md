---
title: "Perencanaan Batch Ketiga: Sistem Sorotan + Sistem Cetak Biru"
---

# Perencanaan Batch Ketiga: Sistem Sorotan + Sistem Cetak Biru

Dua fitur bernilai tinggi dari cmclient telah diteliti secara mendalam, halaman ini mencatat skema porting dan estimasi beban kerja.

## I. Sistem Sorotan Objek (cm_highlight, 2888 baris)

**Fitur**: Saat alat bangun aktif, sorot objek yang akan dibangun secara real-time——pratinjau tepat untuk **15 objek** termasuk seluruh segmen rel, seluruh area stasiun, jalan/tempat parkir, sinyal, jembatan, terowongan, dermaga, bandara, industri, dll.

**Dependensi porting (vanilla API → jrpm API)**:

| Dependensi cmclient | Padanan jrpm |
|---|---|
| `DrawSelectionSprite` / `SetSelectionTilesDirty` / `DrawTileSelectionRect` / `DrawAutorailSelection` (di dalam viewport.cpp) | Ada tetapi tanda tangan berbeda (jgrpp telah direstrukturisasi), perlu diselaraskan satu per satu |
| `TileZoning` (pewarnaan zonasi kota) | jrpm tidak ada → perlu dibuat baru |
| `_fn_mod` dan status global lainnya | Eksklusif cmclient → perlu desain ulang |
| Konstruktor objek `ObjectTileHighlight::make_rail/road_stop/...` | Bergantung pada API stasiun/jalan NewGRF, jrpm dan vanilla berbeda |

**Beban kerja**: Sekitar 2–3 putaran (setiap putaran 4–6 jam), kesulitan inti ada pada penyelarasan pipeline rendering viewport.

## II. Sistem Cetak Biru (cm_blueprint, 660 baris)

**Fitur**: Pilih area → rekam urutan perintah bangun (rel/stasiun/terowongan/jembatan/sinyal) → penyimpanan 16 slot → bangun ulang satu tombol + rotasi.

**Dependensi porting**:

| Dependensi cmclient | Padanan jrpm |
|---|---|
| `cm_commands.hpp` **Lapisan objek perintah** (as_company / with_callback / set_auto / no_estimate + 100+ kelas perintah yang dihasilkan, 2251 baris) | jrpm tidak memiliki abstraksi ini → **harus di-porting/ditulis ulang terlebih dahulu** |
| Status `_station_gui` dari `cm_station_gui.hpp` | Struktur status GUI stasiun jrpm berbeda |
| Pointer pintar `sp<Blueprint>` | jrpm dapat menggunakan `std::shared_ptr` |
| Penelusuran ubin `BlueprintCopyArea` | Bergantung pada API umum seperti `TileIndexDiffC` (jrpm punya) |

**Beban kerja**: Sekitar 2–3 putaran (termasuk lapisan objek perintah).

## III. Urutan Implementasi yang Direkomendasikan

```
Langkah 1: Lapisan objek perintah (desain cm_command_type, implementasi ulang dengan Command<T>::Do/Post milik jrpm)
         —— Fondasi bersama untuk cetak biru dan pemutaran ulang perintah (load_commands)
Langkah 2: Sorotan objek (selaraskan pipeline viewport, porting objek satu per satu)
Langkah 3: Cetak biru (salin/putar/slot/pemutaran ulang)
Langkah 4 (opsional): Pemutaran ulang perintah (pemutaran ulang seluruh game lzma) + Zonasi kota (growth_tiles, perlu ekstensi arsip)
```

## Catatan Prasyarat

Ketiga fitur ini total sekitar **5000+ baris**, melintasi tiga lapisan inti sistem perintah/arsip/rendering viewport, dan bergantung pada serangkaian lapisan perantara yang tidak dimiliki jrpm (objek perintah, bus acara, serialisasi bitstream). Lebih aman untuk melanjutkan sebagai proyek khusus independen——disarankan untuk dilakukan secara bertahap, setiap batch dikompilasi + diverifikasi online sebelum dikomit, untuk menghindari perubahan besar sekali waktu yang sulit menemukan regresi.