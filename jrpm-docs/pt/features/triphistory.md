---
title: Historico de Viagens de Veiculos (Trip History)
---

# Historico de Viagens de Veiculos (Trip History)

Originado do **embeddedt/OpenTTD-modded** (primeiro lote de portabilidade), adaptado para as novas APIs de string e data do jrpm.

## Funcionalidade

Cada veiculo lembra os dados das ultimas **10 viagens**, acessiveis pelo novo botao **History (Historico)** na janela de detalhes do veiculo:

| Coluna | Significado |
|---|---|
| Received (Chegada) | Data de recebimento da carga desta viagem |
| Profit (Lucro) | Lucro desta viagem (numeros negativos em amarelo) |
| % Change | Variacao percentual do lucro em relacao a viagem anterior (verde + / vermelho -) |
| TBT | Intervalo de tempo desde a viagem anterior (dias) |
| Change | Variacao de tempo em relacao a viagem anterior (dias) |
| Occupancy | Taxa media de ocupacao de passageiros/carga desta viagem (%) |

A parte inferior da janela tambem exibe estatisticas resumidas:

- **Total income for the last N trips**: Lucro total das N viagens + lucro medio por dia de cada viagem
- **Average trip length**: Intervalo medio entre viagens (dias)
- **Improvement over last N trips**: Variacao percentual综合 do lucro

## Mecanismo de Registro

- **AddValue**: Quando um veiculo completa uma viagem (destrutor de `CargoPayment`), registra lucro, data, taxa de ocupacao e distancia entre estacoes
- **NewRound**: Quando um veiculo chega ao primeiro destino conforme o horario, inicia uma nova viagem
- **Ocupacao**: Coletada quando o veiculo sai da estacao (reutiliza o mecanismo `trip_occupancy` ja existente no jrpm)

## Notas de Implementacao

- Os dados sao **NOSAVE** (apenas em tempo de execucao, nao gravados no arquivo), portanto nao e necessario aumentar a versao do arquivo, totalmente compativel com arquivos antigos
- Nova classe de janela `WindowClass::VehicleTripHistory`, ID da janela e o ID do veiculo
- A janela de detalhes do veiculo (trem/nao-trem) ganha um novo botao History na barra de titulo
- Quando o veiculo e excluido, a janela de historico e fechada automaticamente

## Arquivos Envolvidos

- `src/triphistory.h` / `src/triphistory_cmd.cpp` / `src/triphistory_gui.cpp` (novos)
- `src/vehicle_base.h` (Vehicle adiciona campo `trip_history`)
- `src/economy.cpp` (destrutor de CargoPayment registra viagem)
- `src/timetable_cmd.cpp` (chegada ao primeiro destino inicia nova viagem)
- `src/vehicle_gui.cpp` / `src/widgets/vehicle_widget.h` (botao History)
- `src/lang/english.txt` (strings STR_TRIP_HISTORY_*)