/*
 * Edificio.cpp
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#include "Edificio.h"

Edificio::Edificio(int filas, int columnas, int nivel)
{
	this->filas = filas;
		this->columnas = columnas;
		this->nivel = nivel;
		//this->ventanas = Ventana[this->filas][this->columnas];

		/*for(int fil=0;fil<this->filas;fil++)
		{
			for(int col=0;col<this->columnas;col++)
			{
				this->ventanas[fil][col] =
			}
		}*/
}

Edificio::Edificio(int filas, int columnas) {
	this(filas,columnas,0);
}

Edificio::~Edificio() {
	// TODO Auto-generated destructor stub
}

