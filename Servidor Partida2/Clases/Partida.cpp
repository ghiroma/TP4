/*
 * Partida.cpp
 *
 *  Created on: Jul 8, 2014
 *      Author: ghiroma
 */

#include "Partida.h"
#include "../Support/Constantes.h"

Partida::Partida(int id) {
	// TODO Auto-generated constructor stub
	this->cSocket1 = 0;
	this->cSocket2 = 0;
	this->estado = ESTADO_JUGANDO;
	this->felix1 = 0;
	this->felix2 = 0;
	this->edificio = new Edificio(EDIFICIO_FILAS_1,EDIFICIO_COLUMNAS,0);
	this->nivel = 0;
	this->id = id;
}

Partida::~Partida() {
	delete(cSocket1);
	delete(cSocket2);
	delete(felix1);
	delete(felix2);
	delete(edificio);
	// TODO Auto-generated destructor stub
}

