---
title: Menambah Item Pengaturan
---

# Menambah Item Pengaturan

Sistem pengaturan jrpm adalah **berbasis INI**: `src/table/settings/*.ini` adalah sumber definisi pengaturan, kode dihasilkan oleh settingsgen pada saat build; bidang struct terkait ada di `src/settings_type.h`. Dengan `network.content_download_parallel` yang diimplementasikan dalam proyek ini sebagai contoh.

## 1. Bidang Struct

`src/settings_type.h`, temukan struct pengaturan yang sesuai (jaringan → `NetworkSettings`, game → sub-struct terkait `GameSettings`, script → `ScriptSettings`):

```cpp
struct NetworkSettings {
    // ...
    std::string content_mirrors;                          ///< URI cermin dipisahkan koma
    uint8_t content_download_parallel = 4;                ///< Jumlah file unduhan paralel
    // ...
};
```

## 2. Definisi INI

`src/table/settings/network_settings.ini`, tambah blok `[SDTC_VAR]` (pengaturan klien gunakan `SDTC_*`, pengaturan game gunakan `SDT_*`):

```ini
[SDTC_VAR]
var      = network.content_download_parallel
type     = SLE_UINT8
flags    = SettingFlag::NotInSave, SettingFlag::NoNetworkSync
def      = 4
min      = 1
max      = 8
cat      = SC_BASIC
```

Bidang umum:

| Bidang | Deskripsi |
|---|---|
| `var` | Nama lengkap pengaturan (jalur struct, mis. `network.xxx` / `game.script.xxx`) |
| `type` | `SLE_UINT8/16/32/64`, `SLE_INT*`, `SLE_BOOL`, `SLE_STR` (string, perlu `length`) |
| `flags` | `SettingFlag::NotInSave` (tidak masuk arsip), `NoNetworkSync` (tidak sinkron), `NetworkOnly`, `GuiZeroIsSpecial`, dll. |
| `def/min/max/interval` | Nilai default/rentang/langkah |
| `str/strhelp` | String tampilan antarmuka pengaturan |
| `cat` | Kategori pengaturan (`SC_BASIC`/`SC_EXPERT`/`SC_ADVANCED`) |

## 3. String (Tampilan Antarmuka Pengaturan)

`src/lang/english.txt`:

```txt
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS   :Allow AIs to access whole-game data: {STRING2}
STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS_HELPTEXT :Allow AIs to use the Global API...
```

- Pengaturan boolean diakhiri dengan `{STRING2}` (nyala/mati);
- Pengaturan numerik lihat format string sejenis.

## 4. Penggunaan dalam Kode

```cpp
// Pengaturan klien
_settings_client.network.content_download_parallel

// Pengaturan game
_settings_game.script.allow_global_ai_access
```

## 5. Skenario Khusus

- **Pengaturan server**: `network.*` dalam game multiplayer dikirim oleh server (`NetworkOnly` + mekanisme `sync` server);
- **Terkait arsip**: Pengaturan game (`game.*`) disimpan dengan arsip (default); pengaturan klien (`network.*`/`gui.*`) default `NotInSave`;
- **Callback**: `pre_cb`/`post_cb` dapat dikaitkan dengan penanganan perubahan nilai (mis. `UpdateClientConfigValues()`).

## Daftar Periksa

- [ ] Bidang `settings_type.h`
- [ ] Blok `[SDT*_VAR]` / `[SDT_BOOL]` / `[SDTC_SSTR]` di `.ini` terkait
- [ ] String english.txt (untuk tampilan antarmuka pengaturan)
- [ ] Konfigurasi ulang + build