---
layout: home

hero:
  name: "OpenTTD-JRPM"
  text: "Uma Versao Aprimorada do OpenTTD"
  tagline: Desacoplamento de Locomotivas · Aeroportos Modulares · Download Paralelo · IA de Percepcao Global · Reproducao de Comandos · Blueprints · Zoneamento Urbano
  image:
    src: /jrpm-hero.png
    alt: OpenTTD-JRPM
  actions:
    - theme: brand
      text: " Inicio Rapido"
      link: /pt/guide/build
    - theme: alt
      text: " Visao Geral dos Recursos"
      link: /pt/features/overview
    - theme: alt
      text: " GitHub"
      link: https://github.com/Maicarons/jrpm

features:
  - icon: " "
    title: Desacoplamento de Locomotivas (decouple)
    details: " Sistema completo de desengate/engate de trens: ordens de desengate, transferencia de tiquetes, limites de comprimento e velocidade de acoplamento, suporte a cabecas duplas, acoplamento NewGRF, despacho independente apos desengate."
  - icon: " "
    title: Aeroportos Modulares (multitile-airport)
    details: " Reformulacao do sistema de aeroportos multibloco: sistema de tipos air, despacho aereo PBS, navegacao aerea YAPF, layout de aeroporto modificavel (allow_modify_airports)."
  - icon: " "
    title: Download Paralelo de Conteudo
    details: Multiplos espelhos (separados por virgula, personalizaveis) + download paralelo de varios arquivos + alternancia automatica de espelho em caso de falha + concorrencia configuravel.
  - icon: " "
    title: IA de Percepcao Global
    details: Mantem NoAI, adiciona API global AIGlobal (dados financeiros/veiculos/classificacao/mapa de todas as empresas) com controle de acesso por configuracao do jogo e IA de exemplo.
  - icon: " "
    title: Dica de Custo de Construcao
    details: Ao construir trilhos/estradas/terreno, exibe o custo estimado em tempo real ao passar o mouse, evitando cliques acidentais em grandes projetos.
  - icon: " "
    title: Agrupamento Automatico de Veiculos
    details: Agrupa automaticamente veiculos por ordens compartilhadas/escalas com um clique; o nome do grupo e automaticamente definido como o nome da rota; suporta comando de console e botao de janela.
  - icon: " "
    title: Destaque de Construcao em Nivel de Objeto
    details: Exibe pre-visualizacao em tempo real de objetos ao posicionar/arrastar ferramentas de construcao para estacoes, trilhos, garagens, aeroportos e industrias (nao apenas retangulos simples).
  - icon: " "
    title: Sistema de Blueprint
    details: Selecione uma area com blueprint_copy, depois use blueprint_build em outro lugar para reconstruir trilhos, garagens, tuneis, pontes, estacoes e sinais (16 slots de memoria + rotacao).
  - icon: " "
    title: Zoneamento Urbano + Arquivo growth_tiles
    details: Zonas urbanas Tz0-Tz4 e dados de construcao/demolicao de edificios do mes atual/anterior persistidos em novo bloco GRWT (compativel automaticamente com arquivos antigos).
  - icon: " "
    title: Gravacao e Reproducao de Comandos
    details: cmdrecord start/stop grava todos os comandos executados; cmdreplay reproduz usando a serializacao de comandos nativa do jrpm, sem necessidade de camada de objeto de comando adicional.
---

<div class="vp-doc cta-block">

### Uma Breve Introducao ao jrpm

OpenTTD-JRPM (jrpm) e um fork refinado baseado no **JGR's Patchpack**, que integra recursos selecionados do **pulsexlb px-patch** (desacoplamento de locomotivas,
aeroportos modulares multibloco), do **OpenTTD-modded** e do **cmclient**, alem de adicionar capacidades proprias do jrpm, como IA de percepcao
global, download paralelo e agrupamento automatico de veiculos. Consulte o
[historico de versoes](/pt/guide/version-history) e a [introducao do projeto](/pt/guide/intro) para uma comparacao completa e filosofia de design.

</div>