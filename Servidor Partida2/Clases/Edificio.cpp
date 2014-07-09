/*
 * Edificio.cpp
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#include "Edificio.h"

Edificio::Edificio(int filas, int columnas, int nivel) {
	this->filas = filas;
	this->columnas = columnas;
	this->nivel = nivel;
	this->ventanas = new Ventana *[this->filas];
	for (int i = 0; i < this->filas; i++) {
		this->ventanas[i] = new Ventana[this->columnas];
	}

}

Edificio::Edificio(int filas, int columnas) {

}

Edificio::~Edificio() {
	for (int i = 0; i < this->filas; i++) {
		delete (this->ventanas[i]);
	}
	delete (this->ventanas);
}

