---
title: Desenganche de locomotoras (decouple)
---

# Desenganche de locomotoras (decouple)

> Fuente: Rama de caracteristica `jgrpp-decouple` de pulsexlb/OpenTTD-patches (nucleo de los 152 commits), incorporada a jrpm mediante git merge.

## Introduccion a la funcion

El "desenganche de locomotoras" permite que los trenes **desenganches y enganches** vagones o locomotoras en estaciones/durante el despacho, logrando:

- **Desenganche**: El tren, siguiendo una "orden de desenganche", deja parte de sus vagones (o vagones+locomotora) en una estacion designada, el resto continua su marcha;
- **Enganche**: El tren espera en la estacion y se acopla automaticamente con otro tren (o grupo de vagones estacionados);
- **Dos trenes ejecutan despachos independientes tras el desenganche**: A traves de mecanismos como "salto condicional de ordenes", los dos trenes desenganchados ejecutan diferentes planes de despacho;
- **Transferencia de tokens**: Al desenganchar/enganchar, los tokens (etiquetas de restriccion de seguimiento) se transfieren correctamente con el tren o se deduplican.

## Capacidades principales

| Capacidad | Descripcion |
|---|---|
| Ordenes de desenganche/enganche | Nuevos tipos de ordenes que soportan restricciones como "cargar/no cargar", "esperar enganche", "desenganchar" |
| Limites de longitud y velocidad de acoplamiento | Limita la operacion de acoplamiento segun la longitud del tren; la operacion de acoplamiento tiene limite de velocidad |
| Soporte de doble locomotora | Locomotoras delantera y trasera (incluyendo locomotoras de doble cabezal NewGRF) se acoplan/desenganchan correctamente |
| Busqueda de rutas de acoplamiento | YAPF/NPF soportan planificacion de rutas de acoplamiento; solo en estaciones |
| Marcha en reversa | Puede invertir la marcha despues del acoplamiento, soporta tecnicas de despacho como "reversa tras desenganche" |
| Acoplamiento NewGRF | Soporta propiedades de acoplamiento definidas por NewGRF (propiedades 0xC6/0xF2, etc.) |
| Archivo | Version de archivo `SLV_ORDER_DECOUPLE`; contador `num_decouple` persistente |

## Modo de uso

1. Use la orden de "desenganche" (`decouple`) en la estacion para dividir el tren;
2. Establezca una orden de enganche para que el tren espere la conexion;
3. Combine con "salto condicional de ordenes" (conditional order skip) para que los dos trenes desenganchados tomen diferentes rutas;
4. Arrastre en la estacion para ajustar el orden de los vagones, o use herramientas auxiliares como "filtrar por longitud".

## Codigo relacionado

- Ordenes: `src/order_cmd.cpp`, `src/order_gui.cpp`, `src/order_type.h`
- Tren: `src/train_cmd.cpp`, `src/train.h`
- Busqueda de rutas: `src/pathfinder/yapf`, `src/pathfinder/npf`
- Archivo: `src/sl/saveload_common.h` (`SLV_ORDER_DECOUPLE`)

## Nota

- Esta caracteristica implica cambios profundos en el orden fisico del tren/logica de reserva de senales, **se recomienda encarecidamente realizar pruebas de regresion en una compilacion real**: escenarios de desenganche, enganche, reversa, reparacion de colisiones, reemplazo automatico (autoreplace), etc.;
- Las cadenas de texto relacionadas con el desenganche y la GUI se han incorporado (`STR_DECOUPLE*`, etc.).