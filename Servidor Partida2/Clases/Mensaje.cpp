/*
 * Mensaje.cpp
 *
 *  Created on: Jul 8, 2014
 *      Author: ghiroma
 */

#include "Mensaje.h"
#include "../Support/Constantes.h"

Mensaje::Mensaje(int jugador, string mensaje, Partida * partida) {
	this->jugador = jugador;
	this->codigo = mensaje.substr(0,LONGITUD_CODIGO);
	this->mensaje = mensaje.substr(LONGITUD_CODIGO,LONGITUD_CONTENIDO);
	this->partida = partida;

}

Mensaje::~Mensaje() {
	// TODO Auto-generated destructor stub
}

