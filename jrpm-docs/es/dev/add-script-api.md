---
title: Anadir nueva API de script
---

# Anadir nueva API de script

Usando el `ScriptGlobal` (API de IA de percepcion global) implementado en este proyecto como plantilla, se presentan los pasos completos para anadir una nueva clase de API de script (visible para AI y GameScript) en jrpm.

## Antecedentes

- Las clases de API de script se encuentran en `src/script/api/`: `script_<nombre>.hpp` (declaracion) + `script_<nombre>.cpp` (implementacion);
- Los `.hpp` son **descubiertos automaticamente** por `file(GLOB script_*.hpp)` en tiempo de construccion y generan enlaces de Squirrel (`ai_*.sq.hpp` / `gs_*.sq.hpp`), **no es necesario registro manual de la clase**;
- Los `.cpp` deben anadirse a la lista de fuentes de `src/script/api/CMakeLists.txt`.

## 1. Archivo de encabezado `script_global.hpp`

```cpp
/** @file script_global.hpp Documentacion. */
#ifndef SCRIPT_GLOBAL_HPP
#define SCRIPT_GLOBAL_HPP

#include "script_object.hpp"
#include "script_company.hpp"

/**
 * Documentacion de la clase, debe incluir @api.
 * @api ai game        # Expuesto tanto a AI como a GS; "game" solo GS; "-ai" excluye AI
 */
class ScriptGlobal : public ScriptObject {
public:
	/** Las enumeraciones se exportan como constantes de clase (ej. AIGlobal.VT_TRAIN). */
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

## 2. Implementacion `script_global.cpp`

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

Puntos clave:
- Los metodos son todos `static`, prefijo `/* static */` en la definicion;
- Los tipos de retorno usan tipos amigables para Squirrel: `SQInteger`, `bool`, `Money`, `std::optional<std::string>`, `std::string`, `ScriptList*`, etc.;
- Control de acceso: verificar `ScriptCompanyMode::IsDeity() || interruptor de configuracion` dentro del metodo, devolver valor de error si no se cumple.

## 3. Clase de lista (opcional)

Cuando se necesita devolver una lista de empresas/elementos, definir una subclase de `ScriptList`:

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

## 4. Registrar en la construccion

En la lista de fuentes de `src/script/api/CMakeLists.txt` anadir:

```cmake
script_global.cpp
```

::: tip
Los `.hpp` no necesitan registro (GLOB los descubre automaticamente); pero es necesario **reconfigurar CMake** para que vea los nuevos enlaces generados por los `.hpp`.
:::

## 5. Uso en scripts

```js
// Lado AI: prefijo de clase AI
AIGlobal.GetCompanyCount();
AIGlobalCompanyList();
// Lado GS: prefijo GS
GSGlobal.GetCompanyCount();
```

## Lista de verificacion

- [ ] `script_<nombre>.hpp` (con anotacion `@api`)
- [ ] `script_<nombre>.cpp` (implementacion con `/* static */`)
- [ ] Anadir `.cpp` en `CMakeLists.txt`
- [ ] Reconfigurar + construir
- [ ] (Opcional) Script de ejemplo en `bin/ai/`