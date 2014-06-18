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
	this->jugando = false;
}

Jugador::Jugador(int Id, string Nombre) {
	this->Id = Id;
	this->Nombre = Nombre;
	this->Puntaje = 0;
	this->SocketAsociado = 0;
	this->jugando = false;
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

int Jugador::obtenerOponente(list<Jugador>* listJugadores) {
	bool oponenteEncontrado = false;
	int idOponenteEncontrado = 0;
	//si ya esta jugando no le busco un oponente
	if (this->jugando) {
		return 0;
	}

	//busco un jugador que no haya jugado nunca contra este (que no este jugando)
	for (list<Jugador>::iterator it = (*listJugadores).begin(); it != (*listJugadores).end(); it++) {
		if ((*it).Id != this->Id && !(*it).jugando) {
			if (this->Partidas[(*it).Id] == 0) {
				cout << "oponente encontrado: " << (*it).Id << endl;
				idOponenteEncontrado = (*it).Id;
				break;
			}
		}
	}

	//si no encuentro ninguno
	int idJugadorMenosVecesEnfrentado;
	int minCantVecesEnfrentado;
	map<int, int>::iterator p = this->Partidas.begin();
	if (p != this->Partidas.end() && ) {
		idJugadorMenosVecesEnfrentado = p->first;
		minCantVecesEnfrentado = p->second;
	}
	while (p != this->Partidas.end()) {

		p++;
	}

	if (idOponenteEncontrado == 0) {
		//contabilizo los enfrentamientos y veo con quien se enfrento menos veces (que no este jugando)
		for (list<Jugador>::iterator it = (*listJugadores).begin(); it != (*listJugadores).end(); it++) {
			if ((*it).Id != this->Id && !(*it).jugando) {
				if ((*it).Partidas[]) {

				}
			}
		}

	}

	//si no encuentro ninguno
	if (idOponenteEncontrado == 0) {
		//asigno al primer jugador que encuentre (que no este jugando)
		for (list<Jugador>::iterator it = (*listJugadores).begin(); it != (*listJugadores).end(); it++) {
			if ((*it).Id != this->Id && !(*it).jugando) {

			}
		}

	}

	//seteo que los dos jugadoros que van a enfrentarse estan jugando

	//devuelvo los id

	return 1;
}

Jugador::~Jugador() {
	// TODO Auto-generated destructor stub
}

