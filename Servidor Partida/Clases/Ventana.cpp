/*
 * Ventana.cpp
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#include "Ventana.h"

Ventana::Ventana() {
	// TODO Auto-generated constructor stub
	this->felix = NULL;
	this->marquesina = false;
	this->torta = false;
	this->ventanaRota = 0;
}

Ventana::Ventana(Felix * felix, bool torta, bool marquesina, int rota)
{
	this->felix = felix;
	this->torta = torta;
	this->marquesina = marquesina;
	this->ventanaRota = rota;
}

Ventana::Ventana(bool torta,bool marquesina,int rota)
{
	this->felix = NULL;
	this->torta = torta;
	this->marquesina = marquesina;
	this->ventanaRota = rota;
}

Ventana::Ventana(int rota)
{
	this->felix = NULL;
	this->marquesina = false;
	this->torta = false;
	this->ventanaRota = rota;
}


Ventana::~Ventana() {
	// TODO Auto-generated destructor stub
}

