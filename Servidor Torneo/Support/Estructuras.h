/*
 * Estructuras.h
 *
 *  Created on: Jul 20, 2014
 *      Author: ghiroma
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <pthread.h>

struct thTemporizador_data {
	int duracion;
	pthread_t thAceptarJugadores;
	pthread_t thEstablecerPartidas;
};

struct thModoGrafico_data {
	int duracion;
};

struct puntajesPartida {
	int idJugador1;
	int idJugador2;
	int puntajeJugador1;
	int puntajeJugador2;
	bool partidaFinalizadaOK;
};

struct datosPartida {
	pid_t pidPartida;
	int idJugador1;
	int idJugador2;
	bool libre;
};

#endif /* ESTRUCTURAS_H_ */
