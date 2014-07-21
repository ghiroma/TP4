/*
 * Timer.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: ghiroma
 */

#include <string.h>
#include "Timer.h"
#include "../Support/Constantes.h"
#include "../Support/Helper.h"
#include <stdio.h>
#include <stdlib.h>

using namespace std;

Timer::Timer() {
	this->startingTimeKeepAlive = time(0);
	this->startingTimeRalph = time(0);
	this->startingTimePaloma = time(0);
	this->startingTimePersiana = time(0);
	this->startingTimeTorta = time(0);
	this->filaPersianaAnterior = -1;
	this->columnaPersianaAnterior = -1;
}

string Timer::ralph(int nivel) {
	string message;
	char aux[5];

	if (nivel == 0) {
		if (Helper::timeDifference(INTERVALOS_RALPH_1, this->startingTimeRalph)) {
			message.append(CD_MOVIMIENTO_RALPH);
			sprintf(aux, "%d", this->randomRalphMovement());
			message.append(Helper::fillMessage(aux));
			this->startingTimeRalph = time(0);
		}
	} else if (nivel == 1) {
		if (Helper::timeDifference(INTERVALOS_RALPH_2, this->startingTimeRalph)) {
			message.append(CD_MOVIMIENTO_RALPH);
			sprintf(aux, "%d", this->randomRalphMovement());
			message.append(Helper::fillMessage(aux));
			this->startingTimeRalph = time(0);
		}
	}
	return message;
}

string Timer::paloma(int nivel) {
	string message;
	char aux[5];

	if (nivel == 0) {
		if (Helper::timeDifference(INTERVALOS_PALOMA_1, this->startingTimePaloma)) {
			message.append(CD_PALOMA);
			sprintf(aux, "%d", randomPaloma(nivel));
			message.append(Helper::fillMessage(aux));
			this->startingTimePaloma = time(0);
		}
	} else if (nivel == 1) {
		if (Helper::timeDifference(INTERVALOS_PALOMA_2, this->startingTimePaloma)) {
			message.append(CD_PALOMA);
			sprintf(aux, "%d", randomPaloma(nivel));
			message.append(Helper::fillMessage(aux));
			this->startingTimePaloma = time(0);
		}
	}

	return message;
}

string Timer::keepAlive() {
	string message;

	if (Helper::timeDifference(INTERVALOS_KEEPALIVE, this->startingTimeKeepAlive)) {
		message.append(CD_ACK);
		message.append(Helper::fillMessage("0"));
		this->startingTimeKeepAlive = time(0);
	}

	return message;
}

/*
 * Genero aparicion random de persiana.
 */
char * Timer::randomPersiana() {
	char location[3];
	char aux[2];

//No hay persiana cerrada. Actualizo el edificio.
	if (this->filaPersianaAnterior != -1 && this->columnaPersianaAnterior != -1) {
		sprintf(aux, "%d", rand() % (EDIFICIO_FILAS_1));
		strcpy(location, aux);
		this->filaPersianaAnterior = atoi(aux);
		sprintf(aux, "%d", rand() % (EDIFICIO_COLUMNAS));
		strcat(location, aux);
		this->columnaPersianaAnterior = atoi(aux);
		return location;
	} else //Hay persiana cerrada. Debo abrirla.
	{
		sprintf(aux, "%d", this->filaPersianaAnterior);
		strcpy(location, aux);
		sprintf(aux, "%d", this->columnaPersianaAnterior);
		strcat(location, aux);
		this->filaPersianaAnterior = -1;
		this->columnaPersianaAnterior = -1;
		return location;
	}

}

/*string Timer::torta(int nivel)
 {

 }*/

/*string randomTorta() {
 char location[3];
 char aux[2];

 sprintf(aux, "%d", rand() % (EDIFICIO_FILAS_1));
 strcpy(location, aux);
 sprintf(aux, "%d", rand() % (EDIFICIO_COLUMNAS));
 strcat(location, aux);

 return location;
 }*/

int Timer::randomPaloma(int nivel) {
	return rand() % (EDIFICIO_FILAS_1);
}

int Timer::randomRalphMovement() {
	return rand() % (EDIFICIO_COLUMNAS);
}

Timer::~Timer() {
	// TODO Auto-generated destructor stub
}

