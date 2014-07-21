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

void Edificio::CambiarTramo()
{
	this->nivel++;
	for(int i = 0; i<this->filas;i++)
	{
		for(int c = 0; c<this->columnas;c++)
		{
			this->ventanas[i][c].ventanaRota = 1;
			this->ventanas[i][c].ocupado = false;
		}
	}
}

Edificio::~Edificio() {
	for (int i = 0; i < this->filas; i++) {
		delete[] this->ventanas[i];
	}
	delete[] this->ventanas;
}

