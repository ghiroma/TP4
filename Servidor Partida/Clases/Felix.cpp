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
	this->fila = 0;
	this->columna = 0;
	this->puntaje_parcial = 0;
	this->cantidad_vidas = cantVidas;
}

Felix::Felix(int cantVidas, int id) {
	this->id = id;
	this->fila = 0;
	this->columna = 0;
	this->puntaje_parcial = 0;
	this->cantidad_vidas = cantVidas;
}

bool Felix::mover(int columna, int fila, Edificio * edificio) {

	if (fila < edificio->filas && fila >= 0 && columna < edificio->columnas
			&& columna >= 0
			&& (!edificio->ventanas[fila][columna].marquesina
					|| (edificio->ventanas[fila][columna].marquesina
							&& (this->columna != columna
+									|| (this->columna == columna
+											&& this->fila + 1 != fila))))
			&& !edificio->ventanas[fila][columna].ocupado
			&& !edificio->ventanas[fila][columna].persiana) {

		edificio->ventanas[this->fila][this->columna].ocupado = false;
		edificio->ventanas[fila][columna].ocupado = true;
		this->columna = columna;
		this->fila = fila;

		cout << "Salgo validacion movimiento true" << endl;
		return true;
	}
	cout << "Salgo validacion movimiento false" << endl;
	return false;
}

bool Felix::reparar(Edificio * edificio) {
	if (this->fila < edificio->filas && this->fila >= 0
			&& this->columna < edificio->columnas && this->columna >= 0) {

				if (edificio->ventanas[this->fila][this->columna].ventanaRota
				> 0) {
						edificio->ventanas[this->fila][this->columna].ventanaRota--;
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

