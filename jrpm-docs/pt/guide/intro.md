---
title: Introducao ao Projeto
---

# Introducao ao OpenTTD-JRPM

## O que e

**OpenTTD-JRPM (jrpm)** e um branch de desenvolvimento secundario baseado no [JGR's Patchpack](https://github.com/JGRennison/OpenTTD-patches) (jgrpp), que integra os recursos de "desacoplamento de locomotivas" e "aeroportos modulares" do [pulsexlb/OpenTTD-patches](https://github.com/pulsexlb/OpenTTD-patches), alem de adicionar funcionalidades exclusivas do jrpm. Versao atual: **jrpm-0.1.0**.

E uma versao totalmente nova, instalavel e jogavel online de forma independente: o cliente/servidor jrpm usa identificadores de versao independentes (`jrpm-0.1.0`) para handshake, completamente isolado do jgrpp original e das versoes pulsexlb, evitando confusao de versoes.

## Visao Geral dos Recursos

### Do pulsexlb (152 commits, incorporados via git merge)
- **Desacoplamento de locomotivas (decouple)**: Sistema completo de desengate/engate de trens -- ordens de desengate, transferencia de tiquetes, limites de comprimento e velocidade de acoplamento, suporte a cabecas duplas, acoplamento NewGRF, navegacao de acoplamento (YAPF/NPF), despacho independente apos desengate;
- **Aeroportos modulares (multitile-airport)**: Reformulacao do sistema de aeroportos multibloco -- sistema de tipos air (`air.h`/`air_type.h`/`newgrf_airtype.*`), despacho aereo PBS (`pbs_air.*`), navegacao aerea YAPF, modificacao de layout de aeroporto `station.allow_modify_airports`, tipo de aeronave padrao `gui.default_air_type`.

### Exclusivos do jrpm
| Recurso | Descricao | Entrada |
|---|---|---|
| Download de recursos: Multiplos espelhos + paralelo | Multiplos espelhos separados por virgula, download paralelo em nivel de arquivo (concorrencia configuravel), alternancia automatica de espelho em caso de falha | Configuracoes → `network.content_mirrors` / `network.content_download_parallel` |
| Agrupamento automatico de veiculos | Agrupa automaticamente veiculos por ordens/escalas compartilhadas com um clique; nome do grupo definido automaticamente como nome da rota | Botao na janela de grupos / console `autogroup` |
| Dica de custo de construcao | Exibe custo estimado em tempo real ao passar o mouse sobre trilhos/estradas/terreno | Barra de ferramentas de trilhos/estradas/terreno |
| IA de percepcao global | Mantem NoAI, adiciona API global `AIGlobal` + IA de exemplo GlobalAI, controlada por configuracao do jogo | `game.script.allow_global_ai_access` |
| Compatibilidade de multiplas versoes do servidor | Servidor jrpm aceita clientes jrpm / jgrpp original / pulsexlb simultaneamente | Ativado automaticamente na verificacao de entrada do servidor |

### Heranca Completa
- Todos os recursos do jgrpp (sinais aprimorados, scheduled dispatch, tracerestrict, substituicao de modelos, etc.);
- Compatibilidade total com recursos upstream do OpenTTD e ecossistema NewGRF/scripts.

## Relacao de Versoes

```
                 jgrpp-0.73.1 (ancestral comum)
                 /                 \
 jgrpp branch (63 commits)        pulsexlb px-patch (152 commits)
 ├ atualizacoes recentes jgrpp             ├ desacoplamento (branch decouple)
 ├ recursos exclusivos jrpm              └ aeroportos modulares (branch multitile-airport)
 └ renomeacao de versao jrpm-0.1.0
                 \                 /
                  branch jrpm (git merge)
```

## Branch e Commits

- Branch: `jrpm` (linha de desenvolvimento principal)
- Commits importantes:
  - `d4c45740` 5 recursos exclusivos do jrpm
  - `71fe214c` merge pulsexlb (desacoplamento + aeroportos modulares)
  - `425e7207` Renomeacao de versao para openttd-jrpm / jrpm-0.1.0
  - `cb9848b7` Compatibilidade de multiplas versoes de cliente no servidor
  - `4716b925` Concorrencia de download paralelo configuravel

## Licenca

Mesma do OpenTTD: **GPL-2.0**.