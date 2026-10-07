---
title: Ferry de trens e chuva
---

# Ferry de trens e chuva

> Fonte: pulsexlb/OpenTTD-patches `px-patch` (lote 2026-10, `pxp-2610.1` – `pxp-2610.3`), mesclado no jrpm via git merge.

Este lote traz duas grandes novidades: o **ferry de trens** (navios transportando trens inteiros) e o **sistema de chuva** (com escurecimento do mundo e camada de gotas).

## Ferry de trens

Para além do transporte de veículos rodoviários (RoRo) já existente, onde carros são carregados em trens, navios e aeronaves, agora os navios também podem transportar **trens inteiros**.

### Relação com o RoRo

| | Transporte de veículos rodoviários (VEHC) | Transporte de trens (RAIL) |
|---|---|---|
| Transportadores | trens, navios, aeronaves | navios |
| Objeto transportado | veículos rodoviários | trens (locomotivas e vagões) |
| Rótulo de carga | `VEHC` | `RAIL` |
| Slot de carga | `NUM_CARGO - 1` | `NUM_CARGO - 2` |

Os dois slots de carga ficam fora dos 64 slots do NewGRF, portanto nunca entram em conflito.

### Carregamento

- Um navio adquire a capacidade de transportar trens inteiros ao ser **re-equipado manualmente** com a carga "Veículos (Trem)";
- **Carregamento por vagão**: um trem pode ser distribuído por vários porões em vez de precisar caber inteiro em um só, o que elimina o problema de trens longos que excedem a capacidade de um porão;
- O **seletor de placa** nas opções de transporte oferece separadamente placas de trem e de veículo, de modo que cada tipo de transporte possa declarar seu próprio destino;
- A barra de status não lista mais os veículos transportados em detalhe, liberando espaço para informações de operação mais importantes.

### Exibição de capacidade

- A **janela de informações do navio** mostra o peso de carga que cada porão comporta;
- A **janela de informações do trem** mostra a capacidade de cada vagão.

### Correções relacionadas

Este lote também corrige vários defeitos de carga, descarga e reserva de plataforma:

- navios julgavam incorretamente o tipo de trilho incompatível ao descarregar trens, impedindo que eles descessem;
- a reserva de plataforma não era liberada na subida de um trem, e ocorriam erros de estrutura e de reserva na descida;
- reservas de plataforma obsoletas impediam que trens voltassem a descarregar nas plataformas;
- certas orientações de plataforma recusavam a descarga de um trem;
- o limite de vagões não era aplicado aos veículos re-equipados para transporte.

## Sistema de chuva

Uma simulação meteorológica puramente decorativa: afeta apenas o visual e não mexe na lógica do jogo nem nos valores econômicos.

### Apresentação do tempo

- **Períodos de chuva aleatórios**: o estado do tempo é conduzido por um gerador congruencial determinístico semeado com a semente de geração do mapa, de modo que todos os jogadores e o servidor veem exatamente o mesmo tempo;
- **Escurecimento gradual do mundo**: o mundo escurece aos poucos durante a chuva e clareia quando ela para. O escurecimento percorre `RAIN_SHADE_LEVELS` níveis e é lido ao desenhar as janelas de visualização;
- **Camada de gotas**: uma camada de chuva em tela cheia que **se adapta ao zoom** e tem **vibração aleatória**, para que as gotas não pareçam uma textura estática.

### Opção e cheat

- **Opção de dificuldade** `difficulty.rain`: decide na criação de um jogo se o escurecimento do mundo durante a chuva está ativado;
- **Cheat de sandbox** "Tempo": um ciclo de três estados — automático / chuva forçada / sol forçado.

### Arquivos salvos

O estado do tempo (se está chovendo agora, a semente do gerador, o início do período de chuva atual) e o cheat de tempo do sandbox são **salvos junto com o jogo**; após carregar, o escurecimento salta direto para o tempo atual em vez de surgir novamente em fade.

- Bloco: `WTHR`;
- Flag de funcionalidade: `XSLFI_WEATHER`;
- Código: `src/weather.cpp`, `src/weather.h`, `src/sl/weather_sl.cpp`.

## Código relacionado

- Ferry de trens e capacidades: `src/roadveh_transport.cpp`, `src/cargo_type.h` (`CT_RAILVEHICLES`), `src/table/cargo_const.h`
- Efeitos visuais da chuva: `src/weather.cpp`, `src/blitter/32bpp_anim.cpp`, `src/blitter/40bpp_anim.cpp`, `src/viewport.cpp`
- Salvamento do tempo: `src/sl/weather_sl.cpp`, `src/sl/extended_ver_sl.cpp` (`XSLFI_WEATHER`)
- Configurações e cheat: `src/table/settings/difficulty_settings.ini` (`difficulty.rain`), `src/cheat_gui.cpp`, `src/cheat_type.h`
