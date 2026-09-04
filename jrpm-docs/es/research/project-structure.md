---
title: Investigacion de estructura del proyecto y organizacion del codigo
---

> Objeto de investigacion: `G:\GitHub\OpenTTD-patches` (area de trabajo de codigo fuente de jgrpp)
> Recursos de referencia: `G:\game\openttd-jgrpp` (juego completo compilado, solo lectura)
> Fecha de investigacion: 2026-08-14
> Rama: jgrpp (HEAD actual `5b5c452e1b`, aproximadamente version 16.0)

---

## 1. Vision general del proyecto

OpenTTD jgrpp (JGR's Patchpack) es una rama de mejora conocida basada en OpenTTD. El codigo fuente de esta area de trabajo corresponde a la version **16.0**, e incluye numerosas caracteristicas privadas de JGR (tracerestrict, senales programables, scheduled dispatch, reemplazo de plantillas, senales mejoradas, etc.).

| Proyecto | Descripcion |
|---|---|
| Lenguaje | C++20 (`CMAKE_CXX_STANDARD 20`, sin extensiones) |
| Construccion | CMake (≥3.17), `src/CMakeLists.txt` organizado por directorio |
| Motor de scripts | Squirrel (`src/3rdparty/squirrel/`), usado para AI / GameScript / scripts de plantillas |
| Red | Capa de protocolo TCP/UDP propia (`src/network/`), HTTP con WinHttp (Windows) / libcurl (otros) / JS (Emscripten) |
| Archivo | `src/saveload/` + `src/sl/` formato binario propio |
| Hilos | `src/thread.h` + `src/worker_thread.cpp` (pool de tareas WorkerThreadPool), `src/timer/` temporizadores |

---

## 2. Estructura del directorio de codigo fuente y division de modulos

Nivel superior: `CMakeLists.txt` es el script de construccion raiz; `src/` contiene todo el codigo fuente C++; `bin/` contiene datos de ejecucion (scripts de AI compatibles, archivos de idioma, etc.); `media/`, `os/`, `cmake/`, `docs/` contienen recursos, codigo de plataforma, scripts de construccion y documentacion respectivamente.

Subdirectorios principales dentro de `src/` y sus responsabilidades:

| Directorio | Responsabilidad |
|---|---|
| `src/core/` | Herramientas basicas: tipos pool, bitset, contenedores, encapsulacion de hilos, operaciones de bits, etc. |
| `src/network/` | Red: servidor/cliente/UDP/HTTP/descarga de contenido/protocolo de administrador (subdirectorio `core/` es la base del protocolo) |
| `src/script/` | Marco de ejecucion de scripts: encapsulacion de Squirrel, instancias, configuracion; `api/` contiene todas las clases de API expuestas a AI/GS |
| `src/ai/` | Marco NoAI (instancias de AI, escaner, configuracion, GUI) |
| `src/game/` | Marco GameScript (instancias de GS, configuracion, GUI) |
| `src/newgrf/` | Decodificacion y procesamiento de NewGRF |
| `src/pathfinder/` | Busqueda de rutas (YAPF, NPF) |
| `src/saveload/`, `src/sl/` | Lectura/escritura de archivos guardados |
| `src/blitter/`, `src/video/`, `src/fontcache/`, `src/music/`, `src/sound/` | Backends de renderizado/audio/video |
| `src/lang/` | Cadenas de localizacion (generadas por strgen) |
| `src/table/` | Tablas estaticas; `table/settings/*.ini` son **fuentes de definicion de configuracion** (generadas por settingsgen) |
| `src/3rdparty/` | Bibliotecas de terceros (squirrel, llvm, icu, etc.) |
| `src/timer/`, `src/os/`, `src/misc/` | Temporizadores, plataforma, varios |

Los archivos dispersos en el nivel superior se nombran por sistema (ej. `rail_gui.cpp`, `group_cmd.cpp`, `vehicle.cpp`, `order_cmd.cpp`, `economy.cpp`), siguiendo la convencion de OpenTTD: `*_cmd` logica de comandos, `*_gui` ventanas, `*_base/_type/_func` estructuras de datos y funciones inline.

---

## 3. Metodo de construccion

- **Tres pasos de CMake**: `cmake -B build ..` → `cmake --build build` → resultado `openttd.exe`. El repositorio incluye `build.sh` / `build-dedicated.sh`.
- **Dependencias**: Declaradas en `vcpkg.json` (zlib, lzma, lzo, zstd, png, SDL2, freetype, harfbuzz, icu, opus, etc.); Windows usa WinHttp (sin curl), no Windows usa libcurl (`CMakeLists.txt:121-127`).
- **Cadena de herramientas (host tools)**: `strgen` (archivos de idioma), `settingsgen` (genera codigo de configuracion desde `src/table/settings/*.ini`), `squirrel_export` (genera enlaces de Squirrel desde `src/script/api/script_*.hpp`).
- **Productos clave generados**: `generated/script/api/<ai|gs>/...sq.hpp` (enlaces de API, **descubiertos automaticamente por `file(GLOB script_*.hpp)`, no es necesario modificar la lista de registro para nuevas clases de API**, solo anadir `.hpp` e incluir `.cpp` en la lista de fuentes de `src/script/api/CMakeLists.txt`); `generated/rev.cpp`; `generated/ottdres.rc`.
- **Sistema de configuracion**: La version moderna ha cambiado a **controlado por INI** -- `src/table/settings/*.ini` (incluye secciones `[SDTC_VAR]`, `cat=SC_*` clasificacion, `flags`, `post_cb`, etc.), settingsgen genera `settings_*.cpp/h`; los miembros de estructura correspondientes estan en `src/settings_type.h` (ej. `NetworkSettings` desde la linea 575).

---

## 4. Ubicaciones de implementacion de los cinco sistemas funcionales principales

### 4.1 Descarga de recursos (descarga de contenido / BaNaNaSplit)

| Punto de interes | Ubicacion |
|---|---|
| Clase principal del cliente de contenido | `src/network/network_content.h/.cpp` -- `ClientNetworkContentSocketHandler` (tambien `ContentCallback` + `HTTPCallback`) |
| GUI de descarga de contenido | `src/network/network_content_gui.cpp/.h` |
| Cliente HTTP | `src/network/core/http.h/.cpp` -- `NetworkHTTPSocketHandler::Connect(uri, callback, data)`, asincrono basado en eventos (no bloqueante, sondeo en bucle principal) |
| Cadena de conexion del servidor de contenido | `src/network/core/config.cpp` -- `NetworkContentServerConnectionString()`: variable de entorno `OTTD_CONTENT_SERVER_CS`, predeterminado `content.openttd.org` (protocolo de metadatos TCP) |
| URI de espejo | `src/network/core/config.cpp` -- `NetworkContentMirrorUriString()`: variable de entorno `OTTD_CONTENT_MIRROR_URI`, predeterminado `https://binaries.openttd.org/bananas` |
| Flujo de descarga | `DownloadSelectedContent()` → `DownloadSelectedContentHTTP()` (POST de todos los content ID al espejo, el espejo devuelve un flujo tar de multiples archivos, escribe cada archivo en disco) → `AfterDownload()` hace gunzip + `TarScanner` desempaquetado |
| Descompresion | `GunzipFile()` (zlib), `TarScanner`/`ExtractTar` (`src/tar_type.h` / `src/fileio.cpp`) |
| Infraestructura de hilos (utilizable para paralelismo) | `src/worker_thread.h/.cpp` -- `WorkerThreadPool` + `EnqueueJob`; `src/thread.h` encapsulacion de hilos de plataforma |

**Conclusion del estado actual**: ① Solo hay 1 espejo y solo se puede configurar mediante variable de entorno, sin configuracion en el juego; ② La descarga es **de unica conexion, secuencial** (un POST para todos los archivos); ③ La descompresion se ejecuta de forma sincrona en el hilo principal. → Los puntos de modificacion para multiproceso/multiples espejos son claros.

### 4.2 Limite de jugadores en linea / empresas del servidor

| Punto de interes | Ubicacion |
|---|---|
| Constante de limite de clientes | `src/network/network_type.h:21` -- `static const uint MAX_CLIENTS = 255;` |
| Pool de clientes | Mismo archivo `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>`; `ClientID` es `uint32_t` |
| Pool de IDs de empresa | `src/company_type.h` -- `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15`; empresas falsas ocupan 253/254/255 |
| Mascara de empresa | Mismo archivo `CompanyMask : BaseBitSet<CompanyMask, CompanyID, uint16_t>` (16 bits, solo puede rastrear 16 empresas) |
| Verificacion de aceptacion del servidor | `src/network/network_server.cpp:360` -- `_network_clients_connected < MAX_CLIENTS`; `static_assert(NetworkClientSocketPool::MAX_SIZE == MAX_CLIENTS + 1)` |
| Elementos de configuracion de cliente | `src/table/settings/network_settings.ini:231/241` -- `network.max_companies` (def 15, max MAX_COMPANIES), `network.max_clients` (def 25, max MAX_CLIENTS); estructura en `src/settings_type.h` `NetworkSettings` |
| **Ancho de bits del protocolo (restriccion estricta)** | `src/network/core/network_game_info.cpp` -- `companies_max` y `clients_max` se envian como **`Send_uint8`/`Recv_uint8`** (lineas 251-296, 422-432) |
| Visualizacion de lista de servidores | `src/network/network_gui.cpp:519` etc. |

**Conclusion del estado actual**:
- **Limite de clientes = 255 es el limite del protocolo** (campo uint8 + compatibilidad con Game Coordinator/navegador de servidores). Para superarlo, los campos relacionados de `network_game_info` deben cambiarse a uint16 (toda la cadena cliente↔servidor + transmision UDP + protocolo de Game Coordinator), lo que constituye un cambio de protocolo externo.
- **Limite de empresas = 15** (`CompanyIDTag` con End=0xF). Se puede aumentar de forma segura a **252** (End=0xFC): la capa subyacente sigue siendo uint8, el ancho de bytes del archivo no cambia (compatible con archivos antiguos), es necesario ampliar simultaneamente `CompanyMask` (uint16→uint32) y las suposiciones de interfaz/bucle.

### 4.3 Agrupacion de vehiculos

| Punto de interes | Ubicacion |
|---|---|
| Estructura de datos de grupo | `src/group.h` -- `Group : GroupPool::PoolItem` (name/owner/vehicle_type/flags/livery/statistics/parent/number); `GroupID`, `DEFAULT_GROUP`, `IsDefaultGroupID/IsAllGroupID/IsTopLevelGroupID` |
| Comandos de grupo | `src/group_cmd.cpp` -- `CmdCreateGroup`(536), `CmdDeleteGroup`(585), `CmdAlterGroup`(646), `CmdAddVehicleGroup`, `CmdAddSharedVehicleGroup` (anadir vehiculos de ordenes compartidas a un grupo existente, cerca de la linea 718 `AddVehicleToGroup`) |
| Registro de comandos | `src/group_cmd.h:27-35` -- `DEF_CMD_TUPLE_NT(Commands::XXX, CmdXXX, {}, CommandType::RouteManagement, CmdDataT<...>)`; enumeracion en `src/command_type.h` `enum class Commands` (desde linea 492) |
| GUI de grupos | `src/group_gui.cpp/.h`, `src/vehiclelist.cpp` |
| Vehiculo↔Grupo | `src/vehicle_base.h` (`Vehicle::group_id`), `SetTrainGroupID/UpdateTrainGroupID` (group.h:130-131) |
| Ordenes/despacho compartido | `src/order_base.h` (`OrderList`, `VehicleOrdersID`), `src/order_cmd.cpp`, `src/order_func.h`, `src/schdispatch.h/.cpp` (scheduled dispatch, vinculado a order list) |
| Estadisticas de grupo | `GroupStatistics` (group.h:60-66), `GetGroupNumVehicle` etc. (group.h:125-128) |

**Conclusion del estado actual**: Ya existen `CmdAddSharedVehicleGroup` (anadir vehiculos de ordenes compartidas de un vehiculo a un grupo) y `CmdCreateGroupFromList` (crear grupo desde una lista), pero **no hay una logica completa de "crear/agrupar automaticamente por ordenes compartidas"**. El nuevo comando `AutoGroupSharedOrders` (recorrer todos los vehiculos principales de la empresa → agregar por `OrderList` → crear grupo y agrupar automaticamente) tiene un camino de implementacion claro.

### 4.4 Tooltip de construccion (indicacion de precio sobre el cursor al construir vias, etc.)

| Punto de interes | Ubicacion |
|---|---|
| GUI/logica de construccion de vias | `src/rail_gui.cpp` (`BuildRailToolbarWindow`), `src/rail_cmd.cpp`, `src/rail.h/.cpp`; carreteras `road_gui.cpp/road_cmd.cpp` |
| Estimacion de costos | Cada `*_cmd.cpp` tiene `DoCommand` que devuelve `CommandCost`; en la GUI se puede usar el modo `DC_QUERY_COST` para consultar precios |
| Tile bajo el cursor | `src/viewport_func.h:36` -- `GetTileBelowCursor()`; `_cursor.pos` (coordenadas de pantalla); `src/viewport.cpp:1056` |
| Mecanismo de texto indicador existente | `src/texteff.hpp` -- `AddTextEffect(msg, x, y, duration, mode, ...)` (texto flotante en coordenadas del mundo), `UpdateTextEffect`; `src/texteff.cpp` |
| UX de indicacion de construccion existente | La barra de herramientas de via `OnPlaceDrag` tiene area seleccionada y acumulacion de costos durante el arrastre (`_thd` tilehighlight, `src/tilehighlight_func.h`); la barra de estado `statusbar_gui.cpp` puede mostrar el costo de la herramienta |
| Punto de actualizacion por frame | Cada ventana de barra de herramientas `OnMouseLoop` / `viewport.cpp` `HandleMouseEvents` (lineas 5422/5733) |

**Conclusion del estado actual**: No hay "tooltip de precio sobre el cursor". Se puede implementar con `AddTextEffect` anclado al tile del cursor (sigue la casilla del cursor), o dibujando un tooltip en coordenadas de pantalla; el costo se puede consultar con `DC_QUERY_COST` para un solo tile de la herramienta actual + acumulacion del area de arrastre.

### 4.5 Interfaz de AI (conservar NoAI + IA de percepcion global)

| Punto de interes | Ubicacion |
|---|---|
| Framework NoAI | `src/ai/` -- `ai_core.cpp` (bucle principal de AICore), `ai_instance.cpp` (AIInstance/VM de Squirrel), `ai_scanner.cpp` (escanea directorio `ai/`), `ai_gui.cpp` (seleccion/configuracion), `ai_config.cpp` |
| Framework GameScript | `src/game/` -- `game_core.cpp`, `game_instance.cpp` etc. (GS es "modo deidad", permisos superiores a AI) |
| Clases de API de scripts | `src/script/api/script_*.hpp/.cpp` (`script_company`, `script_map`, `script_vehicle`, `script_industry`, `script_town`, `script_game`, `script_admin`, etc., 60+ clases) |
| Registro automatico de API | `src/script/api/CMakeLists.txt` -- `file(GLOB script_*.hpp)` genera automaticamente enlaces `ai_*.sq.hpp`/`gs_*.sq.hpp`; los `.cpp` deben anadirse a la lista de fuentes (desde linea 235) |
| Macros de enlace de Squirrel | `src/script/squirrel_class.hpp` -- `DefSQClass` / `DefSQStaticMethod`; `ai/ai_controller.sq.hpp` es el enlace del controlador de AI |
| Despacho de instancias | `src/script/script_instance.cpp`, `src/script/script_suspend.hpp` (suspender/reanudar), eventos `script_event*` |
| Control de acceso a empresas | `src/script/api/script_object.hpp:318` -- `ScriptObject::GetCompany()`; `ScriptCompanyMode` (`IsDeity()`) distingue el modo deidad de GS; validacion de parametros de API de empresa `ResolveCompanyID`, `EnforceCompanyModeValid` |
| Configuracion de scripts | `src/table/settings/script_settings.ini` (`game.script.*`); configuracion de instancia de AI `ai_config.cpp` |

**Conclusion del estado actual**: En la API de scripts moderna, parte de la informacion de los competidores (como `GetBankBalance`) ya no tiene restricciones, pero **no hay una API de agregacion orientada a la percepcion global** (sin punto de entrada unificado para "enumerar todas las empresas/economia global/estadisticas globales del mapa"), ni un punto de acceso de "AI global" controlado por interruptor. Anadir una nueva clase de API `ScriptGlobal` (registro automatico GLOB) + una configuracion `game.script` de "permitir percepcion global de AI" es suficiente para lograrlo, GS siempre disponible (modo deidad), AI controlada por interruptor -- es decir, "conservar NoAI, anadir nueva IA de percepcion global, con control de acceso".

---

## 5. Resumen de riesgos de modificacion

| Funcion | Archivos de modificacion principales | Riesgo |
|---|---|---|
| F1 Descarga multi-fuente/multihilo | `src/table/settings/network_settings.ini`, `src/settings_type.h`, `src/network/core/config.cpp`, `src/network/network_content.h/.cpp` | Medio (la maquina de estados de callback de red requiere cuidado) |
| F2 Extension de limites | `src/company_type.h`, `src/table/settings/network_settings.ini`, `src/network/core/network_game_info.cpp` (opcional uint16) | Bajo-Medio (empresa 252 riesgo bajo; cliente >255 requiere cambio de protocolo) |
| F3 Agrupacion automatica de vehiculos | `src/group_cmd.h/.cpp`, `src/command_type.h`, `src/console_cmds.cpp`, `src/group_gui.cpp`, `src/lang/english.txt` | Bajo |
| F4 Tooltip de construccion | Nuevo `src/construction_cost_tip.h/.cpp`, `src/rail_gui.cpp`/`road_gui.cpp` ganchos, `src/lang/english.txt` | Bajo-Medio |
| F5 IA de percepcion global | Nuevo `src/script/api/script_global.hpp/.cpp`, `src/script/api/CMakeLists.txt`, `src/table/settings/script_settings.ini`, AI de ejemplo `bin/ai/GlobalAI/` | Bajo (registro automatico del framework) |

> Nota: Todos los cambios se basan en el area de trabajo local de la rama `jgrpp`, sin compilacion verificada (no hay cadena de herramientas de construccion disponible en esta maquina); todos los parches se pueden revisar con `git diff` y aplicar con `git apply`.