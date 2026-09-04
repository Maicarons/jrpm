---
title: Guia Online
---

# Guia Online

## Estrategia de Compatibilidade de Versoes

jrpm e uma **versao com tag** (`jrpm-0.1.0`), e o handshake online exige que a string de revisao seja **exatamente correspondente**:

| Cenario | Comportamento |
|---|---|
| Cliente jrpm <-> Servidor jrpm |  Normal (versoes iguais) |
| Cliente jrpm <-> Servidor jgrpp original / pulsexlb |  Recusado (isolamento de versao) |
| Cliente jgrpp original -> Servidor jrpm |  Permitido (servidor aceita revisao `jgrpp-*`) |
| Cliente pulsexlb -> Servidor jrpm |  Permitido (aceita revisao `pxp`) |

::: warning Versao NewGRF
Independentemente da versao do cliente, o **numero da versao NewGRF deve ser exatamente igual ao do servidor** (`_openttd_newgrf_version` e estritamente verificado), esta e a linha de base da simulacao deterministica.
:::

## Configurando um Servidor

```bash
# Servidor dedicado (sem GUI)
openttd-jrpm -D -c server.cfg
```

Configuracao sugerida para `server.cfg` (veja [Ajuste de Desempenho do Servidor](../performance/server-tuning) para detalhes):

```ini
[network]
server_name = My JRPM Server
server_port = 3979
max_clients = 32
max_companies = 15
frame_freq = 3
sync_freq = 50
commands_per_frame = 8
bytes_per_frame = 16
bytes_per_frame_burst = 512
max_join_time = 1000
max_download_time = 2000
max_lag_time = 1200
```

## Entrando em um Servidor

- No jogo: "Multijogador -> Entrar no servidor da Internet/Adicionar servidor", ou
- Linha de comando: `openttd-jrpm -n <host>:<port>`

## Download de Conteudo (NewGRF/Cenarios)

jrpm suporta **multiplos espelhos + download paralelo**:

```ini
[network]
content_server = content.openttd.org        ; Servidor de metadados
content_mirrors = https://binaries.openttd.org/bananas,https://your-mirror.example/bananas
content_download_parallel = 4               ; Numero de arquivos para download paralelo (1-8)
```

- A lista de espelhos e separada por virgulas, usada em ordem; se um espelho falhar, alterna automaticamente para o proximo;
- As variaveis de ambiente `OTTD_CONTENT_MIRROR_URI` / `OTTD_CONTENT_SERVER_CS` tem prioridade sobre as configuracoes;
- Se todos os espelhos falharem, recai automaticamente para o protocolo de download TCP antigo.

## Gerenciamento do Servidor

- Comandos do console: `status`, `clients`, `kick`, `ban`, `save`, `reset_company`, `autogroup` (agrupamento automatico de veiculos), etc.;
- RCON: Apos definir `rcon_password` no servidor, comandos do console podem ser enviados remotamente.