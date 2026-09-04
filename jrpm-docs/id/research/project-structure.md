---
title: Riset Struktur Proyek dan Organisasi Kode
---

> Objek riset: `G:\GitHub\OpenTTD-patches` (workspace sumber jgrpp)
> Referensi: `G:\game\openttd-jgrpp` (game lengkap terkompilasi, hanya baca)
> Tanggal riset: 2026-08-14
> Cabang: jgrpp (HEAD saat ini `5b5c452e1b`, sekitar versi 16.0)

---

## 1. Ikhtisar Proyek

OpenTTD jgrpp (JGR's Patchpack) adalah cabang peningkatan terkenal berdasarkan OpenTTD, kode sumber workspace ini sesuai dengan versi **16.0**, berisi banyak fitur privat JGR (tracerestrict, programmable signals, scheduled dispatch, penggantian templat, peningkatan sinyal, dll.).

| Proyek | Deskripsi |
|---|---|
| Bahasa | C++20 (`CMAKE_CXX_STANDARD 20`, tanpa ekstensi) |
| Build | CMake (≥3.17), `src/CMakeLists.txt` diatur per direktori |
| Mesin Script | Squirrel (`src/3rdparty/squirrel/`), digunakan untuk AI / GameScript / script templat |
| Jaringan | Lapisan protokol TCP/UDP buatan sendiri (`src/network/`), HTTP menggunakan WinHttp (Windows) / libcurl (lainnya) / JS (Emscripten) |
| Arsip | `src/saveload/` + `src/sl/` format biner buatan sendiri |
| Thread | `src/thread.h` + `src/worker_thread.cpp` (WorkerThreadPool task pool), `src/timer/` timer |

---

## 2. Struktur Direktori Sumber dan Pembagian Modul

Tingkat atas: `CMakeLists.txt` sebagai skrip build root; `src/` untuk semua kode sumber C++; `bin/` untuk data runtime (script kompatibel AI, file bahasa, dll.); `media/`, `os/`, `cmake/`, `docs/` masing-masing untuk sumber daya, kode platform, skrip build, dan dokumentasi.

Subdirektori utama dalam `src/` dan tanggung jawabnya:

| Direktori | Tanggung Jawab |
|---|---|
| `src/core/` | Alat dasar: tipe pool, bitset, container, pembungkus thread, operasi bit, dll. |
| `src/network/` | Jaringan: server/klien/UDP/HTTP/unduhan konten/protokol admin (subdirektori `core/` adalah dasar protokol) |
| `src/script/` | Kerangka eksekusi script: pembungkus Squirrel, instance, konfigurasi; `api/` adalah semua kelas API yang diekspos ke AI/GS |
| `src/ai/` | Kerangka NoAI (instance AI, pemindai, konfigurasi, GUI) |
| `src/game/` | Kerangka GameScript (instance GS, konfigurasi, GUI) |
| `src/newgrf/` | Dekode dan pemrosesan NewGRF |
| `src/pathfinder/` | Navigasi (YAPF, NPF) |
| `src/saveload/`, `src/sl/` | Baca/tulis arsip |
| `src/blitter/`, `src/video/`, `src/fontcache/`, `src/music/`, `src/sound/` | Backend rendering/audio/video |
| `src/lang/` | Sumber string lokal (dihasilkan oleh strgen) |
| `src/table/` | Tabel statis; `table/settings/*.ini` adalah **sumber definisi pengaturan** (dihasilkan oleh settingsgen) |
| `src/3rdparty/` | Pustaka pihak ketiga (squirrel, llvm, icu, dll.) |
| `src/timer/`, `src/os/`, `src/misc/` | Timer, platform, lainnya |

File tingkat atas dinamai berdasarkan sistem (mis. `rail_gui.cpp`, `group_cmd.cpp`, `vehicle.cpp`, `order_cmd.cpp`, `economy.cpp`), mengikuti konvensi OpenTTD: `*_cmd` untuk logika perintah, `*_gui` untuk jendela, `*_base/_type/_func` untuk struktur data dan fungsi inline.

---

## 3. Metode Build

- **CMake tiga langkah**: `cmake -B build ..` → `cmake --build build` → hasil `openttd.exe`. Repo menyertakan `build.sh` / `build-dedicated.sh`.
- **Dependensi**: Deklarasi `vcpkg.json` (zlib, lzma, lzo, zstd, png, SDL2, freetype, harfbuzz, icu, opus, dll.); Windows menggunakan WinHttp (tidak perlu curl), non-Windows menggunakan libcurl (`CMakeLists.txt:121-127`).
- **Toolchain (host tools)**: `strgen` (file bahasa), `settingsgen` (menghasilkan kode pengaturan dari `src/table/settings/*.ini`), `squirrel_export` (menghasilkan binding Squirrel dari `src/script/api/script_*.hpp`).
- **Hasil penting**: `generated/script/api/<ai|gs>/...sq.hpp` (binding API, **ditemukan secara otomatis oleh `file(GLOB script_*.hpp)`, kelas API baru tidak perlu mengubah daftar registrasi**, hanya perlu tambah `.hpp` dan masukkan `.cpp` ke daftar sumber `src/script/api/CMakeLists.txt`); `generated/rev.cpp`; `generated/ottdres.rc`.
- **Sistem pengaturan**: Versi modern telah diubah menjadi **berbasis INI**——`src/table/settings/*.ini` (berisi segmen `[SDTC_VAR]`, `cat=SC_*` kategori, `flags`, `post_cb`, dll.), kode `settings_*.cpp/h` dihasilkan oleh settingsgen; anggota struct terkait ada di `src/settings_type.h` (mis. `NetworkSettings` mulai baris 575).

---

## 4. Lokasi Implementasi Lima Sistem Fitur Besar

### 4.1 Unduhan Sumber Daya (Unduhan Konten / BaNaNaSplit)

| Poin Perhatian | Lokasi |
|---|---|
| Kelas utama klien konten | `src/network/network_content.h/.cpp` — `ClientNetworkContentSocketHandler` (merangkap `ContentCallback` + `HTTPCallback`) |
| GUI unduhan konten | `src/network/network_content_gui.cpp/.h` |
| Klien HTTP | `src/network/core/http.h/.cpp` — `NetworkHTTPSocketHandler::Connect(uri, callback, data)`, digerakkan peristiwa asinkron (non-blocking, polling loop utama) |
| String koneksi server konten | `src/network/core/config.cpp` — `NetworkContentServerConnectionString()`: Variabel lingkungan `OTTD_CONTENT_SERVER_CS`, default `content.openttd.org` (protokol metadata TCP) |
| URI cermin | `src/network/core/config.cpp` — `NetworkContentMirrorUriString()`: Variabel lingkungan `OTTD_CONTENT_MIRROR_URI`, default `https://binaries.openttd.org/bananas` |
| Alur unduhan | `DownloadSelectedContent()` → `DownloadSelectedContentHTTP()` (POST semua ID konten ke cermin, cermin mengembalikan aliran tar multi-file, tulis file per file) → `AfterDownload()` melakukan gunzip + `TarScanner` ekstrak |
| Dekompresi | `GunzipFile()` (zlib), `TarScanner`/`ExtractTar` (`src/tar_type.h` / `src/fileio.cpp`) |
| Fasilitas thread (dapat digunakan untuk paralel) | `src/worker_thread.h/.cpp` — `WorkerThreadPool` + `EnqueueJob`; `src/thread.h` adalah pembungkus thread platform |

**Kesimpulan status saat ini**: ① Hanya 1 cermin dan hanya dapat dikonfigurasi melalui variabel lingkungan, tanpa pengaturan dalam game; ② Unduhan adalah **koneksi tunggal, serial** (satu POST semua file); ③ Dekompresi dilakukan sinkron di thread utama. → Titik perubahan multi-thread/multi-cermin jelas.

### 4.2 Batas Pemain Online / Perusahaan Server

| Poin Perhatian | Lokasi |
|---|---|
| Konstanta batas klien | `src/network/network_type.h:21` — `static const uint MAX_CLIENTS = 255;` |
| Pool klien | File yang sama `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>`; `ClientID` adalah `uint32_t` |
| Pool ID perusahaan | `src/company_type.h` — `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15`; perusahaan palsu menempati 253/254/255 |
| Masker perusahaan | File yang sama `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>` (16 bit, hanya dapat melacak 16 perusahaan) |
| Pemeriksaan penerimaan server | `src/network/network_server.cpp:360` — `_network_clients_connected < MAX_CLIENTS`; `static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| Item pengaturan klien | `src/table/settings/network_settings.ini:231/241` — `network.max_companies` (def 15, max MAX_COMPANIES), `network.max_clients` (def 25, max MAX_CLIENTS); struct di `src/settings_type.h` `NetworkSettings` |
| **Lebar bit protokol (kendala kaku)** | `src/network/core/network_game_info.cpp` — `companies_max` dan `clients_max` keduanya dikirim dengan **`Send_uint8`/`Recv_uint8`** (baris 251-296, 422-432) |
| Tampilan daftar server | `src/network/network_gui.cpp:519` dll. |

**Kesimpulan status saat ini**:
- **Batas klien = 255 sudah merupakan batas protokol** (bidang uint8 + kompatibilitas game coordinator/browser server). Untuk menembusnya, bidang terkait `network_game_info` harus diubah menjadi uint16 (seluruh rantai client↔server + siaran UDP + protokol game coordinator), ini adalah perubahan protokol eksternal.
- **Batas perusahaan = 15** (End=0xF dari `CompanyIDTag`). Dapat ditingkatkan dengan aman ke **252** (End=0xFC): dasarnya masih uint8, lebar byte arsip tidak berubah (kompatibel dengan arsip lama), perlu melonggarkan `CompanyMask` (uint16→uint32) dan asumsi antarmuka/lokal.

### 4.3 Pengelompokan Kendaraan

| Poin Perhatian | Lokasi |
|---|---|
| Struktur data grup | `src/group.h` — `Group : GroupPool::PoolItem` (name/owner/vehicle_type/flags/livery/statistics/parent/number); `GroupID`, `DEFAULT_GROUP`, `IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| Perintah grup | `src/group_cmd.cpp` — `CmdCreateGroup`(536), `CmdDeleteGroup`(585), `CmdAlterGroup`(646), `CmdAddVehicleGroup`, `CmdAddSharedVehicleGroup` (memasukkan kendaraan pesanan bersama ke grup yang sudah ada, sekitar baris 718 `AddVehicleToGroup`) |
| Registrasi perintah | `src/group_cmd.h:27-35` — `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)`; enum di `src/command_type.h` `enum class Commands` (mulai baris 492) |
| GUI grup | `src/group_gui.cpp/.h`, `src/vehiclelist.cpp` |
| Kendaraan↔grup | `src/vehicle_base.h` (`Vehicle::group_id`), `SetTrainGroupID/UpdateTrainGroupID` (group.h:130-131) |
| Pesanan/jadwal bersama | `src/order_base.h` (`OrderList`, `VehicleOrdersID`), `src/order_cmd.cpp`, `src/order_func.h`, `src/schdispatch.h/.cpp` (scheduled dispatch, terikat ke order list) |
| Statistik grup | `GroupStatistics` (group.h:60-66), `GetGroupNumVehicle` dll. (group.h:125-128) |

**Kesimpulan status saat ini**: Sudah ada `CmdAddSharedVehicleGroup` (memasukkan kendaraan pesanan bersama dari satu kendaraan ke grup tertentu) dan `CmdCreateGroupFromList` (membuat grup dari daftar), tetapi **tidak ada logika lengkap untuk "membuat grup secara otomatis dan mengelompokkan berdasarkan pesanan bersama"**. Perintah baru `AutoGroupSharedOrders` (menelusuri semua kendaraan utama perusahaan → mengelompokkan berdasarkan `OrderList` → membuat grup secara otomatis dan mengelompokkan) jalur implementasinya jelas.

### 4.4 Tooltip Bangunan (petunjuk harga saat membangun rel dll.)

| Poin Perhatian | Lokasi |
|---|---|
| GUI/logika pembangunan rel | `src/rail_gui.cpp` (`BuildRailToolbarWindow`), `src/rail_cmd.cpp`, `src/rail.h/.cpp`; jalan `road_gui.cpp/road_cmd.cpp` |
| Estimasi biaya | `DoCommand` dari masing-masing `*_cmd.cpp` mengembalikan `CommandCost`; dalam GUI dapat menggunakan mode `DC_QUERY_COST` untuk menanyakan harga |
| Ubin di bawah kursor | `src/viewport_func.h:36` — `GetTileBelowCursor()`; `_cursor.pos` (koordinat layar); `src/viewport.cpp:1056` |
| Mekanisme petunjuk teks yang ada | `src/texteff.hpp` — `AddTextEffect(msg, x, y, duration, mode, ...)` (teks mengambang koordinat dunia), `UpdateTextEffect`; `src/texteff.cpp` |
| UX petunjuk pembangunan yang ada | Toolbar rel `OnPlaceDrag` saat menyeret ada area yang dipilih dan akumulasi biaya (`_thd` tilehighlight, `src/tilehighlight_func.h`); status bar `statusbar_gui.cpp` dapat menampilkan biaya alat |
| Titik refresh per frame | Masing-masing jendela toolbar `OnMouseLoop` / `viewport.cpp` `HandleMouseEvents` (baris 5422/5733) |

**Kesimpulan status saat ini**: Tidak ada "tooltip harga di atas kursor". Dapat menggunakan `AddTextEffect` yang ditambatkan ke ubin di mana kursor berada (mengikuti ubin di bawah kursor), atau tooltip koordinat layar yang digambar sendiri; biaya dapat menggunakan `DC_QUERY_COST` untuk menanyakan harga satu ubin dari alat saat ini + akumulasi area yang dipilih.

### 4.5 Antarmuka AI (NoAI Dipertahankan + AI Sadar Permainan Penuh)

| Poin Perhatian | Lokasi |
|---|---|
| Kerangka NoAI | `src/ai/` — `ai_core.cpp` (loop utama AICore), `ai_instance.cpp` (AIInstance/Squirrel VM), `ai_scanner.cpp` (pindai direktori `ai/`), `ai_gui.cpp` (pilih/konfigurasi), `ai_config.cpp` |
| Kerangka GameScript | `src/game/` — `game_core.cpp`, `game_instance.cpp` dll. (GS adalah "mode dewa", hak akses lebih tinggi dari AI) |
| Kelas API script | `src/script/api/script_*.hpp/.cpp` (`script_company`, `script_map`, `script_vehicle`, `script_industry`, `script_town`, `script_game`, `script_admin` dll., total 60+ kelas) |
| Registrasi otomatis API | `src/script/api/CMakeLists.txt` — `file(GLOB script_*.hpp)` secara otomatis menghasilkan binding `ai_*.sq.hpp`/`gs_*.sq.hpp`; `.cpp` perlu ditambahkan ke daftar sumber (mulai baris 235) |
| Makro binding Squirrel | `src/script/squirrel_class.hpp` — `DefSQClass` / `DefSQStaticMethod`; `ai/ai_controller.sq.hpp` adalah binding pengontrol AI |
| Penjadwalan instance | `src/script/script_instance.cpp`, `src/script/script_suspend.hpp` (suspend/resume), peristiwa `script_event*` |
| Kontrol akses perusahaan | `src/script/api/script_object.hpp:318` — `ScriptObject::GetCompany()`; `ScriptCompanyMode` (`IsDeity()`) membedakan mode dewa GS; validasi parameter API perusahaan `ResolveCompanyID`, `EnforceCompanyModeValid` |
| Pengaturan script | `src/table/settings/script_settings.ini` (`game.script.*`); konfigurasi instance AI `ai_config.cpp` |

**Kesimpulan status saat ini**: Dalam script API modern, beberapa informasi pesaing (seperti `GetBankBalance`) sudah tidak dibatasi, tetapi **tidak ada API agregasi yang ditujukan untuk kesadaran seluruh permainan** (tidak ada pintu masuk terpadu untuk "menghitung semua perusahaan/ekonomi global/statistik peta global"), juga tidak ada titik akses "AI global" yang dikontrol sakelar. Menambahkan kelas API `ScriptGlobal` (registrasi otomatis GLOB) + pengaturan `game.script` yang "mengizinkan kesadaran global AI" sudah cukup untuk mewujudkan, GS selalu tersedia (mode dewa), AI dikontrol sakelar——yaitu "mempertahankan NoAI, menambahkan AI sadar permainan penuh, dengan kontrol akses".

---

## 5. Ikhtisar Risiko Modifikasi

| Fitur | File Perubahan Utama | Risiko |
|---|---|---|
| F1 Unduhan multi-sumber/multi-thread | `src/table/settings/network_settings.ini`, `src/settings_type.h`, `src/network/core/config.cpp`, `src/network/network_content.h/.cpp` | Sedang (mesin status callback jaringan perlu hati-hati) |
| F2 Perluasan batas | `src/company_type.h`, `src/table/settings/network_settings.ini`, `src/network/core/network_game_info.cpp` (opsional uint16) | Rendah-Sedang (perusahaan 252 risiko rendah; klien >255 perlu perubahan protokol) |
| F3 Pengelompokan kendaraan otomatis | `src/group_cmd.h/.cpp`, `src/command_type.h`, `src/console_cmds.cpp`, `src/group_gui.cpp`, `src/lang/english.txt` | Rendah |
| F4 Tooltip bangunan | Baru `src/construction_cost_tip.h/.cpp`, kait `src/rail_gui.cpp`/`road_gui.cpp`, `src/lang/english.txt` | Rendah-Sedang |
| F5 AI sadar permainan penuh | Baru `src/script/api/script_global.hpp/.cpp`, `src/script/api/CMakeLists.txt`, `src/table/settings/script_settings.ini`, contoh AI `bin/ai/GlobalAI/` | Rendah (registrasi otomatis framework) |

> Catatan: Semua perubahan didasarkan pada workspace lokal cabang `jgrpp`, belum diverifikasi kompilasi (mesin ini tidak memiliki toolchain build yang tersedia); semua patch dapat di `git diff` review lalu `git apply`.