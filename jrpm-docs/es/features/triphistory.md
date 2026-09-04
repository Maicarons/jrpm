---
title: Historial de viajes de vehiculos (Trip History)
---

# Historial de viajes de vehiculos (Trip History)

Originado de **embeddedt/OpenTTD-modded** (primera tanda de portabilidad), adaptado a las nuevas API de cadenas y fechas de jrpm.

## Funcion

Cada vehiculo recuerda los datos de los ultimos **10 viajes**, visibles en el nuevo boton **History (Historial)** en la ventana de detalles del vehiculo:

| Columna | Significado |
|---|---|
| Received (Llegada) | Fecha en que se recibio la carga en este viaje |
| Profit (Ganancia) | Ganancia de este viaje (numeros negativos en amarillo) |
| % Change | Cambio porcentual de ganancia respecto al viaje anterior (verde + / rojo -) |
| TBT | Intervalo de tiempo respecto al viaje anterior (dias) |
| Change | Cambio de tiempo respecto al viaje anterior (dias) |
| Occupancy | Tasa media de carga/pasajeros de este viaje (%) |

En la parte inferior de la ventana tambien hay estadisticas resumidas:

- **Total income for the last N trips**: Ganancia total de N viajes + ganancia media diaria por viaje
- **Average trip length**: Intervalo medio entre viajes (dias)
- **Improvement over last N trips**: Porcentaje de cambio綜合 de ganancia

## Mecanismo de registro

- **AddValue**: Cuando un vehiculo completa un transporte (destructor de `CargoPayment`), registra ganancia, fecha, ocupacion, distancia entre estaciones
- **NewRound**: Cuando el vehiculo llega al primer destino segun el horario, inicia un nuevo viaje
- **Ocupacion**: Se recopila cuando el vehiculo sale de la estacion (reutiliza el mecanismo `trip_occupancy` existente de jrpm)

## Notas de implementacion

- Los datos son **NOSAVE** (solo en tiempo de ejecucion, no se escriben en el archivo), por lo tanto no es necesario aumentar la version del archivo, los archivos antiguos son totalmente compatibles
- Nueva clase de ventana `WindowClass::VehicleTripHistory`, ID de ventana es el ID del vehiculo
- La barra de titulo de la ventana de detalles del vehiculo (tren/no tren) tiene un nuevo boton History
- Cuando se elimina un vehiculo, la ventana de historial se cierra automaticamente

## Archivos involucrados

- `src/triphistory.h` / `src/triphistory_cmd.cpp` / `src/triphistory_gui.cpp` (nuevos)
- `src/vehicle_base.h` (Vehicle nuevo campo `trip_history`)
- `src/economy.cpp` (destructor de CargoPayment registra el viaje)
- `src/timetable_cmd.cpp` (llegada a la primera estacion inicia un nuevo viaje)
- `src/vehicle_gui.cpp` / `src/widgets/vehicle_widget.h` (boton History)
- `src/lang/english.txt` (cadenas STR_TRIP_HISTORY_*)