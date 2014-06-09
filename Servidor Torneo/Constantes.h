/*
 * Constantes.h
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#ifndef CONSTANTES_H_
#define CONSTANTES_H_

#define ARRIBA "Movimiento Arriba"
#define ABAJO "Movimiento Abajo"
#define DERECHA "Movimiento Derecha"
#define IZQUIERDA "Movimiento Izquierda"
#define REPARAR "Tecla de Reparar"
#define SALIR "Tecla para Salir"

enum Codigo_Mensaje {
    Movimiento_Ralph = 00,
    Movimiento_Felix = 01,
    Vida_Perdida =02,
    Torta_Comida = 03,
    Torta_Generada = 04,
    Persiana = 05,
    Jugador_Desconectado = 06,
    Nivel_Superado = 07,
    Generar_Ventana_Rota = 10,
    Ventana_Arreglada = 11,
    Generar_Paloma=12
};

enum Intervalos{
    Ralph_1 = 1,
    Ralph_2 = 1,
    Ladrillo_1 = 2,
    Ladrillo_2 = 1,
    Paloma = 2,
};


#endif /* CONSTANTES_H_ */
