---
title: Guia de desarrollo
---

# Guia de desarrollo

Este documento explica como continuar desarrollando en jrpm. Todos los patrones provienen de la implementacion real de este proyecto (se puede comparar con los commits en `git log`).

## Resumen rapido de la organizacion del codigo

| Directorio | Responsabilidad |
|---|---|
| `src/` | Todo el codigo fuente C++ (nivel superior nombrado por sistema: `rail_cmd.cpp`, `group_gui.cpp`...) |
| `src/network/` | Red (servidor/cliente/UDP/HTTP/descarga de contenido) |
| `src/script/` | Framework de scripts (Squirrel); `api/` son las clases de API de AI/GS |
| `src/table/settings/*.ini` | **Fuentes de definicion de configuracion** (settingsgen genera codigo) |
| `src/lang/english.txt` | Definiciones de cadenas (strgen genera) |
| `src/sl/saveload_common.h` | Enumeracion de versiones de archivo (SLV) |
| `bin/ai/` | Scripts de AI (`GlobalAI` es un ejemplo) |

## Patrones de desarrollo comunes

- [Anadir nuevo comando del juego](./add-command): Enumeracion de comandos + `DEF_CMD_TUPLE_NT` + manejador + entrada GUI/consola
- [Anadir nueva API de script](./add-script-api): `script_*.hpp/.cpp` (registro automatico de enlace Squirrel)
- [Anadir nueva configuracion](./add-setting): `.ini` + campo `settings_type.h` + cadena de texto

## Construccion

```bash
cmake -B build ..
cmake --build build -j
```

## Notas de desarrollo

1. **Codificacion**: Despues de modificar codigo fuente/documentos que contengan chino, preste atencion a UTF-8 (las herramientas Write/Edit de esta maquina pueden producir codificacion GBK, se puede reparar con `jgrpp-features/_fix_utf8.py`);
2. **Mensajes de commit**: Se recomienda usar ingles (evita problemas de codificacion en la terminal);
3. **Insercion en medio de la enumeracion de comandos** desplazara los IDs de comandos posteriores -- las versiones antiguas del binario no coincidiran con las nuevas al conectarse, es normal dentro del fork, la actualizacion debe ser sincronizada;
4. **Version de archivo**: Al modificar la estructura del archivo, anadir una nueva entrada `SLV_*` en `src/sl/saveload_common.h` y actualizar `SAVEGAME_VERSION`;
5. **Archivos nuevos**: Los `.cpp` deben incluirse en la lista de fuentes del `CMakeLists.txt` correspondiente (`src/CMakeLists.txt` o subdirectorio/`script/api/CMakeLists.txt`); `script_*.hpp` son descubiertos automaticamente por `file(GLOB)`, no necesitan registro;
6. **Cadenas nuevas**: Anadir en `src/lang/english.txt` (otros idiomas usan ingles como fallback);
7. **Compatibilidad de NewGRF/archivos**: Modificar `_openttd_content_version` / el numero de version de NewGRF requiere precaucion (afecta la compatibilidad multijugador y de contenido).

## Sugerencias de regresion (importante)

La rama actual **no ha sido compilada en una maquina real**, despues de la primera construccion, se recomienda hacer regresion en este orden:
1. Basico: `openttd-jrpm -v` muestra `jrpm-0.1.0`;
2. Archivo: iniciar partida en solitario y jugar 1-2 anos;
3. Multijugador: clientes jrpm se conectan entre si; clientes jgrpp/pxp se unen;
4. Nuevas caracteristicas: desenganche de locomotoras (desenganche/enganche/reversa), aeropuertos modulares (modificar disposicion), descarga paralela, agrupacion automatica, tooltip de construccion, GlobalAI;
5. Regresion de caracteristicas antiguas: senales, tracerestrict, scheduled dispatch, reemplazo de plantillas.