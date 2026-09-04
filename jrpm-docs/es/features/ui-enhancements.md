---
title: Mejoras de UI multijugador (marcadores de posicion / detalles de carga / espectador)
---

# Mejoras de UI multijugador

Segunda tanda portada de **citymania-org/cmclient**, implementada como **comandos de consola** (sin depender de la infraestructura de teclas/barra de herramientas de cmclient, estable y scripteable).

## Marcadores de posicion (Viewport Locations)

Guarda/restaura la posicion y el zoom de la ventana principal (9 ranuras), util para desplazarse rapidamente entre sus propias fabricas, estaciones y areas de oponentes en multijugador.

```
savelocation <1-9>    # Guarda la posicion y zoom actual del viewport
gotolocation <1-9>    # Salta a la posicion guardada
```

## Detalles de carga de la compania (Company Cargo Details)

Ventana de estadisticas que lista el **volumen entregado + ingresos** de la empresa por carga, conmutable entre **Total / Ultimo mes**.

```
company_cargo <company_id>    # Abre la ventana de detalles de carga
```

- La ventana lista el volumen e ingresos de cada carga estandar + total en la parte inferior (identico a cmclient)
- Haga clic en el encabezado "Cargo" para cambiar entre total/ultimo mes
- Los ingresos por carga se rastrean mediante `CompanyEconomyEntry::cargo_income` (extension de archivo `XSLFI_COMPANY_CARGO_INCOME`), sin perdida en el archivo

## Ayuda para espectador (Watch)

Los espectadores pueden localizar rapidamente el area de construccion de una empresa (saltar a las ultimas coordenadas de construccion de esa empresa).

```
watch <company_id>    # Salta el viewport a la ubicacion de la empresa
```

## Elementos omitidos y justificacion

| Funcion de cmclient | Manejo en jrpm |
|---|---|
| Capa flotante de lista de jugadores (cm_client_list_gui) | jgrpp **ya tiene** ventana de Jugadores en Linea (`NetworkClientList`), no se duplica |
| Informacion detallada del suelo (cm_tooltips) | La ventana LandInfoWindow de jrpm **ya cubre** la visualizacion de detalles de casas/industrias/estaciones |

## Archivos involucrados

- `src/jrpm_locations.cpp/.h` (nuevos: marcadores de posicion + company_cargo + comandos de consola watch)
- `src/jrpm_cargo_table.cpp/.h` (nuevos: ventana de detalles de carga)
- `src/window_type.h` (nuevo `WindowClass::CompanyCargos`)
- `src/console_cmds.cpp` (registro de comandos)
- `src/lang/english.txt` (cadenas STR_JRPM_CARGOS_*)