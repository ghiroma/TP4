/*
 * Felix.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: ghiroma
 */

#include "Felix.h"
#include <iostream>

using namespace std;

Felix::Felix(int cantVidas) {
	this->id = 0;
	this->posicion_x = 0;
	this->posicion_y = 0;
	this->puntaje_parcial = 0;
	this->cantidad_vidas = cantVidas;
}

Felix::Felix(int cantVidas, int id) {
	this->id = id;
	this->posicion_x = 0;
	this->posicion_y = 0;
	this->puntaje_parcial = 0;
	this->cantidad_vidas = cantVidas;
}

bool Felix::mover(int fila, int columna, Edificio * edificio) {

	if (fila < edificio->filas && fila >= 0 && columna < edificio->columnas
			&& columna >= 0
			&& (!edificio->ventanas[fila][columna].marquesina
					|| (edificio->ventanas[fila][columna].marquesina
							&& (this->posicion_y != columna
									|| (this->posicion_y == columna
											&& this->posicion_x + 1 != fila))))
			&& !edificio->ventanas[fila][columna].ocupado
			&& !edificio->ventanas[fila][columna].persiana) {

		edificio->ventanas[this->posicion_x][this->posicion_y].ocupado = false;
		edificio->ventanas[fila][columna].ocupado = true;
		this->posicion_x = fila;
		this->posicion_y = columna;

		cout << "Salgo validacion movimiento true" << endl;
		return true;
	}
	cout << "Salgo validacion movimiento false" << endl;
	return false;
}

bool Felix::reparar(Edificio * edificio) {
	if (this->posicion_x < edificio->filas && this->posicion_x >= 0
			&& this->posicion_y < edificio->columnas && this->posicion_y >= 0) {

		if (edificio->ventanas[this->posicion_x][this->posicion_y].ventanaRota
				> 0) {
			edificio->ventanas[this->posicion_x][this->posicion_y].ventanaRota--;
			return true;
		}
	}
	return false;
}

bool Felix::perderVida() {
	this->cantidad_vidas--;
	if (this->cantidad_vidas <= 0) {
		return true;
	}
	return false;
}

Felix::~Felix() {
// TODO Auto-generated destructor stub
}

