---
title: Historico de Versoes
---

# Historico de Versoes

## jrpm-0.1.0 (2026-08-14) Portabilidade do cmclient concluida

Com base na versao inicial 0.1.0, a portabilidade dos cinco lotes de recursos do cmclient (modded + cmclient completo) foi concluida:

### Novos Recursos (Portabilidade cmclient / modded)

- **Primeiro lote modded**: Historico de viagens de veiculos (`b89f93f9`) + Velocidade de taxi de aeronaves ajustavel (`b89f93f9`);
- **Segundo lote cmclient**: Marcadores de posicao / Detalhes de carga / Observacao (`089480b3`);
- **Lote zero cmclient**: Comandos de console `cmgamespeed` / `cmgamestats` / `cmexport` / `cmtreemap` (`1fd94d12`);
- **Terceiro lote cmclient 1 Destaque**: Pre-visualizacao de construcao em nivel de objeto (estacoes/trilhos/garagens/aeroportos), conectado ao pipeline de renderizacao viewport e ferramentas de construcao (6 commits, `d97aa38a` -> `1957bf45`);
- **Terceiro lote cmclient 3 Blueprint**: Selecao de area/copia/rotacao/16 slots/reconstrucao (`8e08ca6b`);
- **Quarto lote cmclient 4 Zoneamento urbano**: Zonas Tz + coloracao growth_tiles + bloco de salvamento GRWT (`9e3f95a2`);
- **Quinto lote cmclient 5 Reproducao de comandos**: `cmdrecord` / `cmdreplay` (`f113acce28`);
- **Correcao de convencao de parametros do console**: Todos os comandos jrpm agora usam argv[1] como parametro inicial (argv[0] e o nome do comando).

### Conclusao Arquitetural

- **2 Camada de objeto de comando nao portada**: Destaque/blueprint/reproducao usam `CMD_ERROR`, fechamentos de comando e serializacao nativa de comandos jrpm respectivamente, sem necessidade dos 2251 linhas de codigo gerado do cmclient.

### Compatibilidade de Arquivos Salvos

- Dados growth_tiles armazenados no chunk independente `GRWT`, arquivos antigos (sem esse chunk) carregam com compatibilidade total;
- Arquivos de gravacao de comando (`.jrcm`) sao formato privado do jrpm, sem garantia de compatibilidade entre versoes.

## jrpm-0.1.0 (2026-08-14)

Primeira versao do jrpm, baseada no jgrpp 0.73.1 + pulsexlb px-patch mesclados, com recursos portados do modded / cmclient.

### Alteracoes

- **Mesclagem do pulsexlb px-patch (152 commits)**:
  - Desacoplamento de locomotivas (decouple): Ordens de desengate/engate, transferencia de tiquetes, limites de comprimento/velocidade de acoplamento, cabecas duplas, acoplamento NewGRF, navegacao de acoplamento, despacho independente apos desengate;
  - Aeroportos modulares (multitile-airport): Sistema de tipos air, despacho aereo PBS, navegacao aerea YAPF, modificacao de layout de aeroporto (`allow_modify_airports`);
  - Nova versao de salvamento: `SLV_MULTITILE_AIRPORTS` / `SLV_ORDER_DECOUPLE`.
- **Renomeacao de versao**: `openttd-jrpm` / `jrpm-0.1.0` (nome do executavel e string de revisao).
- **Compatibilidade de multiplas versoes no servidor**: Servidor jrpm aceita clientes jrpm / jgrpp original / pulsexlb.
- **Recursos exclusivos do jrpm**:
  - Download de recursos com multiplos espelhos + paralelo em nivel de arquivo (concorrencia configuravel);
  - Agrupamento automatico de veiculos por ordens compartilhadas (botao de janela + comando `autogroup`);
  - IA de percepcao global (API `AIGlobal` + exemplo GlobalAI, controlado por configuracao);
  - Dica de custo de construcao (removida conforme solicitado, commit `96ebfb75`).
- **Portabilidade modded (primeiro lote)**:
  - Historico de viagens de veiculos (ultimas 10 viagens: lucro/taxa de ocupacao/duracao, botao History na janela de detalhes do veiculo);
  - Velocidade de taxi de aeronaves ajustavel (`vehicle.plane_taxi_speed`, controlado por XSLF, compativel com arquivos antigos).
- **Inspirado no cmclient (segundo lote)**:
  - Marcadores de posicao (`savelocation` / `gotolocation`, 9 slots);
  - Janela de detalhes de carga da empresa (`company_cargo`);
  - Auxilio de observacao (`watch <company_id>`).

### Compatibilidade de Arquivos Salvos

- Herda a convencao do jgrpp: pode carregar arquivos trunk (ate a versao incorporada mais recente);
- Arquivos salvos jrpm (com dados de aeroportos multibloco/ordens de desengate) **nao garantem** leitura mutua com arquivos jgrpp antigos;
- Numero de versao de salvamento `SAVEGAME_VERSION` e consistente com pulsexlb (`SLV_CUSTOM_SUBSIDY_DURATION`);
- Novos recursos usam XSLF ou NOSAVE, sem quebrar arquivos antigos.

## Versoes Upstream

- **jgrpp 0.73.1**: Base do projeto (kernel OpenTTD 16.0 + todos os recursos JGR).
- **pulsexlb px-patch 2608.3**: Fonte do desacoplamento de locomotivas e aeroportos modulares.
- **embeddedt/OpenTTD-modded (era 0.59.1)**: Fonte do historico de viagens e velocidade de taxi.
- **citymania-org/cmclient (vanilla 15.3)**: Fonte de inspiracao para marcadores de posicao/detalhes de carga/observacao.

## Roteiro

- [x] Verificacao de construcao real e correcao de erros de primeira compilacao
- [x] Historico de viagens + Velocidade de taxi (primeiro lote modded)
- [x] Marcadores de posicao + Detalhes de carga + Observacao (segundo lote cmclient)
- [ ] Sistema de destaque + Sistema de blueprint (terceiro lote cmclient, veja [Roteiro](../features/highlight-blueprint-plan))
- [ ] Pool de threads da camada de transporte HTTP + download por partes Range (aceleracao de arquivos grandes)
- [ ] Otimizacao de compressao de envio de mapa
- [ ] Estatisticas de receita por carga (requer extensao de formato de salvamento)
- [ ] (Futuro) Integracao de runtime de servidor Rust (referencia Openttd-Cluster)