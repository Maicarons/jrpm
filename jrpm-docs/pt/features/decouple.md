---
title: Desacoplamento de Locomotivas (decouple)
---

# Desacoplamento de Locomotivas (decouple)

> Fonte: Branch de recurso `jgrpp-decouple` do pulsexlb/OpenTTD-patches (nucleo dos 152 commits), incorporado ao jrpm via git merge.

## Introducao a Funcionalidade

O "desacoplamento de locomotivas" permite que trens **desenganchem e engatem** vagoes ou locomotivas em estacoes/durante a programacao, possibilitando:

- **Desengate**: O trem, seguindo uma "ordem de desengate", solta parte dos vagoes (ou vagoes + locomotiva) em uma estacao designada, e o restante continua a viagem;
- **Engate**: O trem aguarda em uma estacao e se acopla automaticamente a outro trem (ou grupo de vagoes estacionado);
- **Apos o desengate, os dois trens executam programacoes independentes**: Atraves de mecanismos como "pular condicionalmente comandos", os dois trens apos o desengate seguem programacoes diferentes;
- **Transferencia de tiquetes**: Ao desengatar/engatar, os tiquetes (etiquetas de rastreamento de restricao) sao transferidos corretamente ou deduplicados entre os trens.

## Capacidades Principais

| Capacidade | Descricao |
|---|---|
| Ordens de desengate/engate | Novos tipos de ordem, suportando restricoes como "carregar/nao carregar", "aguardar engate", "desengatar" |
| Limites de comprimento e velocidade de acoplamento | Limita operacoes de acoplamento com base no comprimento do trem; operacoes de acoplamento tem limite de velocidade |
| Suporte a cabecas duplas | Locomotivas dianteira e traseira (incluindo locomotivas duplas NewGRF) acoplam/desacoplam corretamente |
| Navegacao de acoplamento | YAPF/NPF suportam planejamento de rota de acoplamento; apenas em estacoes |
| Marcha a re | Pode inverter a direcao apos acoplamento, suporta tecnicas como "inverter apos desengate" |
| Acoplamento NewGRF | Suporta atributos de acoplamento definidos por NewGRF (atributos 0xC6/0xF2, etc.) |
| Arquivo | Versao de arquivo `SLV_ORDER_DECOUPLE`; contador `num_decouple` persistido |

## Como Usar

1. Use ordens de "desengate" (`decouple`) na estacao para dividir o trem;
2. Defina ordens de engate para o trem aguardar a conexao;
3. Use "pular condicionalmente comandos" (conditional order skip) para que os dois trens apos o desengate sigam rotas diferentes;
4. Arraste na estacao para ajustar a ordem dos vagoes, ou use ferramentas auxiliares como "filtrar por comprimento".

## Codigo Relacionado

- Ordens: `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_type.h`
- Trem: `src/train_cmd.cpp`, `src/train.h`
- Navegacao: `src/pathfinder/yapf`, `src/pathfinder/npf`
- Arquivo: `src/sl/saveload_common.h` (`SLV_ORDER_DECOUPLE`)

## Atencao

- Este recurso envolve alteracoes profundas na ordem fisica dos trens/logica de reserva de sinal, **recomenda-se testar exaustivamente apos compilacao real**: desengate, engate, reversao, reparo de colisao, substituicao automatica (autoreplace), etc.;
- Strings e GUI relacionadas ao desengate ja foram incorporadas (`STR_DECOUPLE*`, etc.).