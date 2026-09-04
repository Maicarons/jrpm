---
layout: home

hero:
  name: "OpenTTD-JRPM"
  text: "A curated enhanced edition of OpenTTD"
  tagline: Train Decoupling · Modular Airports · Parallel Downloads · Whole-Game-Aware AI · Command Replay · Blueprints · Town Zoning
  image:
    src: /jrpm-hero.png
    alt: OpenTTD-JRPM
  actions:
    - theme: brand
      text: 🚀 Quick Start
      link: /en/guide/build
    - theme: alt
      text: 📚 Feature Overview
      link: /en/features/overview
    - theme: alt
      text: 🔍 GitHub
      link: https://github.com/Maicarons/jrpm

features:
  - icon: 🚂
    title: Train Decoupling (Decouple)
    details: " Complete train decouple/couple system: decouple orders, path signal transfer, coupling length and speed limits, dual-head support, NewGRF coupling, and independent scheduling after decoupling."
  - icon: 🛫
    title: Modular Airports (Multitile-Airport)
    details: " Multitile airport system overhaul: air type system, PBS air traffic control, YAPF air pathfinding, modifiable airport layouts (allow_modify_airports)."
  - icon: ⬇️
    title: Parallel Content Download
    details: Multiple mirror sources (comma-separated, customizable) + parallel file downloads + automatic mirror fallback on failure + configurable concurrency.
  - icon: 🤖
    title: Whole-Game-Aware AI
    details: Retains NoAI, adds AIGlobal global API (all company finances/vehicles/ratings/map data), with game setting access control and example AI.
  - icon: 💰
    title: Build Cost Tooltip
    details: When building rails/roads/terrain, the mouse cursor shows a real-time estimated construction cost, preventing costly mistakes.
  - icon: 📦
    title: Vehicle Auto-Grouping
    details: One-click auto-create and assign groups by shared orders/schedules, with group names automatically derived from route names; supports console command and window button.
  - icon: 🧱
    title: Object-Level Build Highlighting
    details: Station, track, depot, airport, and industry placement/drag tools show a real-time object preview (not just a simple rectangle).
  - icon: 📐
    title: Blueprint System
    details: Select an area with blueprint_copy, then blueprint_build elsewhere to reconstruct rails, depots, tunnels, bridges, stations, and signals (16 memory slots + rotation).
  - icon: 🏘️
    title: Town Zoning + growth_tiles Save
    details: Tz0–Tz4 town zones and monthly house construction/demolition tiles persisted to a new GRWT save chunk (old saves auto-compatible).
  - icon: ⏺️
    title: Command Recording & Replay
    details: cmdrecord start/stop records all executed commands; cmdreplay replays them using jrpm's own command serialization, no extra command object layer needed.
---

<div class="vp-doc cta-block">

### 💎 A Brief Introduction to jrpm

OpenTTD-JRPM (jrpm) is a curated fork based on **JGR's Patchpack**, blending **pulsexlb px-patch** (train decoupling, multitile modular airports), selected features from **OpenTTD-modded** and **cmclient**, along with jrpm's own whole-game-aware AI, parallel downloads, and vehicle auto-grouping. For a complete version comparison and design philosophy, see [Version History](/en/guide/version-history) and [Project Introduction](/en/guide/intro).

</div>