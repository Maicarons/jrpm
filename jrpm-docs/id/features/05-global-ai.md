---
title: Antarmuka AI Sadar Permainan Penuh
---

## Status Saat Ini (Kesimpulan Riset)

- Kerangka NoAI: `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config)——Sistem AI asli **dipertahankan apa adanya**, fitur ini tidak menyentuhnya;
- Sistem API script: `src/script/api/script_*.hpp/.cpp` (60+ kelas), pengikatan Squirrel **dihasilkan secara otomatis** oleh alat build:
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` secara otomatis menemukan kelas API baru (menghasilkan `ai_*.sq.hpp` / `gs_*.sq.hpp`), **kelas baru tidak perlu registrasi manual**;
  - `.cpp` harus ditambahkan ke daftar sumber CMake; komentar kelas `@api ai game` mengontrol eksposur ke AI/GS;
- Data perusahaan: `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy)、`GetAvailableMoney()`；
- Mode dewa GS: `ScriptCompanyMode::IsDeity()`；
- Batasan saat ini: AI secara default hanya dapat mengakses data perusahaannya sendiri dengan andal, tidak ada API agregasi seluruh permainan, tidak ada sakelar akses.

**Kesimpulan**: Menambahkan kelas API `ScriptGlobal` (registrasi otomatis) + sakelar `game.script.allow_global_ai_access` sudah cukup untuk mewujudkan "mempertahankan NoAI, menambahkan AI sadar permainan penuh, dengan kontrol akses".

## Implementasi Fitur Ini

### 1. API `ScriptGlobal` (`src/script/api/script_global.hpp/.cpp`)

Metode statis yang diekspos ke AI dan GS (`@api ai game`):

| Metode | Kembalian | Sumber Data |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| sakelar pengaturan` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`、`GetDate()`、`GetYear()` | int | `MapSizeX/Y`、`EconTime::CurDate`、`CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` hitung berdasarkan pemilik |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- ID perusahaan menggunakan kembali `ScriptCompany::CompanyID` (COMPANY_SELF diselesaikan ke perusahaan saat ini);
- **Kontrol akses**: Semua metode memeriksa `IsGlobalAccessAllowed()` di baris pertama, jika tidak terpenuhi kembalikan -1/nullopt/false (GS selalu tersedia, AI dikontrol sakelar);
- `ScriptGlobalCompanyList : ScriptList`: `ScriptList::FillList<Company>` menghitung semua perusahaan.

### 2. Sakelar Akses (Pengaturan)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`, default false, `SC_EXPERT`);
- Bidang struct `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`);
- String `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`).

### 3. AI Contoh: `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController);
- Demonstrasi: Membaca data keuangan/kendaraan/stasiun/peringkat dan peta semua perusahaan dan mencatat log; menjelaskan kontrol akses (peringatan saat sakelar tidak diaktifkan);
- AI ini adalah implementasi referensi untuk "merasakan seluruh permainan + kontrol akses", logika kontrol AI selanjutnya dapat diperluas dalam `Start()`.

### File Terkait

- Baru `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` (tambah .cpp)
- `src/table/settings/script_settings.ini`, `src/settings_type.h`, `src/lang/english.txt`
- Baru `bin/ai/GlobalAI/info.nut`, `main.nut`

## Poin Verifikasi

1. AI baru terlihat di antarmuka konfigurasi AI (pindai `ai/GlobalAI`);
2. Saat `game.script.allow_global_ai_access` tidak diaktifkan, log AI menunjukkan tidak ada izin; setelah diaktifkan, output data lengkap setiap perusahaan;
3. GameScript tidak memerlukan sakelar untuk mengakses menggunakan `GSGlobal`;
4. Pembuatan otomatis `ai_global.sq.hpp` / `gs_global.sq.hpp` tanpa kesalahan (bergantung pada `file(GLOB)`, perlu mengkonfigurasi ulang CMake).

## Arah Pengembangan

- Menambah data agregasi lebih banyak seperti ekonomi/pinjaman/infrastruktur (`GetCompanyInfrastructure`);
- Menambah berlangganan acara (kebangkrutan perusahaan/akuisisi/pendirian perusahaan baru);
- Membuat logika keputusan AI menjadi parameter yang dapat dikonfigurasi (`GetSettings()`).