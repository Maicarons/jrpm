---
title: "Planejamento do Terceiro Lote: Sistema de Destaque + Sistema de Blueprint"
---

# Planejamento do Terceiro Lote: Sistema de Destaque + Sistema de Blueprint

Dois recursos de alto valor do cmclient foram estudados em profundidade. Esta pagina registra o plano de portabilidade e a estimativa de esforco.

## I. Sistema de Destaque em Nivel de Objeto (cm_highlight, 2888 linhas)

**Funcionalidade**: Quando a ferramenta de construcao esta ativa, destaca em tempo real o objeto a ser construido -- pre-visualizacao precisa de **15 tipos de objetos**, incluindo trilhos completos, areas de estacao, estradas/estacionamentos, sinais, pontes, tuneis, docas, aeroportos, industrias, etc.

**Dependencias de portabilidade (API vanilla -> API jrpm)**:

| Dependencia cmclient | Correspondente jrpm |
|---|---|
| `DrawSelectionSprite` / `SetSelectionTilesDirty` / `DrawTileSelectionRect` / `DrawAutorailSelection` (viewport.cpp interno) | Existe, mas assinatura diferente (jgrpp foi refatorado), necessario alinhar um por um |
| `TileZoning` (coloracao de zoneamento urbano) | jrpm nao tem -> necessario criar |
| `_fn_mod` e outros estados globais | Especifico do cmclient -> necessario redesenhar |
| Construtores de objeto `ObjectTileHighlight::make_rail/road_stop/...` | Dependem de API NewGRF de estacoes/estradas, jrpm difere do vanilla |

**Esforco**: Aproximadamente 2-3 rodadas (cada rodada 4-6 horas), o principal desafio e o alinhamento do pipeline de renderizacao viewport.

## II. Sistema de Blueprint (cm_blueprint, 660 linhas)

**Funcionalidade**: Selecionar area -> gravar sequencia de comandos de construcao (trilhos/estacoes/tuneis/pontes/sinais) -> armazenamento em 16 slots -> reconstrucao com um clique + rotacao.

**Dependencias de portabilidade**:

| Dependencia cmclient | Correspondente jrpm |
|---|---|
| **Camada de objeto de comando** `cm_commands.hpp` (as_company / with_callback / set_auto / no_estimate + 100+ classes de comando geradas, 2251 linhas) | jrpm nao tem essa abstracao -> **necessario portar/reescrever primeiro** |
| Estado `_station_gui` de `cm_station_gui.hpp` | Estrutura de estado da GUI de estacao do jrpm e diferente |
| Ponteiro inteligente `sp<Blueprint>` | jrpm pode usar `std::shared_ptr` |
| Iteracao de tiles de `BlueprintCopyArea` | Depende de API generica como `TileIndexDiffC` (jrpm tem) |

**Esforco**: Aproximadamente 2-3 rodadas (incluindo a camada de objeto de comando).

## III. Ordem de Implementacao Recomendada

```
Passo 1: Camada de objeto de comando (design de cm_command_type, reimplementado com Command<T>::Do/Post do jrpm)
         -- Fundacao comum para blueprint e reproducao de comandos (load_commands)
Passo 2: Destaque em nivel de objeto (alinhamento do pipeline viewport, portar objeto por objeto)
Passo 3: Blueprint (copiar/rotacionar/slots/reconstruir)
Passo 4 (opcional): Reproducao de comandos (reproducao completa com lzma) + Zoneamento urbano (growth_tiles, requer extensao de arquivo)
```

## Observacao Preliminar

Esses tres recursos totalizam aproximadamente **5000+ linhas**, abrangendo tres camadas principais (sistema de comandos/arquivo/renderizacao viewport) e dependem de camadas intermediarias que o jrpm nao possui (objeto de comando, barramento de eventos, serializacao de fluxo de bits). E mais seguro trata-los como projetos independentes separados -- recomenda-se implementar em lotes, compilando e verificando online cada lote antes de submeter, evitando grandes alteracoes unicas que dificultam a identificacao de regressoes.