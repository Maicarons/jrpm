---
title: Resumen de la fusion de versiones de jrpm
---

> Rama: `jrpm` ｜ Version: jrpm-0.1.0 (tagged, 2026-08-14)
> Resultado de construccion: `openttd-jrpm` (nombre del ejecutable)

## Relacion de versiones

```
                        jgrpp-0.73.1 (ancestro comun)
                        /                 \
        rama jgrpp (63 commits)          pulsexlb px-patch (152 commits)
        ├ tracerestrict y otras actualizaciones recientes  ├ jgrpp-decouple (desenganche de locomotoras)
        ├ mis 5 funciones (d4c45740)                       └ jgrpp-multitile-airport (aeropuertos modulares)
        └ cambio de nombre jrpm-0.1.0 (425e7207)
                        \                 /
                        rama jrpm (merge 71fe214c + compatibilidad cb9848b)
```

## Contenido de la fusion

### 1. pulsexlb/OpenTTD-patches (px-patch completo 152 commits) → Ya fusionado

| Funcion | Descripcion | Archivos principales |
|---|---|---|
| **Desenganche de locomotoras (decouple)** | Desenganche/enganche de trenes: ordenes de desenganche, transferencia de tokens, limites de longitud/velocidad de acoplamiento, doble locomotora, acoplamiento NewGRF, busqueda de rutas de acoplamiento (YAPF/NPF), despacho independiente de dos trenes tras desenganche | train_cmd.cpp, order_cmd.cpp, order_gui.cpp, train.h, yapf/npf |
| **Aeropuertos modulares (multitile-airport)** | Reestructuracion del sistema de aeropuertos de multiples casillas: sistema de tipos air (air.h/air_type.h/newgrf_airtype.*), despacho aereo PBS (pbs_air.*), busqueda de rutas YAPF para aviacion, `station.allow_modify_airports` (modificar disposicion de aeropuerto), `gui.default_air_type`, sprites de aeropuerto de multiples casillas | air.*, pbs_air.*, aircraft_cmd.cpp (reestructuracion de 3600 lineas), airport_cmd/gui, station_cmd |

Manejo de conflictos: Solo 2 conflictos de archivos de encabezado (aircraft.h / airport.h) -- la reestructuracion de aviacion de pulsexlb elimino tipos muertos **sin referencia** en el area de trabajo (`VehicleAirFlags` bitset, `AirportMovingDataFlag`), se tomo la eliminacion del lado de pulsexlb, se confirmo que ningun otro archivo los referencia.

### 2. Openttd-Cluster (proyecto de cluster Rust de usuario) → Referencia selectiva

| Parche | Manejo | Descripcion |
|---|---|---|
| 0006 vanilla-native-server (compatibilidad multiversion de clientes) | ✅ **Ya fusionado** (cb9848b7) | El servidor jrpm acepta simultaneamente clientes jrpm / jgrpp original (`jgrpp-`) / pulsexlb (`pxp`); la version de NewGRF sigue siendo estrictamente verificada |
| 0001 revision-handshake / 0005 version-metadata | ✅ Idea ya adoptada | jrpm usa cadena de revision tagged independiente `jrpm-0.1.0`, el handshake de conexion esta aislado de jgrpp/pxp, logrando "nueva version facil de conectar" |
| 0007 parallel-download (pool de hilos HTTP + descarga por fragmentos Range, 30KB) | 📝 Referencia, no fusionado | Mismo tema que F1 "paralelismo a nivel de archivo + multiples espejos" y modifica los mismos archivos; el **pool de hilos de capa de transporte/descarga por fragmentos** de 0007 se registra como direccion de mejora futura para F1 |
| 0002-0004 snapshot/command/FFI bridge | 📝 Referencia de arquitectura | Depende de todo el runtime Rust de otc-engine (enlace estatico FFI), pertenece a "integracion general a largo plazo" mas que a fusion a nivel de parche; jrpm actualmente mantiene un unico binario C++ puro |

### 3. Exclusivo de jrpm (5 funciones anteriores, d4c45740) → Ya en la rama jrpm

Descarga paralela (multiples espejos + 4 sesiones concurrentes), agrupacion automatica, tooltip de construccion, IA de percepcion global (ScriptGlobal + GlobalAI), configuracion de espejo/servidor de contenido.

## Estrategia multijugador ("nueva version facil de conectar")

- jrpm es una **version tagged**: `IsNetworkCompatibleVersion` requiere que la cadena de revision coincida exactamente → **el cliente jrpm solo se conecta con servidores jrpm**, completamente aislado de jgrpp 0.73.x / pxp;
- **El servidor es mas permisivo**: El servidor jrpm acepta adicionalmente clientes `jgrpp-*` y `pxp*` (`IsJgrppNativeNetworkRevision` / `IsPxpNetworkRevision`, la version de NewGRF debe coincidir);
- Por lo tanto: el anfitrion del servidor jrpm abre = solo acepta jugadores jrpm (predeterminado); cuando se necesita compatibilidad con clientes antiguos, puede aceptar jugadores jgrpp/pxp sin cambiar la configuracion.

## Construccion y verificacion (ejecutar en la maquina del usuario)

```bash
# Primera vez (necesita CMake + dependencias, consulte COMPILING.md)
cmake -B build ..
cmake --build build -j
# Resultado: build/openttd-jrpm.exe
```

Prioridad de verificacion:
1. `openttd-jrpm -v` muestra `jrpm-0.1.0`;
2. Iniciar una partida en solitario y jugar 1-2 anos (la fusion implica cambios importantes en tren/aeropuerto + la version de archivo puede aumentar debido a allow_modify_airports y otras adiciones);
3. Despues de abrir el servidor: cliente jrpm se une ✓; cliente jgrpp 0.73.x original intenta unirse (se espera que pueda entrar, cuando NewGRF coincida);
4. Desenganche de locomotoras: anadir ordenes de desenganche/enganche al tren, verificar que los dos trenes desenganchados se despachen independientemente; Aeropuertos modulares: activar `station.allow_modify_airports` y modificar la disposicion del aeropuerto;
5. Regresion de las 5 funciones anteriores (descarga paralela, agrupacion automatica, tooltip de construccion, GlobalAI).

## Riesgos conocidos

- **Sin compilacion verificada**: El codigo fusionado+modificado no se ha compilado en esta maquina (sin cadena de herramientas), la primera compilacion real puede tener cambios de interfaz pasados por alto (especialmente las tres grandes modificaciones de aircraft/airport/train);
- Version de archivo: px-patch puede haber aumentado SLV (M9 menciona savegame version gate), los archivos de jrpm y los archivos de jgrpp 0.73.x pueden no ser legibles mutuamente (como es convencion de jgrpp, compatible hacia abajo con archivos trunk);
- El merge incorpora todo el historial de pulsexlb, si necesita rastrear la pertenencia de funciones, use `git log --oneline pulsexlb/px-patch`.

---

## Registro de fusión: 2026-09-28 (jgrpp-0.73.3 + px-patch 2609.x)

> Commits de fusión: jgrpp 94 commits (hasta `jgrpp-0.73.3`) + px-patch 113 commits (hasta después de `pxp-2609.10`).

**Nuevo:** de pulsexlb, transporte de vehículos de carretera (RoRo, ver Funciones), 128 tipos de carga (CargoTypes en Uint128, vía XSLFI_CARGO_TYPES_128), lote de correcciones de desacople/acoplamiento y mejoras de horarios; de jgrpp, arrastrar y soltar órdenes/doble clic, barcos de doble proa (XSLFI_DOUBLE_ENDED_SHIPS) y numerosas correcciones generales.

**Manejo clave de conflictos:** la capa de guardado de pulsexlb usa el antiguo sistema de macros SLE_ mientras jrpm/jgrpp 0.73.3 usa VarFileType/VarMemType + VarTypes; todos los conflictos de saveload/ y sl/ se reescribieron en el sistema moderno añadiendo soporte U128 (VarFileType::U128=13, VarMemType::U128, SLE_UINT128); las versiones 367/368 reservadas por jrpm se mantienen y las nuevas de upstream pasan a 369/370; la compatibilidad con partidas de upstream sigue por el sub-bloque XSLFI_UPSTREAM_VERSION; CT_VEHICLES como cadena, comparación de grfid con GrfID, y los bloques de consola autogroup + RORO_DEBUG_COMMANDS conservados.

**Estado:** compilación MinGW ninja correcta, artefacto `build/openttd-jrpm.exe`; prueba rápida de partida nueva superada.

---

## Registro de fusión: 2026-10-07 (jgrpp 0.73.3+89 + px-patch 2610.3)

> Commits de fusión: 23 commits de jgrpp (tras `jgrpp-0.73.3`, hasta `6318727b02`) + 55 commits de px-patch (hasta `pxp-2610.3`, `4a4d0724b5`).

**Nuevo:** de pulsexlb — ferry de trenes (los barcos se adaptan para transportar trenes completos, carga dedicada RAIL/CT_RAILVEHICLES, carga por vagón), el sistema de lluvia (weather.cpp: períodos de lluvia aleatorios, oscurecimiento progresivo del mundo, capa de gotas que sigue el zoom; opción difficulty.rain; un truco de sandbox impone el clima; el estado del clima se guarda con WTHR/XSLFI_WEATHER), la salida en el mismo sentido tras desacoplar esperando a la otra parte, la visualización de la carga de cada bodega y de cada vagón, la elección de cartel de tren/vehículo en las opciones RoRo, la eliminación del tirón del pathfinding de acoplamiento (validación de carga convertida en comprobación previa con retroceso en caso de fallo) y la compilación de Android con publicación de APK; de jgrpp — StringID como tipo fuerte, rechazo correcto de nombres de empresa duplicados, cálculo del tamaño preferido de los widgets y numerosos refactorizaciones y correcciones.

**Manejo clave de conflictos:** el mayor obstáculo de compilación fue el tipado fuerte de Label/StringID: todas las etiquetas literales de 4 caracteres del código propio de jrpm pasaron a construcción por cadena (CT_RAILVEHICLES{"RAIL"}); las comparaciones de etiquetas de tipo de vía en afterload.cpp pasaron a RoadTypeLabel{"ROAD"}; los campos de cadena del AirTypeInfo vacío en airport.cpp usan STR_NULL; las ramas switch de STR_CHEAT_RAIN en cheat_gui.cpp usan .base(); el antiguo tipo de bloque CH_TABLE del nuevo bloque de clima WTHR pasó a ChunkType::Table; la tabla de trampas conserva el tipo VarMemType y el centinela InflationCheat de jrpm añadiendo la nueva fila de lluvia; SetStringTip(SPR_GOTO_LOCATION, …) en jrpm_watch_gui.cpp pasó a SetSpriteTip; train_cmd.cpp conserva la puerta enable_decouple y la salida de depuración desync mientras adopta la corrección upstream que solo marcha atrás cuando la cabeza del tren pasa el cambio (if (v->IsMovingFront())) y las banderas de salida en el mismo sentido; order_cmd.cpp adopta la nueva semántica de DrivingBackwards basada en TCF_NO_DRIVING_CAB conservando la variable DecouplePart; deploy-docs.yml conserva la compilación VitePress y el despliegue en GitHub Pages, igual que README/.gitignore/.ottdrev-vc conservan la identidad de jrpm; SL_UPSTREAM_VERSION sigue fijado en 368 (DoubleEndedShips de upstream), el static_assert de src/saveload/engine_sl.cpp pasa y las versiones 367/368 reservadas por jrpm no cambian.

**Estado:** compilación MinGW ninja correcta (-j3), artefacto `build/openttd-jrpm.exe`; prueba de humo del servidor dedicado `-D` superada — generación de mapa → guardado → carga → guardado de nuevo → salida sin ninguna aserción ni fallo de SetupEngines, y las puertas de versión de los bloques upstream funcionan correctamente.
