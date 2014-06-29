/*
 * Estructuras.h
 *
 *  Created on: Jun 20, 2014
 *      Author: ghiroma
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

struct puntajes
{
	int idJugador1;
	int idJugador2;
	int puntajeJugador1;
	int puntajeJugador2;
	bool jugador1Alive;
	bool jugador2Alive;
	bool jugando;
	bool keepAlivePartida;
	bool keepAliveTorneo;
};

struct idsSharedResources
{
	int shmId;
	char semNamePartida[100];
	char semNameTorneo[100];
};

#endif /* ESTRUCTURAS_H_ */
