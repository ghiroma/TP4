/*
 * Edificio.h
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#ifndef EDIFICIO_H_
#define EDIFICIO_H_

#include "Ventana.h"

class Edificio {

public:
	int filas;
	int columnas;
	int nivel;
	Ventana **ventanas;

	Edificio(int filas, int columnas, int nivel);
	Edificio(int filas,int columnas);
	virtual ~Edificio();
};

#endif /* EDIFICIO_H_ */
