---
title: Interfaz de IA de percepcion global
---

## Estado actual (conclusion de la investigacion)

- Framework NoAI: `src/ai/` (ai_core/ai_instance/ai_scanner/ai_gui/ai_config) -- el sistema de IA original se **conserva tal cual**, esta funcion no lo modifica;
- Sistema de API de scripts: `src/script/api/script_*.hpp/.cpp` (60+ clases), el enlace Squirrel es **generado automaticamente** por herramientas de construccion:
  - `src/script/api/CMakeLists.txt` `file(GLOB script_*.hpp)` descubre automaticamente nuevas clases de API (genera `ai_*.sq.hpp` / `gs_*.sq.hpp`), **no se necesita registro manual para nuevas clases**;
  - Los `.cpp` deben anadirse a la lista de fuentes de CMake; el comentario de clase `@api ai game` controla la exposicion a AI/GS;
- Datos de empresa: `company_base.h` `Company` (money/current_loan/old_economy[quarter] (company_value, performance_history)/group_all[type].num_vehicle/months_of_bankruptcy), `GetAvailableMoney()`;
- Modo deidad de GS: `ScriptCompanyMode::IsDeity()`;
- Limitacion actual: Por defecto, la IA solo puede acceder de forma fiable a los datos de su propia empresa, no hay API de agregacion global ni interruptor de acceso.

**Conclusion**: Anadir una nueva clase de API `ScriptGlobal` (registro automatico) + un interruptor `game.script.allow_global_ai_access` es suficiente para lograr "conservar NoAI, anadir nueva IA de percepcion global, con control de acceso".

## Implementacion de esta funcion

### 1. API `ScriptGlobal` (`src/script/api/script_global.hpp/.cpp`)

Metodos estaticos expuestos a AI y GS (`@api ai game`):

| Metodo | Retorno | Fuente de datos |
|---|---|---|
| `IsGlobalAccessAllowed()` | bool | `ScriptCompanyMode::IsDeity() \|\| interruptor de configuracion` |
| `GetCompanyCount()` | int | `Company::Iterate()` |
| `GetMapSizeX/Y()`, `GetDate()`, `GetYear()` | int | `MapSizeX/Y`, `EconTime::CurDate`, `CalTime::CurYear` |
| `GetCompanyName(id)` | string? | `STR_COMPANY_NAME` |
| `GetCompanyBankBalance(id)` | Money | `GetAvailableMoney` |
| `GetCompanyLoan(id)` | Money | `current_loan` |
| `GetCompanyValue(id)` | Money | `old_economy[0].company_value` |
| `GetCompanyPerformanceRating(id)` | int | `old_economy[0].performance_history` |
| `GetCompanyVehicleCount(id, vt)` | int | `group_all[vt].num_vehicle` (VT_TRAIN/ROAD/SHIP/AIRCRAFT) |
| `GetCompanyStationCount(id)` | int | `Station::Iterate()` contando por owner |
| `IsCompanyBankrupt(id)` | bool | `months_of_bankruptcy != 0` |

- El ID de empresa reutiliza `ScriptCompany::CompanyID` (COMPANY_SELF se resuelve a la empresa actual);
- **Control de acceso**: todos los metodos verifican `IsGlobalAccessAllowed()` al inicio, si no se cumple devuelven -1/nullopt/false (GS siempre disponible, AI controlado por interruptor);
- `ScriptGlobalCompanyList : ScriptList`: `ScriptList::FillList<Company>` enumera todas las empresas.

### 2. Interruptor de acceso (configuracion)

- `game.script.allow_global_ai_access` (`src/table/settings/script_settings.ini` `[SDT_BOOL]`, predeterminado false, `SC_EXPERT`);
- Campo de estructura `ScriptSettings::allow_global_ai_access` (`src/settings_type.h`);
- Cadena `STR_CONFIG_SETTING_ALLOW_GLOBAL_AI_ACCESS[_HELPTEXT]` (`src/lang/english.txt`).

### 3. AI de ejemplo: `bin/ai/GlobalAI/`

- `info.nut` (GlobalAIInfo) + `main.nut` (GlobalAI : AIController);
- Demostracion: lee y registra las finanzas/vehiculos/estaciones/clasificaciones e informacion del mapa de todas las empresas; explica el control de acceso (aviso cuando el interruptor no esta activado);
- Esta IA es la implementacion de referencia de "percepcion global del juego + control de acceso", la logica de control de la IA se puede ampliar en su metodo `Start()`.

### Archivos involucrados

- Nuevos `src/script/api/script_global.hpp/.cpp` + `src/script/api/CMakeLists.txt` (anadir .cpp)
- `src/table/settings/script_settings.ini`, `src/settings_type.h`, `src/lang/english.txt`
- Nuevos `bin/ai/GlobalAI/info.nut`, `main.nut`

## Puntos de verificacion

1. La nueva AI es visible en la interfaz de configuracion de AI (escanea `ai/GlobalAI`);
2. Cuando `game.script.allow_global_ai_access` no esta activado, el registro de la AI indica que no hay permiso; al activarlo, muestra los datos completos de cada empresa;
3. GameScript puede acceder a `GSGlobal` sin necesidad del interruptor;
4. La generacion automatica de `ai_global.sq.hpp` / `gs_global.sq.hpp` durante la construccion no produce errores (depende de `file(GLOB)`, se necesita reconfigurar CMake).

## Direcciones de extension

- Anadir mas datos agregados como economia/prestamos/infraestructura (`GetCompanyInfrastructure`);
- Anadir suscripcion a eventos (bancarrota de empresa/adquisicion/creacion de nueva empresa);
- Convertir la logica de decision de la IA en parametros configurables (`GetSettings()`).