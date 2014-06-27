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
	bool keepAlive;
	bool jugando;
};

struct shmIds
{
	int shmId;
	int shmKAId;
	char * semName;
};

#endif /* ESTRUCTURAS_H_ */
