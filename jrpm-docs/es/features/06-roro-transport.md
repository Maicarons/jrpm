---
title: Transporte de vehículos de carretera (RoRo)
---

# Transporte de vehículos de carretera (RoRo: Road-vehicle on Road-vehicle)

> Fuente: pulsexlb/OpenTTD-patches `px-patch` (lote de septiembre de 2026), incorporado a jrpm mediante git merge.

## Descripción general

El "transporte de vehículos de carretera" permite que trenes, barcos y aviones **transporten vehículos de carretera directamente**:

- Los vehículos de carretera ya no tienen que circular por todas partes por sí mismos: pueden "ir de aventón": un portador los lleva a una estación lejana y allí bajan a la carretera para seguir por su cuenta;
- El portador (tren/barco/avión) obtiene la capacidad de llevar vehículos de carretera al **reconvertirse al carga "Vehicles (Road)"** (carga dedicada `VEHC`, que ocupa espacio de carga);
- En el lado del vehículo de carretera, las opciones de pedido "**Esperar para ser transportado**" y "**Bajar aquí**" se emparejan con las de "cargar vehículos de carretera" / "descargar vehículos de carretera" del portador.

## Cómo usarlo

### Lado del portador (tren/barco/avión)

1. Reconvertir el tren en un depósito **manualmente** al carga "Vehicles (Road)" — convertirse en portador solo es posible mediante reconversión manual; la reconversión por pedido no aplica;
2. Activar "**Cargar vehículos de carretera**" en un pedido de estación: el tren sube los vehículos que esperan en esa estación;
3. Opciones de emparejamiento opcionales:
   - "Esperar la carga" (salir solo cuando esté cargado);
   - "Coincidencia de destino": solo subir vehículos cuya estación de bajada declarada coincida con la próxima parada del portador;
   - "Bajar aquí todos los vehículos": bajarlos todos, ignorando la estación declarada por cada uno.

### Lado del vehículo de carretera

1. Establecer "**Esperar para ser transportado**" en un pedido de estación: el vehículo se detiene ahí y espera a un portador;
2. Establecer "**Bajar aquí**": el vehículo baja del portador en esta estación;
3. Las dos opciones son mutuamente excluyentes (una por pedido);
4. Al bajar, el vehículo ejecuta una pasada de búsqueda de ruta para elegir la mejor plataforma.

## Detalles y reglas

- **Ranura de carga dedicada**: el transporte de vehículos de carretera usa la ranura de carga 128 (`NUM_CARGO - 1`), fuera de las 64 ranuras que puede definir un NewGRF; el total de tipos de carga se amplía de 64 a **128**;
- **Detección de portador dedicado**: cuando todas las partes del vehículo están reconvertidas a "Vehicles (Road)", sus botones de pedido muestran por defecto el transporte de vehículos de carretera; los vehículos con carga normal muestran carga normal;
- **Ajuste de partes portadoras**: `vehicle.rv_transport_carrier_parts` decide qué partes pueden llevar vehículos de carretera;
- **Carga entre empresas**: opcionalmente permitir cargar/descargar vehículos de otras empresas con liquidación automática de tarifas;
- **Aviso de "transportado demasiado tiempo"**: una advertencia única cuando un vehículo ha sido llevado demasiado tiempo;
- **Listas de pedidos creadas por el jugador**: las listas compartidas/independientes también admiten las marcas de transporte de vehículos de carretera.

## Compatibilidad con partidas guardadas

- El estado de espera/transporte y las marcas de pedido se guardan;
- Las partidas antiguas (sin la marca XSLFI_CARGO_TYPES_128) se leen con 64 ranuras de carga y siguen siendo compatibles.

## Comandos de consola de depuración (desactivados por defecto)

La familia de comandos `rvtransport` solo se compila con la opción de CMake `RORO_DEBUG_COMMANDS=ON`, para pruebas de regresión.

## Código relacionado

- Núcleo: `src/roadveh_transport.h`, `src/cargo_type.h` (carga VEHC)
- Pedidos: `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_base.h` (OrderExtraInfo)
- Carga de trenes: `src/train_cmd.cpp`, `src/station_cmd.cpp`
- Extensión de carga: `src/sl/station_sl.cpp`, `src/sl/company_sl.cpp` (XSLFI_CARGO_TYPES_128)
