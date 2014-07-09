/*
 * Mensaje.h
 *
 *  Created on: Jul 8, 2014
 *      Author: ghiroma
 */

#ifndef MENSAJE_H_
#define MENSAJE_H_

#include "Partida.h"
#include <string>

class Mensaje {
public:
	Partida * partida;
	int jugador;
	string codigo;
	string mensaje;
	Mensaje(int jugador, string mensaje, Partida * partida);
	virtual ~Mensaje();
};

#endif /* MENSAJE_H_ */
