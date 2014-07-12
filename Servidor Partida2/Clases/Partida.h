/*
 * Partida.h
 *
 *  Created on: Jul 8, 2014
 *      Author: ghiroma
 */

#ifndef PARTIDA_H_
#define PARTIDA_H_

#include "Felix.h"
#include "Edificio.h"
#include "Timer.h"
#include "CommunicationSocket.h"

class Partida {

public:
	Partida(int id);
	CommunicationSocket * cSocket1;
	CommunicationSocket * cSocket2;
	Felix * felix1;
	Felix * felix2;
	Edificio * edificio;
	Timer * timer;
	bool jugador1Listo;
	bool jugador2Listo;
	int nivel;
	int estado;
	int id;
	virtual ~Partida();
};

#endif /* PARTIDA_H_ */
