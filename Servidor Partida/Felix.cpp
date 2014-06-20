/*
 * Felix.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: ghiroma
 */

#include "Felix.h"

Felix::Felix() {
	this->nombre = "";
	this->posicion_x = -1;
	this->posicion_y = -1;
	this->puntaje_parcial = 0;
}

Felix::Felix(string nombre) {

	this->nombre = nombre;
	this->posicion_x = -1;
	this->posicion_y = -1;
	this->puntaje_parcial = 0;
}

Felix::Felix(string nombre, int x, int y) {
	this->nombre = nombre;
	this->posicion_x = x;
	this->posicion_y = y;
	this->puntaje_parcial = 0;
}

Felix::~Felix() {
	// TODO Auto-generated destructor stub
}

