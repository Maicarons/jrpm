---
layout: home

hero:
  name: "OpenTTD-JRPM"
  text: "Version amelioree d'OpenTTD, fusion de qualite"
  tagline: Decouplage de locomotives - Aeroports modulaires - Telechargement parallele - IA consciente de la partie - Rejeu de commandes - Plans - Zonage urbain
  image:
    src: /jrpm-hero.png
    alt: OpenTTD-JRPM
  actions:
    - theme: brand
      text: "Demarrage rapide"
      link: /fr/guide/build
    - theme: alt
      text: "Apercu des fonctionnalites"
      link: /fr/features/overview
    - theme: alt
      text: "GitHub"
      link: https://github.com/Maicarons/jrpm

features:
  - icon: "🚂"
    title: Decouplage de locomotives
    details: " Systeme complet d'attelage/decouplage de trains : ordres de decouplage, transfert de consignes, limite de longueur/vitesse d'attelage, support de double locomotive, attelage NewGRF, deux trains independants apres decouplage."
  - icon: "🛫"
    title: Aeroports modulaires
    details: " Refonte du systeme d'aeroport multi-tuiles :体系e de type air, gestion aerienne PBS, pathfinding aerien YAPF, disposition d'aeroport modifiable (allow_modify_airports)."
  - icon: "⬇️"
    title: Telechargement parallele de contenu
    details: Multiples miroirs (separes par des virgules, personnalisables) + telechargement parallele de fichiers + bascule automatique en cas d'echec + nombre de connexions configurable.
  - icon: "🤖"
    title: IA consciente de la partie
    details: Conserve NoAI, ajoute l'API globale AIGlobal (donnees financieres/vehicules/evaluations/carte de toutes les entreprises), avec controle d'acces via les parametres de jeu et IA d'exemple.
  - icon: "💰"
    title: Infobulle de cout de construction
    details: Lors de la construction de rails/routes/terrains, affiche en temps reel le cout estime au-dessus de la souris, evitant les clics sur de grands projets.
  - icon: "📦"
    title: Groupement automatique de vehicules
    details: Cree et regroupe automatiquement les vehicules par ordres partages/affectations en un clic, le nom du groupe reprend automatiquement le nom de la ligne ; supporte la console et les boutons de fenetre.
  - icon: "🧱"
    title: Surbrillance d'objet en construction
    details: Affiche un apercu en temps reel des objets (gares, voies, depots, aeroports, industries) lors de l'utilisation des outils de construction/etirement (plus un simple rectangle).
  - icon: "📐"
    title: Systeme de plans
    details: Apres avoir selectionne une zone avec blueprint_copy, utilisez blueprint_build ailleurs pour reconstruire voies, depots, tunnels, ponts, gares et signaux (16 emplacements memoire + rotation).
  - icon: "🏘️"
    title: Zonage urbain + archive growth_tiles
    details: Zonage urbain Tz0-Tz4 et donnees de construction/demolition de maisons du mois courant/precedent persistees dans un nouveau bloc d'archive GRWT (compatibilite automatique avec les anciennes archives).
  - icon: "⏺️"
    title: Enregistrement et rejeu de commandes
    details: cmdrecord start/stop enregistre toutes les commandes executees ; cmdreplay rejoue, utilisant la propre serialisation de commandes de jrpm, sans couche d'objet de commande supplementaire.
---

<div class="vp-doc cta-block">

### Presentation succincte de jrpm

OpenTTD-JRPM (jrpm) est un fork soigneux base sur **JGR's Patchpack**, fusionnant des fonctionnalites selectionnees de **pulseXLB px-patch** (decouplage de locomotives, aeroports modulaires multi-tuiles), **OpenTTD-modded** et **cmclient**, tout en ajoutant des capacites propres a jrpm comme l'IA consciente de la partie, le telechargement parallele et le groupement automatique de vehicules. Pour une comparaison complete des versions et la philosophie de conception, veuillez consulter l'[historique des versions](/fr/guide/version-history) et la [presentation du projet](/fr/guide/intro).

</div>