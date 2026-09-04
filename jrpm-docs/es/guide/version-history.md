---
title: Historial de versiones
---

# Historial de versiones

## jrpm-0.1.0 (2026-08-14) Portabilidad de cmclient completada

Sobre la base del primer 0.1.0, se completa la portabilidad de las cinco tandas de funciones de cmclient (modded + cmclient completo):

### Nuevas funciones (portadas de cmclient / modded)

- **Primera tanda de modded**: Historial de viajes de vehiculos (`b89f93f9`) + Velocidad de taxi de aviones ajustable (`b89f93f9`);
- **Segunda tanda de cmclient**: Marcadores de posicion / Detalles de carga / Espectador (`089480b3`);
- **Tanda cero de cmclient**: Comandos de consola `cmgamespeed` / `cmgamestats` / `cmexport` / `cmtreemap` (`1fd94d12`);
- **Tercera tanda de cmclient ① Resaltado**: Vista previa de construccion a nivel de objeto (estaciones/vias/depositos/aeropuertos), integrado en el pipeline de renderizado de viewport y herramientas de construccion (6 commits, `d97aa38a` → `1957bf45`);
- **Tercera tanda de cmclient ③ Planos**: Seleccion de area copiar/rotar/16 ranuras/reconstruir (`8e08ca6b`);
- **Cuarta tanda de cmclient ④ Zonificacion urbana**: Zonas Tz + coloracion growth_tiles + bloque de archivo GRWT (`9e3f95a2`);
- **Quinta tanda de cmclient ⑤ Reproduccion de comandos**: `cmdrecord` / `cmdreplay` (`f113acce28`);
- **Correccion de convencion de parametros de consola**: Todos los comandos jrpm ahora usan argv[1] como parametro inicial (argv[0] es el nombre del comando).

### Conclusiones de arquitectura

- **② Capa de objetos de comando no portada**: Resaltado/planos/reproduccion usan respectivamente `CMD_ERROR`, cierres de comando y serializacion nativa de comandos de jrpm, evitando los 2251 lineas de codigo generado de cmclient.

### Compatibilidad de archivos guardados

- Los datos de growth_tiles se almacenan en un chunk `GRWT` independiente; la carga de archivos antiguos (sin ese chunk) es totalmente compatible;
- Los archivos de grabacion de comandos (`.jrcm`) tienen formato privado de jrpm, no se garantiza compatibilidad entre versiones.

## jrpm-0.1.0 (2026-08-14)

Primera version de jrpm, basada en la fusion de jgrpp 0.73.1 + pulsexlb px-patch, con portabilidad de caracteristicas de modded / cmclient.

### Cambios

- **Fusion de pulsexlb px-patch (152 commits)**:
  - Desenganche de locomotoras (decouple): ordenes de desenganche/enganche, transferencia de tokens, limites de longitud/velocidad de acoplamiento, doble locomotora, acoplamiento NewGRF, busqueda de rutas de acoplamiento, despacho independiente tras desenganche;
  - Aeropuertos modulares (multitile-airport): sistema de tipos air, despacho aereo PBS, busqueda de rutas YAPF para aviacion, modificacion de disposicion de aeropuerto (`allow_modify_airports`);
  - Nuevas versiones de archivo `SLV_MULTITILE_AIRPORTS` / `SLV_ORDER_DECOUPLE`.
- **Cambio de nombre**: `openttd-jrpm` / `jrpm-0.1.0` (nombre del ejecutable y cadena de revision).
- **Compatibilidad multiversion del servidor**: El servidor jrpm acepta clientes jrpm / jgrpp original / pulsexlb.
- **Funciones exclusivas de jrpm**:
  - Descarga de recursos con multiples espejos + paralelismo a nivel de archivo (concurrencia configurable);
  - Agrupacion automatica de vehiculos por ordenes compartidas (boton de ventana + comando `autogroup`);
  - IA de percepcion global (API `AIGlobal` + GlobalAI de ejemplo, controlado por configuracion);
  - Tooltip de precio de construccion (eliminado segun requerimiento, commit `96ebfb75`).
- **Portabilidad de modded (primera tanda)**:
  - Historial de viajes de vehiculos (ultimos 10 viajes: ganancia/ocupacion/duracion, boton History en ventana de detalles del vehiculo);
  - Velocidad de taxi de aviones ajustable (`vehicle.plane_taxi_speed`, controlado por XSLF, compatible con archivos antiguos).
- **Referencia de cmclient (segunda tanda)**:
  - Marcadores de posicion (`savelocation` / `gotolocation`, 9 ranuras);
  - Ventana de detalles de carga de la compania (`company_cargo`);
  - Ayuda para espectador (`watch <company_id>`).

### Compatibilidad de archivos guardados

- Hereda la convencion de jgrpp: puede cargar archivos trunk (hasta la version incorporada mas reciente);
- Los archivos de jrpm (que contienen datos de aeropuertos multiples/ordenes de desenganche) **no garantizan** lectura mutua con archivos antiguos de jgrpp;
- El numero de version de archivo `SAVEGAME_VERSION` es consistente con pulsexlb (`SLV_CUSTOM_SUBSIDY_DURATION`);
- Las nuevas funciones usan XSLF o NOSAVE, no rompen archivos antiguos.

## Versiones upstream

- **jgrpp 0.73.1**: Base de este proyecto (kernel OpenTTD 16.0 + todas las caracteristicas de JGR).
- **pulsexlb px-patch 2608.3**: Fuente del desenganche de locomotoras y aeropuertos modulares.
- **embeddedt/OpenTTD-modded (era 0.59.1)**: Fuente del historial de viajes y velocidad de taxi.
- **citymania-org/cmclient (vanilla 15.3)**: Fuente de referencia para marcadores de posicion/detalles de carga/espectador.

## Hoja de ruta

- [x] Verificacion de construccion real y correccion de errores de compilacion inicial
- [x] Historial de viajes + Velocidad de taxi (primera tanda de modded)
- [x] Marcadores de posicion + Detalles de carga + Espectador (segunda tanda de cmclient)
- [ ] Sistema de resaltado + Sistema de planos (tercera tanda de cmclient, ver [hoja de ruta](../features/highlight-blueprint-plan))
- [ ] Pool de hilos de capa de transporte HTTP + Descarga por fragmentos Range (aceleracion de archivos grandes individuales)
- [ ] Optimizacion de compresion de envio de mapas
- [ ] Estadisticas de ingresos por carga (requiere extension de formato de archivo)
- [ ] (Futuro) Integracion de Rust runtime de servidor (referencia de Openttd-Cluster)