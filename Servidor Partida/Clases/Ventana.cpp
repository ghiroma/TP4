/*
 * Ventana.cpp
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#include "Ventana.h"

Ventana::Ventana() {
	// TODO Auto-generated constructor stub
	this->ocupado = false;
	this->marquesina = false;
	this->torta = false;
	this->persiana = false;
	this->ventanaRota = 1;
}

Ventana::Ventana(bool  ocupado, bool torta, bool marquesina, int rota)
{
	this->ocupado = false;
	this->torta = torta;
	this->marquesina = marquesina;
	this->persiana = false;
	this->ventanaRota = rota;
}

Ventana::Ventana(bool torta,bool marquesina,int rota)
{
	this->ocupado = false;
	this->torta = torta;
	this->marquesina = marquesina;
	this->persiana = false;
	this->ventanaRota = rota;
}

Ventana::Ventana(int rota)
{
	this->ocupado = false;
	this->marquesina = false;
	this->torta = false;
	this->persiana = false;
	this->ventanaRota = rota;
}


Ventana::~Ventana() {
	// TODO Auto-generated destructor stub
}

