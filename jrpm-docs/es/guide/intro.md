---
title: Introduccion del proyecto
---

# Introduccion del proyecto OpenTTD-JRPM

## Que es

**OpenTTD-JRPM (jrpm)** es una rama de desarrollo secundaria basada en [JGR's Patchpack](https://github.com/JGRennison/OpenTTD-patches) (jgrpp), que incorpora las caracteristicas de "desenganche de locomotoras" y "aeropuertos modulares" de [pulsexlb/OpenTTD-patches](https://github.com/pulsexlb/OpenTTD-patches), y anade funcionalidades exclusivas de jrpm. Version actual: **jrpm-0.1.0**.

Es una version completamente nueva que se puede instalar y conectar de forma independiente: el cliente/servidor de jrpm utiliza un identificador de version independiente (`jrpm-0.1.0`) para el handshake, completamente aislado de las versiones originales de jgrpp y pulsexlb, evitando confusiones de version.

## Caracteristicas de un vistazo

### De pulsexlb (152 commits, incorporados via git merge)
- **Desenganche de locomotoras (decouple)**: Sistema completo de desenganche/enganche de trenes -- ordenes de desenganche, transferencia de tokens, limites de longitud y velocidad de acoplamiento, soporte de doble locomotora, acoplamiento NewGRF, busqueda de rutas de acoplamiento (YAPF/NPF), despacho independiente de dos trenes tras desenganche;
- **Aeropuertos modulares (multitile-airport)**: Reestructuracion del sistema de aeropuertos de multiples casillas -- sistema de tipos air (`air.h`/`air_type.h`/`newgrf_airtype.*`), despacho aereo PBS (`pbs_air.*`), busqueda de rutas YAPF para aviacion, `station.allow_modify_airports` para modificar la disposicion del aeropuerto, `gui.default_air_type` como tipo de aviacion predeterminado.

### Exclusivo de jrpm
| Funcion | Descripcion | Acceso |
|---|---|---|
| Descarga de recursos: multiples espejos + paralelo | Multiples fuentes espejo separadas por comas, descarga paralela a nivel de archivo (concurrencia configurable), cambio automatico de espejo en caso de fallo | Configuracion → `network.content_mirrors` / `network.content_download_parallel` |
| Agrupacion automatica de vehiculos | Agrupa automaticamente vehiculos por ordenes/despacho compartidos, nombre de grupo tomado del nombre de la ruta | Boton en ventana de grupos / consola `autogroup` |
| Tooltip de precio de construccion | Muestra el coste estimado en tiempo real sobre el cursor al construir vias ferreas/carreteras/terreno | Barras de herramientas de via ferrea/carretera/terreno |
| IA de percepcion global | Conserva NoAI, anade API global `AIGlobal` + IA de ejemplo GlobalAI, acceso controlado por configuracion del juego | `game.script.allow_global_ai_access` |
| Compatibilidad multiversion del servidor | El servidor jrpm acepta simultaneamente clientes jrpm / jgrpp original / pulsexlb | Activado automaticamente al unirse al servidor |

### Herencia completa
- Todas las caracteristicas de jgrpp (mejoras de senales, scheduled dispatch, tracerestrict, reemplazo de plantillas, etc.);
- Toda la funcionalidad de OpenTTD upstream y compatibilidad con el ecosistema NewGRF/scripts.

## Relacion de versiones

```
                 jgrpp-0.73.1 (ancestro comun)
                 /                 \
 rama jgrpp (63 commits)        pulsexlb px-patch (152 commits)
 ├ actualizaciones recientes jgrpp  ├ desenganche de locomotoras (rama decouple)
 ├ funciones exclusivas jrpm         └ aeropuertos modulares (rama multitile-airport)
 └ cambio de nombre jrpm-0.1.0
                 \                 /
                  rama jrpm (git merge)
```

## Ramas y commits

- Rama: `jrpm` (linea principal de desarrollo)
- Commits clave:
  - `d4c45740` 5 funciones exclusivas de jrpm
  - `71fe214c` merge de pulsexlb (desenganche + aeropuertos modulares)
  - `425e7207` cambio de nombre a openttd-jrpm / jrpm-0.1.0
  - `cb9848b7` compatibilidad multiversion de clientes del servidor
  - `4716b925` concurrencia de descarga paralela configurable

## Licencia

Igual que OpenTTD: **GPL-2.0**.