---
layout: home

hero:
  name: "OpenTTD-JRPM"
  text: "Una version mejorada de OpenTTD con caracteristicas seleccionadas"
  tagline: Enganche y desenganche de locomotoras · Aeropuertos modulares · Descarga paralela · IA de percepcion global · Reproduccion de comandos · Planos · Zonificacion urbana
  image:
    src: /jrpm-hero.png
    alt: OpenTTD-JRPM
  actions:
    - theme: brand
      text: 🚀 Inicio rapido
      link: /es/guide/build
    - theme: alt
      text: 📚 Vision general de funciones
      link: /es/features/overview
    - theme: alt
      text: 🔍 GitHub
      link: https://github.com/Maicarons/jrpm

features:
  - icon: 🚂
    title: Desenganche de locomotoras (decouple)
    details: " Sistema completo de desenganche/enganche de trenes: ordenes de desenganche, transferencia de tokens, limites de longitud y velocidad de acoplamiento, soporte de doble locomotora, acoplamiento NewGRF, despacho independiente de dos trenes tras desenganche."
  - icon: 🛫
    title: Aeropuertos modulares (multitile-airport)
    details: " Reestructuracion del sistema de aeropuertos de multiples casillas: sistema de tipos air, despacho aereo PBS, busqueda de rutas YAPF para aviacion, disposicion de aeropuerto modificable (allow_modify_airports)."
  - icon: ⬇️
    title: Descarga paralela de contenido
    details: Multiples fuentes espejo (separadas por comas, personalizables) + descarga paralela de multiples archivos + cambio automatico de espejo en caso de fallo + numero de concurrencia configurable.
  - icon: 🤖
    title: IA de percepcion global
    details: Conserva NoAI, anade una nueva API global AIGlobal (datos financieros/vehiculos/clasificaciones/mapa de todas las companias), con control de acceso mediante configuracion del juego y AI de ejemplo.
  - icon: 💰
    title: Tooltip de precio de construccion
    details: Al construir vias ferreas/carreteras/terreno, muestra en tiempo real el coste estimado sobre el cursor, evitando clics en proyectos grandes por error.
  - icon: 📦
    title: Agrupacion automatica de vehiculos
    details: Crea y agrupa automaticamente vehiculos por ordenes compartidas/despacho; el nombre del grupo se toma automaticamente del nombre de la ruta; compatible con comandos de consola y botones de ventana.
  - icon: 🧱
    title: Resaltado de construccion a nivel de objeto
    details: Muestra una vista previa en tiempo real de estaciones, vias, depositos, aeropuertos e industrias al colocar/arrastrar herramientas de construccion (ya no es un simple rectangulo).
  - icon: 📐
    title: Sistema de planos
    details: Selecciona un area con blueprint_copy, luego blueprint_build en otro lugar para reconstruir vias, depositos, tuneles, puentes, estaciones y senales (16 ranuras de memoria + rotacion).
  - icon: 🏘️
    title: Zonificacion urbana + archivo growth_tiles
    details: Zonas urbanas Tz0–Tz4 y persistencia de construccion/eliminacion de casas de este mes/mes anterior en un nuevo bloque de archivo GRWT (compatible con archivos antiguos automaticamente).
  - icon: ⏺️
    title: Grabacion y reproduccion de comandos
    details: cmdrecord start/stop graba todos los comandos ejecutados; cmdreplay reproduce, usando la propia serializacion de comandos de jrpm, sin necesidad de una capa adicional de objetos de comando.
---

<div class="vp-doc cta-block">

### 💎 Una breve introduccion a jrpm

OpenTTD-JRPM (jrpm) es un fork selecto basado en **JGR's Patchpack**, que fusiona **pulsexlb px-patch** (desenganche de locomotoras,
aeropuertos modulares de multiples casillas), funcionalidades seleccionadas de **OpenTTD-modded** y **cmclient**, e incorpora capacidades desarrolladas por jrpm como IA de percepcion global,
descarga paralela y agrupacion automatica de vehiculos. Consulte la
[historial de versiones](/es/guide/version-history) y la [introduccion del proyecto](/es/guide/intro) para una comparacion completa de versiones y el diseno conceptual.

</div>