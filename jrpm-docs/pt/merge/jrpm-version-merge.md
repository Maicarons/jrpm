---
title: Visao Geral da Mesclagem de Versoes do jrpm
---

> Branch: `jrpm` | Versao: jrpm-0.1.0 (tagged, 2026-08-14)
> Artefato de construcao: `openttd-jrpm` (nome do executavel)

## Relacao de Versoes

```
                        jgrpp-0.73.1 (ancestral comum)
                        /                 \
        branch jgrpp (63 commits)          pulsexlb px-patch (152 commits)
        ├ tracerestrict e outras atualizacoes recentes      ├ jgrpp-decouple (desacoplamento de locomotivas)
        ├ meus 5 recursos (d4c45740)        └ jgrpp-multitile-airport (aeroportos modulares)
        └ renomeacao de versao jrpm-0.1.0 (425e7207)
                        \                 /
                        branch jrpm (merge 71fe214c + compatibilidade cb9848b)
```

## Conteudo da Mesclagem

### 1. pulsexlb/OpenTTD-patches (px-patch completo 152 commits) -> Ja mesclado

| Recurso | Descricao | Principais Arquivos |
|---|---|---|
| **Desacoplamento de locomotivas (decouple)** | Desengate/engate de trens: ordens de desengate, transferencia de tiquetes, limites de comprimento/velocidade de acoplamento, cabecas duplas, acoplamento NewGRF, navegacao de acoplamento (YAPF/NPF), despacho independente apos desengate | train_cmd.cpp, order_cmd.cpp, order_gui.cpp, train.h, yapf/npf |
| **Aeroportos modulares (multitile-airport)** | Reformulacao do sistema de aeroportos multibloco: sistema de tipos air (air.h/air_type.h/newgrf_airtype.*), despacho aereo PBS (pbs_air.*), navegacao aerea YAPF, `station.allow_modify_airports` (modificacao de layout de aeroporto), `gui.default_air_type`, sprites de aeroporto multibloco | air.*, pbs_air.*, aircraft_cmd.cpp (3600 linhas refatoradas), airport_cmd/gui, station_cmd |

Tratamento de conflitos: Apenas 2 conflitos em arquivos de cabecalho (aircraft.h / airport.h) -- a reformulacao de aviacao do pulsexlb removeu tipos mortos **sem referencias** da area de trabalho (`VehicleAirFlags` bitset, `AirportMovingDataFlag`), aceitou a remocao do lado pulsexlb, confirmado que nenhum outro arquivo os referencia.

### 2. Openttd-Cluster (projeto Rust de cluster do usuario) -> Inspiracao seletiva

| Patch | Tratamento | Descricao |
|---|---|---|
| 0006 vanilla-native-server (compatibilidade de multiplas versoes de cliente) |  **Ja mesclado** (cb9848b7) | Servidor jrpm aceita simultaneamente clientes jrpm / jgrpp original (`jgrpp-`) / pulsexlb (`pxp`); versao NewGRF ainda e estritamente verificada |
| 0001 revision-handshake / 0005 version-metadata |  Ideia ja adotada | jrpm usa string de revisao tagged independente `jrpm-0.1.0`, handshake online isolado do jgrpp/pxp, implementando "nova versao facil para jogar online" |
| 0007 parallel-download (pool de threads HTTP + download por partes Range, 30KB) |  Referencia, nao mesclado | Mesmo tema do F1 deste projeto "paralelismo em nivel de arquivo + multiespelho" e altera os mesmos arquivos; o **pool de threads da camada de transporte/download por partes** do 0007 registrado como direcao de aprimoramento futuro do F1 |
| 0002-0004 ponte snapshot/command/FFI |  Referencia arquitetural | Depende de todo o runtime Rust otc-engine (vinculacao estatica FFI), e uma "integracao geral futura" em vez de mesclagem em nivel de patch; jrpm atualmente mantem binario unico puro C++ |

### 3. Exclusivo do jrpm (5 recursos anteriores, d4c45740) -> Ja no branch jrpm

Download paralelo (multiespelho + 4 sessoes concorrentes), agrupamento automatico, dica de construcao, IA de percepcao global (ScriptGlobal + GlobalAI), configuracoes de espelho/servidor de conteudo.

## Estrategia Online ("Nova versao facil para jogar online")

- jrpm e uma **versao com tag**: `IsNetworkCompatibleVersion` exige string de revisao exatamente correspondente -> **cliente jrpm so se conecta com servidor jrpm**, completamente isolado do jgrpp 0.73.x / pxp;
- **Servidor flexivel**: Servidor jrpm aceita adicionalmente clientes `jgrpp-*` e `pxp*` (`IsJgrppNativeNetworkRevision` / `IsPxpNetworkRevision`, versao NewGRF deve ser igual);
- Portanto: Servidor jrpm = aceita apenas jogadores jrpm (padrao); quando precisar de compatibilidade com clientes antigos, aceita jogadores jgrpp/pxp sem alterar configuracao.

## Construcao e Verificacao (executado pelo usuario localmente)

```bash
# Primeira vez (requer CMake + dependencias, veja COMPILING.md)
cmake -B build ..
cmake --build build -j
# Artefato: build/openttd-jrpm.exe
```

Prioridade de verificacao:
1. `openttd-jrpm -v` exibe `jrpm-0.1.0`;
2. Abrir jogo local e jogar 1-2 anos (mesclagem envolve grandes alteracoes em train/airport + versao de arquivo pode ser alterada devido a allow_modify_airports etc.);
3. Apos abrir servidor: cliente jrpm entra ; cliente jgrpp 0.73.x original tenta entrar (esperado que consiga, quando NewGRF for igual);
4. Desacoplamento de locomotivas: adicionar ordens de desengate/engate ao trem, verificar despacho independente apos desengate; Aeroportos modulares: ativar `station.allow_modify_airports` e modificar layout do aeroporto;
5. Regression dos 5 recursos anteriores (download paralelo, agrupamento automatico, dica de construcao, GlobalAI).

## Riscos Conhecidos

- **Nao compilado/verificado**: Codigo mesclado+modificado nao foi compilado nesta maquina (sem cadeia de ferramentas), primeira compilacao real pode ter mudancas de interface omitidas (especialmente as grandes alteracoes nas tres series aircraft/airport/train);
- Versao de arquivo: px-patch pode ter alterado SLV (M9 menciona savegame version gate), arquivos jrpm e jgrpp 0.73.x podem nao ser legiveis mutuamente (mesma convencao jgrpp, compatibilidade descendente com arquivos trunk);
- O merge traz todo o historico do pulsexlb, para rastrear origem de recursos use `git log --oneline pulsexlb/px-patch`.