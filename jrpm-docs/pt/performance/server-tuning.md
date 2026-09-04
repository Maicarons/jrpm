---
title: Guia de Ajuste de Desempenho do Servidor
---

> Baseado no branch jrpm apos mesclagem (incluindo recursos pulsexlb e otimizacoes exclusivas do jrpm).
> Valores padrao obtidos de `src/table/settings/network_settings.ini`, recomenda-se ajustar com base em testes de carga reais.

## I. Otimizacoes de Desempenho Ja Implementadas (nesta versao)

| Otimizacao | Commit | Descricao |
|---|---|---|
| Concorrencia de download paralelo configuravel | 4716b925 | `network.content_download_parallel` (padrao 4, 1-8): numero de arquivos baixados simultaneamente, substituindo o valor hardcoded 4 |
| Multiespelho + download paralelo em nivel de arquivo | d4c45740 (F1) | `network.content_mirrors` multiplos espelhos separados por virgula, tenta novamente por espelho em caso de falha, recai para protocolo antigo |
| Compatibilidade de multiplas versoes de cliente no servidor | cb9848b7 | Servidor pode aceitar simultaneamente clientes jrpm / jgrpp original / pulsexlb |

## II. Parametros Chave de Desempenho do Servidor (sistema jgrpp)

| Configuracao | Padrao | Faixa | Significado | Sugestao de Ajuste |
|---|---|---|---|---|
| `network.sync_freq` | 100 | 0-100 | A cada quantos quadros fazer uma verificacao de sincronizacao (deteccao de desync). Quanto maior, menos largura de banda; quanto menor, mais cedo detecta perda de sincronia | Prioridade de estabilidade online: reduzir para 20-50; se desync frequente, reduzir |
| `network.frame_freq` | 0 | 0-100 | A cada quantos quadros o servidor empacota e envia quadros de comando (0 = enviar a cada quadro). Quanto maior, menos largura de banda/CPU, mas maior latencia de operacao | Normal 0-3; servidores com muitos jogadores podem usar 5, ajustar apos teste de carga |
| `network.commands_per_frame` | 2 | 1-65535 | Limite maximo de comandos de cliente processados por quadro (anti-spam/ataque) | Com muitos jogadores e operacoes frequentes, aumentar para 4-8 |
| `network.commands_per_frame_server` | 16 | 1-65535 | Limite maximo de comandos do proprio servidor por quadro | Geralmente nao precisa alterar |
| `network.bytes_per_frame` | 8 | 1-65535 | Limite maximo de bytes recebidos por quadro em media (modelagem de largura de banda) | Com banda larga boa, aumentar para 16-32, melhora velocidade de sincronizacao de mapas grandes |
| `network.bytes_per_frame_burst` | 256 | 1-65535 | Limite maximo de bytes em burst (permite picos curtos) | Ajustar junto com o item acima, ex: 512 |
| `network.max_init_time` | 60 | 0-32000 | Tempo limite de inicializacao do cliente (ticks) | Com muitos jogadores de rede fraca, pode aumentar |
| `network.max_join_time` | 500 | 0-32000 | Tempo limite de entrada do cliente (download de mapa + sincronizacao) (ticks) | Para mapas grandes/banda lenta, aumentar para 1000+ |
| `network.max_download_time` | 1000 | 0-32000 | Tempo limite de download de mapa (ticks) | Para mapas grandes (4096+), recomenda-se 2000+ |
| `network.max_lag_time` | 800 | 0-32000 | Tolerancia maxima de lag do cliente (ticks) | Com muitos jogadores de alta latencia, pode aumentar, mas aumenta risco de desync |

> Nota: Unidade de `max_*_time` e ticks de jogo (1/74 segundo ~= 13,5ms); `bytes_per_frame` refere-se a media de bytes da janela de sincronizacao por quadro.

## III. Configuracao Inicial Recomendada para Servidor (cenario multijogador)

```ini
[network]
max_clients = 32            ; ou conforme necessidade
max_companies = 15
frame_freq = 3              ; 0=enviar comando a cada quadro (mais fluido); 3=compromisso para economizar banda
sync_freq = 50              ; detecta perda de sincronia mais cedo
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
content_download_parallel = 4   ; concorrencia de download de conteudo do lado do cliente
```

## IV. Direcoes de Otimizacao de Desempenho Futuras (por ordem de prioridade)

1. **Pool de threads da camada de transporte HTTP + download por partes Range** (ideia do Openttd-Cluster 0007)
   Situacao: F1 ja implementou "paralelismo em nivel de arquivo" (multiplos arquivos baixados simultaneamente). O 0007 adiciona pool de threads na camada HTTP + `CURLOPT_RANGE` para download por partes, que pode acelerar ainda mais **arquivos grandes individuais** (cenarios gigantes .tar.gz). Ambos sao complementares, mas alteram os mesmos arquivos, recomenda-se fazer incrementalmente apos F1 estabilizar.

2. **Otimizacao de envio de mapa**
   Verificar se jrpm ativa compressao de mapa por padrao (zstd/lzma); mapas grandes podem comparar tempo de diferentes algoritmos de compressao.

3. **Runtime de servidor Rust (arquitetura futura de referencia)**
   O otc-engine do Openttd-Cluster (Admin/RCON, metricas Prometheus, failover de cluster, painel Web, ponte de snapshot) depende de integracao FFI completa com projeto Rust, e uma reforma de "proxima geracao" de servidor, nao uma mesclagem em nivel de patch; jrpm atualmente mantem binario unico puro C++.

4. **Desempenho da simulacao do jogo**
   - Em mapas grandes com muitos veiculos, atentar para parametros de `economy`/`linkgraph` (linkgraph_settings.ini);
   - Se precisar de desempenho extremo, estudar classes `settings_game.economy.` e parametros de limite do pathfinder (configuracoes `pathfinding`).

## V. Sugestoes de Verificacao

- Abrir servidor `openttd-jrpm -D -c server.cfg`, usar varios clientes jrpm para testar: tempo de entrada, estabilidade de sincronizacao de quadros (sem aviso de desync), uso de CPU/memoria;
- Download de conteudo: configurar `content_download_parallel=8` e comparar tempo de download de um lote de NewGRF;
- Teste de carga em rede fraca: combinar `max_lag_time` e `sync_freq` para encontrar o limite de estabilidade.