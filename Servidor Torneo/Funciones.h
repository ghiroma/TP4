/*
 * Funciones.h
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_
#include "Jugador.h"
#include <list>

void getConfiguration(unsigned int* port, string* ip, int* duracionTorneo, int* tiempoInmunidad, int* cantVidas);
void SIGINT_Handler(int inum);
void nuevoJugador(list<Jugador> listJugadores, int idJugador);


#endif /* FUNCIONES_H_ */
