/*
 * Felix.h
 *
 *  Created on: Jun 19, 2014
 *      Author: ghiroma
 */

#ifndef FELIX_H_
#define FELIX_H_

using namespace std;

#include <string>
#include "Edificio.h"

class Felix {
public:
	int id;
	short int fila;
	short int columna;
	short int cantidad_vidas;
	unsigned int puntaje_parcial;
	time_t ultimoChoque;

	Felix(int cantVidas);
	Felix(int cantVidas,int id);
	bool mover(int columna, int fila, Edificio * edificio);
	bool reparar(Edificio * edificio);
	int perderVida();
	virtual ~Felix();
};

#endif /* FELIX_H_ */
