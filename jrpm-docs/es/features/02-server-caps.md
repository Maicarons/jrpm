---
title: Estudio de limites del servidor
---

## Conclusion de la investigacion: ambos limites son estructurales

### Limite de jugadores en linea = 255 (restriccion estricta de ancho de bits del protocolo)

| Ubicacion | Contenido |
|---|---|
| `src/network/network_type.h:21` | `static const uint MAX_CLIENTS = 255;` |
| `src/network/network_type.h:54` | `ClientPoolIDTag : PoolIDTraits<uint16_t, MAX_CLIENTS + 1, 0xFFFF>` |
| `src/network/core/network_game_info.cpp` | `clients_max` / `companies_max` se transmiten como **`Send_uint8` / `Recv_uint8`** en `SerializeNetworkGameInfo` (lineas 251-296) y deserializacion (lineas 422-432) |
| `src/table/settings/network_settings.ini:241` | `network.max_clients` (SLE_UINT8, max = MAX_CLIENTS, predeterminado 25) |
| `src/network/network_server.cpp:360` | Verificacion de aceptacion `_network_clients_connected < MAX_CLIENTS` |

**Conclusion**: 255 ya es el limite superior del campo de protocolo uint8. Para superarlo, habria que cambiar `clients_max` y campos relacionados a uint16 -- esto alteraria el protocolo cliente↔servidor, la transmision UDP y el protocolo del **Game Coordinator**, lo que constituye un cambio de protocolo externo (se puede jugar dentro del fork si ambos extremos se actualizan, pero la compatibilidad con la lista de servidores publicos se veria afectada).

### Limite de numero de empresas = 15 (restriccion estricta del formato de almacenamiento de propietario de casilla)

| Ubicacion | Contenido |
|---|---|
| `src/company_type.h:25` | `CompanyIDTag : PoolIDTraits<uint8_t, 0xF, 0xFF>` → `MAX_COMPANIES = CompanyID::End().base() = 15` |
| `src/tile_map.h:195` | `SetTileOwner`: `SB(_m[tile].m1, 0, 5, owner.base())` -- el propietario de la casilla solo ocupa **5 bits** (bits bajos de `_m[].m1`) |
| `src/company_type.h:30-33` | `OWNER_TOWN{0x0F}`, `OWNER_NONE{0x10}`, `OWNER_WATER{0x11}`, `OWNER_DEITY{0x12}`, `OWNER_END{0x13}` comparten el mismo espacio de bytes con los IDs de empresa |

**Conclusion**: `Owner` y `CompanyID` son del mismo tipo, el campo de propiedad de casilla solo tiene 5 bits (valores 0-31), de los cuales 15-18 estan ocupados por propietarios especiales. Por lo tanto, el limite real de empresas es 15, determinado por el **formato del array del mapa** -- para superarlo, habria que ampliar el almacenamiento de propietario de casilla (usar los 8 bits completos de `m1` o cambiar la estructura `_m`) y migrar las constantes `OWNER_*`, lo que implica **conversion completa de archivos guardados** y aumento de memoria del mapa, una reestructuracion de formato importante (OpenTTD upstream no lo ha hecho en anos por esta razon).

## Opciones disponibles

| Opcion | Cambios | Compatibilidad | Sugerencia |
|---|---|---|---|
| A. Mantener el estado actual | Ninguno | Archivo/protocolo/coordinador totalmente compatible | ✅ Recomendado: 255 clientes / 15 empresas ya superan con creces el original para multijugador |
| B. Cliente → protocolo uint16 | `network_game_info.cpp` serializacion/deserializacion cambiar a `Send_uint16/Recv_uint16`; `MAX_CLIENTS` aumentar a 4095; `network_settings.ini` `max_clients` cambiar a SLE_UINT16; `settings_type.h` campo cambiar a uint16; `console_cmds.cpp:1056` ancho de visualizacion | Se puede jugar dentro del fork; coordinador publico/clientes antiguos incompatibles | Opcional, si realmente se necesitan >255 clientes |
| C. Empresa → reestructuracion de formato de casilla | Ampliar campo de propietario de casilla + migrar constantes OWNER_* + conversion completa de archivos | Cambio de formato de archivo (conversion unica) | No recomendado para implementacion a corto plazo |

## Contenido entregado

- Este documento de diseno (con archivos/lineas exactos);
- Parche opcional `option-clients-uint16.diff` (cambios completos de la opcion B, **no aplicado**, disponible para revision con `git apply`).

> Conclusion anticipada: **"Ampliar" ambos limites es esencialmente una actualizacion de formato/protocolo, no un ajuste de configuracion**; manteniendo la compatibilidad de archivos y red publica, el limite actual de 255/15 es el maximo. Si se acepta el costo de compatibilidad, las opciones B/C son como se indican arriba.