/*
 * Jugador.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "Jugador.h"

Jugador::Jugador() {
	this->Id = 0;
	this->Nombre = "";
	this->Puntaje = 0;
	this->SocketAsociado = 0;
}

Jugador::Jugador(int Id, string Nombre) {
	this->Id = Id;
	this->Nombre = Nombre;
	this->Puntaje = 0;
	this->SocketAsociado = 0;
}

int Jugador::sumarPuntaje(int Puntaje) {
	return this->Puntaje += Puntaje;
}

void Jugador::actualizarPartida(int idOponente, int ganado) {
	this->Partidas[idOponente] = ganado;
}

void Jugador::agregarJugador(int idOponente) {
	this->Partidas[idOponente] = 0;
}

void Jugador::quitarJugador(int idOponente) {
	this->Partidas.erase(idOponente);
}

Jugador::~Jugador() {
	// TODO Auto-generated destructor stub
}

