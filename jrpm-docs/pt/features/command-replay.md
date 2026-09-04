---
title: Gravacao e Reproducao de Comandos
---

# Gravacao e Reproducao de Comandos (Command Record / Replay)

Portabilidade da reproducao de comandos cmclient (quinto lote, commit `f113acce28`). **Nao porta a camada de objeto de comando do cmclient** -- usa diretamente a infraestrutura de serializacao de comandos nativa do jrpm (`DynBaseCommandContainer`), ignorando completamente a "camada de objeto de comando".

## Comandos de Console

```
cmdrecord [start [file]]    # Inicia gravacao (arquivo padrao cmdrecord.jrcm, salvo no diretorio pessoal)
cmdrecord stop              # Para (atrasa 10 ticks para fila de comandos esvaziar antes de salvar)
cmdreplay <file>            # Reproduz: desserializa e executa cada comando imediatamente
```

## Pontos de Implementacao

- **Gancho de gravacao**: `CommandRecordLog` e inserido apos a execucao do comando em `DoCommandPInternal` -- este e o **unico ponto real de execucao** para comandos locais, de rede e de reproducao, sem duplicacao de registro.
- **Serializacao**: `cmd / tile / error_msg / payload / company` usando `DynBaseCommandContainer::Serialise` para serializacao sem perdas; formato de arquivo: magic `JRCM` + version + count + entradas.
- **Parada atrasada**: `cmdrecord stop` marca um atraso de 10 ticks, `StateGameLoop` verifica a cada quadro (`CommandRecordTick`) para garantir que comandos ja enfileirados sejam capturados antes do flush.
- **Execucao de reproducao**: Cada comando e executado imediatamente como comando de servidor (`DCIF_NETWORK_COMMAND`); mesmo que o cabecalho de count do arquivo nao esteja atualizado, a leitura e feita com base nos dados reais.

## Verificacao

Teste E2E em servidor dedicado: gravar `pause` -> arquivo de 26 bytes -> reproduzir em novo mapa -> `Game paused (manual)` + `Replay finished: 1 executed, 0 failed`.

## Uso Tipico

```
cmdrecord start build1     # Inicia gravacao
# ... no jogo, construa trilhos, estacoes ...
cmdrecord stop             # Para e salva
# Apos trocar de mapa ou recuperar de crash:
cmdreplay build1           # Reconstroi todas as operacoes com um clique
```