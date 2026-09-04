---
title: "Download de Recursos: Multiplos Espelhos + Download Paralelo"
---

## Situacao Atual (Conclusao da Pesquisa)

| Item | Situacao |
|---|---|
| Servidor de conteudo (protocolo de metadados) | `src/network/core/config.cpp` `NetworkContentServerConnectionString()`: variavel de ambiente `OTTD_CONTENT_SERVER_CS`, padrao `content.openttd.org` (porta TCP 3978) |
| Espelho de download | `NetworkContentMirrorUriString()`: variavel de ambiente `OTTD_CONTENT_MIRROR_URI`, padrao `https://binaries.openttd.org/bananas` |
| Metodo de download | `network_content.cpp` `DownloadSelectedContentHTTP()`: **unica** requisicao POST com todos os IDs de conteudo -> espelho retorna lista de cabecalhos de arquivo (`id,type,filesize,url` por linha) -> download **serial, um por um** de cada arquivo (um GET por arquivo) -> `AfterDownload()` descompacta gunzip + tar |
| Modelo de threads | Camada HTTP (WinHttp) ja processa em thread de fundo, mas **download de arquivos e em fila serial**; sem lista de espelhos, sem configuracao no jogo, sem paralelismo |

## Implementacao deste Recurso

### 1. Novas Configuracoes (`network_settings.ini` + `settings_type.h`)

- `network.content_server` (SLE_STR, padrao vazio = usar fonte oficial)
- `network.content_mirrors` (SLE_STR, multiplos URIs de espelho separados por virgula, padrao vazio = espelho oficial)

Prioridade: variavel de ambiente > configuracao do jogo > padrao oficial.

### 2. Analise de Configuracao (`src/network/core/config.cpp/h`)

- `NetworkContentServerConnectionString()` le a configuracao;
- Novo `NetworkContentMirrorUris()` analisa a lista separada por virgulas (remove espacos, pula itens vazios, usa fonte oficial como fallback);
- `NetworkContentMirrorUriString()` alterado para retornar o primeiro da lista.

### 3. Download Paralelo (`src/network/network_content.h/.cpp`)

- Novo `ContentFileDownload` (unico arquivo a ser baixado: id/type/filesize/url/filename);
- Novo `ContentDownloadSession : HTTPCallback` (estado de download independente e callback para cada arquivo; `IsCancelled` vinculado ao handler);
- `DownloadSelectedContentHTTP()`: POST para `mirrors[mirror_index]` -> `ParseResponseHeaders()` analisa todos os cabecalhos de arquivo de uma vez -> `StartDownloadSessions()` inicia ate **4 sessoes paralelas** (`CONTENT_DOWNLOAD_PARALLEL`), cada sessao pega automaticamente o proximo arquivo a ser baixado ao finalizar;
- Cadeia de retentativa em caso de falha: falha de sessao/falha de requisicao ao espelho -> proximo espelho faz nova requisicao -> todos os espelhos falham -> recai para protocolo TCP antigo (`DownloadSelectedContentFallback`);
- Finalizacao elegante: em caso de falha/cancelamento, define `download_cancelled`, aguarda todas as sessoes em andamento terminarem (`OnAllSessionsDone`) antes de tentar novamente ou recuar, evitando callbacks pendentes;
- `ResetMirrorIndex()`: novo download comeca do primeiro espelho (chamado ao acionar GUI).

## Arquivos Envolvidos

- `src/table/settings/network_settings.ini` (2 novas configuracoes)
- `src/settings_type.h` (2 novos campos em `NetworkSettings`)
- `src/network/core/config.cpp` / `config.h` (analise de lista de espelhos)
- `src/network/network_content.h` / `.cpp` (sessoes paralelas)
- `src/network/network_content_gui.cpp` (reinicia indice de espelho antes do download)

## Pontos de Verificacao

1. Apos preencher `network.content_mirrors` com varios URIs (separados por virgula), o download de conteudo deve funcionar e baixar varios arquivos **em paralelo** (observe o progresso do download/capture multiplas conexoes simultaneas);
2. Em caso de rede indisponivel/espelho incorreto, deve tentar automaticamente o proximo espelho e, por fim, recair para o protocolo antigo;
3. Cancelamento durante o download nao deve deixar `.tar.gz` incompletos;
4. Necessario compilar e verificar em maquina real (este repositorio nao possui ambiente de construcao, codigo nao compilado).

## Limitacoes Conhecidas

- O nivel de paralelismo esta hardcoded como 4 (`CONTENT_DOWNLOAD_PARALLEL`), podendo ser transformado em configuracao futuramente;
- O protocolo de espelho depende do formato oficial da API bananas (POST com lista de IDs retorna lista de cabecalhos de arquivo).