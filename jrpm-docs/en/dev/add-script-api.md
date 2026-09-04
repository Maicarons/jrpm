---
title: Adding a Script API
---

# Adding a Script API

Using `ScriptGlobal` (whole-game-aware AI API) implemented in this project as a template, this describes the complete steps to add a new script API class (visible to AI and GameScript) in jrpm.

## Background

- Script API classes are located in `src/script/api/`: `script_<name>.hpp` (declaration) + `script_<name>.cpp` (implementation);
- `.hpp` is **auto-discovered** by the build-time `file(GLOB script_*.hpp)` and generates Squirrel bindings (`ai_*.sq.hpp` / `gs_*.sq.hpp`), **no manual class registration needed**;
- `.cpp` must be added to the `src/script/api/CMakeLists.txt` source list.

## 1. Header File `script_global.hpp`

```cpp
/** @file script_global.hpp Documentation comment. */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * Class documentation, must include @api.
 * @api ai game        # Exposed to both AI and GS; "game" for GS only; "-ai" excludes AI
 */
class ScriptGlobal : public ScriptObject {
public:
    /** Enum will be exported as class constants (e.g., AIGlobal.VT_TRAIN). */
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

## 2. Implementation `script_global.cpp`

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

Key points:
- All methods are `static`, defined with `/* static */` prefix;
- Return types use Squirrel-friendly types: `SQInteger`, `bool`, `Money`, `std::optional<std::string>`, `std::string`, `ScriptList*`, etc.;
- Access control: check `ScriptCompanyMode::IsDeity() || setting toggle` inside methods, return error value if not satisfied.

## 3. List Class (Optional)

For returning lists of companies/items, define a `ScriptList` subclass:

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

## 4. Register in Build

Add to `src/script/api/CMakeLists.txt` source list:

```cmake
script_global.cpp
```

::: tip
`.hpp` does not need registration (GLOB auto-discovers); but **reconfigure CMake** to see the bindings generated from the new `.hpp`.
:::

## 5. Usage in Scripts

```js
// AI side: class prefix AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// GS side: prefix GS
GSGlobal.GetCompanyCount();
```

## Completion Checklist

- [ ] `script_<name>.hpp` (with `@api` annotation)
- [ ] `script_<name>.cpp` (implemented with `/* static */`)
- [ ] `CMakeLists.txt` add `.cpp`
- [ ] Reconfigure + build
- [ ] (Optional) Example script in `bin/ai/`