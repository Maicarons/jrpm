---
title: Menambah API Script
---

# Menambah API Script

Dengan `ScriptGlobal` (API AI sadar permainan penuh) yang diimplementasikan dalam proyek ini sebagai templat, memperkenalkan langkah lengkap untuk menambah kelas API script baru (terlihat oleh AI dan GameScript) di jrpm.

## Latar Belakang

- Kelas API script terletak di `src/script/api/`: `script_<nama>.hpp` (deklarasi) + `script_<nama>.cpp` (implementasi);
- `.hpp` **ditemukan secara otomatis** oleh `file(GLOB script_*.hpp)` pada saat build dan menghasilkan binding Squirrel (`ai_*.sq.hpp` / `gs_*.sq.hpp`), **tidak perlu registrasi manual kelas**;
- `.cpp` perlu ditambahkan ke daftar sumber `src/script/api/CMakeLists.txt`.

## 1. Header `script_global.hpp`

```cpp
/** @file script_global.hpp Dokumentasi. */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * Dokumentasi kelas, harus menandai @api.
 * @api ai game        # Diekspos ke AI dan GS; "game" hanya GS; "-ai" mengecualikan AI
 */
class ScriptGlobal : public ScriptObject {
public:
    /** Enum akan diekspor sebagai konstanta kelas (mis. AIGlobal.VT_TRAIN). */
    enum VehicleType {
        VT_TRAIN = ::VehicleType::Train,
        VT_AIRCRAFT = ::VehicleType::Aircraft,
    };

    /** @api ai game */
    static bool IsGlobalAccessAllowed();

    /** @api ai game */
    static SQInteger GetCompanyCount();

    /** @api ai game */
    static std::optional<std::string> GetCompanyName(ScriptCompany::CompanyID company);
};

#endif /* SCRIPT_GLOBAL_HPP */
```

## 2. Implementasi `script_global.cpp`

```cpp
#include "../../stdafx.h"
#include "script_global.hpp"
#include "../../company_base.h"
// ...

/* static */ SQInteger ScriptGlobal::GetCompanyCount()
{
    if (!IsGlobalAccessAllowed()) return 0;
    SQInteger count = 0;
    for (const Company *c : Company::Iterate()) count++;
    return count;
}
```

Poin penting:
- Metode semuanya `static`, dengan prefiks `/* static */` saat definisi;
- Tipe nilai kembali menggunakan tipe ramah Squirrel: `SQInteger`, `bool`, `Money`, `std::optional<std::string>`, `std::string`, `ScriptList*`, dll.;
- Kontrol akses: Periksa `ScriptCompanyMode::IsDeity() || sakelar pengaturan` di dalam metode, jika tidak terpenuhi kembalikan nilai error.

## 3. Kelas Daftar (Opsional)

Saat perlu mengembalikan daftar perusahaan/barang, definisikan subkelas `ScriptList`:

```cpp
class ScriptGlobalCompanyList : public ScriptList {
public:
#ifdef DOXYGEN_API
    ScriptGlobalCompanyList();
#else
    ScriptGlobalCompanyList(HSQUIRRELVM vm);
#endif
};
```

```cpp
ScriptGlobalCompanyList::ScriptGlobalCompanyList(HSQUIRRELVM vm)
{
    ScriptList::FillList<Company>(vm, this);
}
```

## 4. Registrasi ke Build

`src/script/api/CMakeLists.txt` daftar sumber tambah:

```cmake
script_global.cpp
```

::: tip
`.hpp` tidak perlu registrasi (GLOB menemukan otomatis); tetapi **konfigurasi ulang CMake** diperlukan untuk melihat binding yang dihasilkan dari `.hpp` baru.
:::

## 5. Penggunaan dalam Script

```js
// Sisi AI: prefiks kelas AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// Sisi GS: prefiks GS
GSGlobal.GetCompanyCount();
```

## Daftar Periksa

- [ ] `script_<nama>.hpp` (tandai `@api`)
- [ ] `script_<nama>.cpp` (implementasi `/* static */`)
- [ ] `CMakeLists.txt` tambah `.cpp`
- [ ] Konfigurasi ulang + build
- [ ] (Opsional) Script contoh `bin/ai/`