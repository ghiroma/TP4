/*
 * Ventana.cpp
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#include "Ventana.h"

Ventana::Ventana() {
	// TODO Auto-generated constructor stub
	this->jugador1 = false;
	this->jugador2 = false;
	this->marquesina = false;
	this->torta = false;
	this->ventanaRota = 0;
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
	this->jugador1 = false;
	this->jugador2 = false;
	this->torta = torta;
	this->marquesina = marquesina;
	this->ventanaRota = rota;
}

Ventana::Ventana(int rota)
{
	this->jugador1 = false;
	this->jugador2 = false;
	this->marquesina = false;
	this->torta = false;
	this->ventanaRota = rota;
}


Ventana::~Ventana() {
	// TODO Auto-generated destructor stub
}

