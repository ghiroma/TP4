/*
 * Ventana.cpp
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#include "Ventana.h"

Ventana::Ventana() {
	// TODO Auto-generated constructor stub
	this(false,false,false,false,0);
}

Ventana::Ventana(bool jugador1,bool jugador2, bool torta, bool marquesina, int rota)
{
	this->jugador1 = jugador1;
	this->jugador2 = jugador2;
	this->torta = torta;
	this->marquesina = marquesina;
	this->ventanaRota = rota;
}

Ventana::Ventana(bool torta,bool marquesina,int rota)
{
	this(false,false,torta,marquesina,rota);
}

Ventana::Ventana(int rota)
{
	this(false,false,false,false,rota);
}


Ventana::~Ventana() {
	// TODO Auto-generated destructor stub
}

