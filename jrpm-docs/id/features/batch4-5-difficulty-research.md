# Riset Kesulitan Porting: Batch Keempat / Kelima / Perintah Server CM

> Objek riset: Pembacaan mendalam kode sumber citymania-org/cmclient (cabang vanilla 15.3)
> Waktu riset: 2026-08-14
> Kesimpulan: Tiga bagian pekerjaan memiliki tingkat kesulitan yang sangat berbeda, diberikan evaluasi per item dan urutan yang direkomendasikan.

---

## I. Batch Keempat: Zonasi Kota (termasuk arsip growth_tiles)

### Komponen Fitur

| Komponen | File | Ukuran | Deskripsi |
|---|---|---|---|
| Enum mode zonasi | cm_zoning.hpp | 37 baris | 12 mode evaluasi (CHECKOPINION/CHECKBUILD/CHECKSTACATCH/CHECKACTIVESTATIONS/CHECKBULUNSER/CHECKINDUNSER/CHECKTOWNZONES/CHECKCBACCEPTANCE/CHECKCBTOWNLIMIT/CHECKTOWNADZONES/CHECKTOWNGROWTHTILES) |
| Logika evaluasi | cm_zoning_cmd.cpp | 413 baris | Satu fungsi kueri per mode (mengembalikan palet SpriteID), termasuk algoritma radius TownZone/Tz, pemeriksaan cakupan StationFinder, pemeriksaan bangunan/industri tidak terlayani |
| GUI toolbar zonasi | cm_zoning_gui.cpp | 204 baris | Jendela pemilihan mode zonasi lapisan dalam/luar (status global `_zoning.inner/outer`) |
| Arsip growth_tiles | cm_saveload.cpp/.hpp | 90 baris | Bidang ekstensi Town `growth_tiles` + `growth_tiles_last_month` (`std::map<TileIndex, uint8_t>`), disimpan dengan handler SaveLoad vanilla |
| Ekstensi data | extensions/cmext_town.hpp | ~80 baris | `ext::Town`: growth_tiles ×2 + **banyak bidang permainan server CM** (statistik kargo CBTownInfo, pelacakan iklan/dana, hitungan pertumbuhan kota hs/cs/hr) |
| Rotasi bulanan + pemicu | cm_game.cpp / town_cmd.cpp | — | Rotasi growth_tiles NewMonth; catat status di pembangunan/penghancuran/pembangunan ulang rumah |

### Evaluasi Kesulitan Porting per Item

| Kesulitan | Tingkat | Deskripsi |
|---|---|---|
| **Penulisan ulang sistem arsip** | 🟠 Sedang | growth_tiles menggunakan `DefaultSaveLoadHandler` vanilla + `SlSetStructListLength`/`SlObject`, jrpm adalah sistem baru `sl/` (SlTableHeader/SlObjectSaveFiltered). Perlu ditulis ulang dengan NSL/SLE + **gerbang fitur XSLF** jrpm (baru `XSLFI_TOWN_GROWTH_TILES`, versi 1), arsip lama tidak terpengaruh |
| **Ekstensi struktur Town** | 🟢 Rendah | Hanya ambil dua peta growth_tiles (**lewati** bidang CBTownInfo/iklan/dana——itu khusus untuk permainan CityBuilder server CM); tambah bidang di jrpm Town + pasang tabel arsip town_sl.cpp |
| **Logika evaluasi** | 🟢 Rendah-Sedang | Sebagian besar kueri murni (GetTileType/StationFinder/Cache Town/periksa rumah), API jrpm ada; Algoritma radius TownZone (`squared_town_zone_radius`) perlu diperiksa nama bidang jrpm |
| **Pipeline rendering** | 🟠 Sedang-Tinggi | `DrawTileZoning` perlu dipasang ke pipeline rendering viewport——**berbagi mekanisme rendering TileHighlight dengan sorotan batch ketiga**. Tanpa pipeline sorotan, harus dibangun sendiri (disarankan **buat sorotan dulu, baru zonasi**) |
| **Sumber daya sprite** | 🟠 Sedang | 12 palet menggunakan **sprite kustom** `CM_SPR_PALETTE_ZONING_*`, jrpm tidak punya → perlu diganti dengan sprite palet yang ada atau tambah sumber daya baru |
| **Kait peristiwa** | 🟠 Sedang | Perekaman growth_tiles bergantung pada **bus acara** cmclient (event::HouseBuilt/HouseCleared/..., Emit dari cm_main.cpp). jrpm tidak memiliki mekanisme ini → langsung tambah kait di town_cmd.cpp di pembangunan/penghancuran rumah + rotasi NewMonth (IntervalTimer) |

### Kesimpulan Kesulitan: 🟠 Sedang-Tinggi (sekitar 1.5–2 putaran khusus, setiap putaran 4–6 jam)

- **Prasyarat**: Sangat disarankan untuk dilakukan setelah **sistem sorotan** batch ketiga selesai (pipeline rendering bersama)
- Jika hanya melakukan "12 jenis pewarnaan zonasi, tanpa arsip growth_tiles": kesulitan turun menjadi 🟡 Sedang (hemat ekstensi arsip, sekitar 1 putaran)

---

## II. Batch Kelima: Pemutaran Ulang Perintah + Ekspor/Rekam

### 2.1 Pemutaran Ulang Perintah (cm_command_log + cm_commands + generated)

| Komponen | Ukuran | Deskripsi |
|---|---|---|
| Lapisan objek perintah | cm_command_type.hpp + generated/cm_gen_commands (2251+1418 baris) | Setiap perintah vanilla dibungkus sebagai objek yang dapat diprogram (as_company/with_callback/set_auto), termasuk **serialisasi perintah→bitstream** |
| Muat log perintah | cm_command_log.cpp (203 baris) | Dekompresi lzma + parsing bitstream BitOStream → antrian `_fake_commands` (tick counter/hasil yang diharapkan/seed acak/CommandPacket) |
| Eksekusi perintah | ExecuteFakeCommands | Eksekusi berurutan berdasarkan tick counter: `ExecuteCommand(&cp)` (API internal vanilla) + **verifikasi seed acak/hasil** (anti-cheat), teruskan ke semua klien saat online |

**Perbedaan Kunci (menentukan kesulitan)**:

| cmclient | jrpm | Dampak |
|---|---|---|
| `ExecuteCommand(CommandPacket*)` | Tidak ada fungsi ini, inti perintah adalah `DoCommandPInternal(Commands, TileIndex, CommandPayloadBase&, ...)` | Perlu menulis lapisan konversi payload CommandPacket → DoCommandPInternal 🟠 |
| Bidang CommandPacket (vanilla) | CommandPacket ada tetapi struktur berbeda (GeneralCommandPacket\<DynBaseCommandContainer\>) | Perlu adaptasi pemetaan bidang 🟠 |
| `GetCommandName` | ✅ Ada (command_func.h:166) | 🟢 |
| `outgoing_queue` (teruskan online) | Lapisan jaringan jrpm berbeda (OutgoingCommandPacket/ServerNetworkGameSocketHandler::SendCommand) | Pemutaran ulang online perlu menulis ulang logika penerusan 🟠 |
| Perekam (bagaimana menghasilkan file .cmd) | **Kait perekaman cmclient bergantung pada intersepsi post() lapisan objek perintah** | Perintah jrpm adalah Post yang ditemplate → perlu tambah kait perekaman di titik distribusi perintah 🔴 Pekerjaan inti |
| Format file | Privat (ID perintah vanilla + bitstream) | ID perintah jrpm sangat berbeda dari vanilla → file pemutaran ulang tidak kompatibel, **format perlu desain ulang** 🟠 |

### 2.2 Ekspor / Rekam (cm_export.cpp, 536 baris)

| Fitur | Deskripsi | Kesulitan |
|---|---|---|
| ExportOpenttdData | Ekspor JSON spesifikasi rumah/spesifikasi kargo/palet/informasi mesin (JsonWriter) | 🟢 Rendah-Sedang (independen, baca struktur Spec tulis JSON) |
| ViewportExport / ExportFrameSprites | Ekspor sprite viewport per frame (bergantung pada vektor rendering internal viewport TileSpriteToDrawVector/ParentSpriteToSortVector) | 🟠 Sedang-Tinggi (pipeline viewport jgrpp berbeda, perlu selaraskan antarmuka vektor) |

### Kesimpulan Kesulitan: 🟠 Sedang-Tinggi (sekitar 2 putaran khusus)

- **Pemutaran ulang perintah adalah bagian terbesar**: Pekerjaan inti = ① Tambah **kait perekaman** di titik distribusi perintah (adaptasi sistem perintah template jrpm) ② Lapisan eksekusi CommandPacket → DoCommandPInternal ③ Desain format file baru. **Disarankan porting lapisan objek perintah terlebih dahulu** (fondasi cetak biru batch ketiga, juga fondasi pemutaran ulang)
- Ekspor: Ekspor data JSON dapat dilakukan secara independen terlebih dahulu (rendah-sedang); Rekam frame bergantung pada penyelarasan viewport (ditangani bersamaan dengan sorotan)

---

## III. Perintah Server CM (cm_console_cmds.cpp, 289 baris)

### Daftar Perintah dan Kesulitan per Item

| Perintah | Fungsi | Kesulitan untuk jrpm | Catatan |
|---|---|---|---|
| `cmgamespeed [n]` | Ubah kecepatan game | 🟢 **Sangat Rendah** (~20 baris) | jrpm sudah memiliki global `_game_speed` (gfx.cpp:52), hanya kurang shell perintah |
| `cmstep [n]` | Langkah n tick | ⏭️ **jrpm sudah ada** | Perintah `step` (ConStepGame) fungsinya sama, lewati |
| `cmexport` | Ekspor openttd.json | 🟢 Rendah-Sedang | Bergantung pada ExportOpenttdData (lihat batch kelima) |
| `cmtreemap <file>` | Tanam pohon dari peta ketinggian | 🟡 Sedang | Baca peta ketinggian + perintah tanam pohon, independen |
| `cmreset_town_growth` | Hapus catatan pertumbuhan kota | 🟢 Rendah | Bergantung pada bidang zonasi kota |
| `cmload_commands` | Muat pemutaran ulang perintah | 🟠 Sedang-Tinggi | Bergantung pada infrastruktur pemutaran ulang perintah |
| `cmstart_record` / `cmstop_record` | Rekam frame | 🟠 Sedang-Tinggi | Bergantung pada infrastruktur rekam |
| `cmgamestats` | Statistik sesi game | 🟢 Rendah | Independen |
| `cmgfxdebug` | Debug grafis | 🟢 Rendah | Independen |

### Kesimpulan Kesulitan: 🟢 Umumnya Rendah (setiap perintah 0.5–2 jam)

**Urutan yang direkomendasikan**: `cmgamespeed` (10 menit) → `cmgamestats`/`cmgfxdebug` (masing-masing ~1 jam) → `cmexport` (bersamaan dengan ekspor batch kelima) → `cmtreemap` (~2 jam) → `cmreset_town_growth` (bersamaan dengan batch keempat) → `cmload_commands`/`cmstart_record` (bersamaan dengan pemutaran ulang/rekam batch kelima).

---

## IV. Rute yang Direkomendasikan Secara Keseluruhan

```
① Sistem sorotan (inti batch ketiga, ~2-3 putaran)      ← Fondasi rendering cetak biru/zonasi
② Lapisan objek perintah (fondasi cetak biru batch ketiga + pemutaran ulang batch kelima, ~1-2 putaran)
③ Cetak biru (~1 putaran)
④ Zonasi kota (~1.5 putaran, bergantung pada pipeline rendering ①; growth_tiles menggunakan gerbang XSLF)
⑤ Pemutaran ulang perintah + ekspor (~2 putaran, bergantung pada ②; ekspor JSON dapat dilakukan independen lebih awal)
⑥ Perintah server CM (disisipkan di antara batch, gamespeed/step dapat segera dilakukan)
```

## V. Item Biaya Rendah yang Dapat Segera Dilakukan (Tidak Bergantung pada Proyek Besar)

1. Perintah `cmgamespeed` (~20 baris, ubah `_game_speed`)
2. `step` sudah ada → tidak perlu dilakukan
3. `cmgamestats` / `cmgfxdebug` (masing-masing ~1 jam, independen)
4. Ekspor data JSON `cmexport` (~2 jam, independen)
5. `cmtreemap` (~2 jam, independen)

Kelima item ini sekitar **setengah hari kerja**, dapat dilakukan sebagai "batch nol" terlebih dahulu, tidak bertentangan dengan batch tiga/empat/lima.