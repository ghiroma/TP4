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
	this->Jugando = false;
}

Jugador::Jugador(int Id, string Nombre) {
	this->Id = Id;
	this->Nombre = Nombre;
	this->Puntaje = 0;
	this->SocketAsociado = 0;
	this->Jugando = false;
}

int Jugador::sumarPuntaje(int Puntaje) {
	return this->Puntaje += Puntaje;
}

void Jugador::actualizarPartida(int idOponente, int ganado) {
	this->Partidas[idOponente] = ganado;
}

void Jugador::agregarJugador(int idOponente) {
	if (idOponente == 1) {
		this->Partidas[idOponente] = 1;
	} else {
		this->Partidas[idOponente] = 2;
	}
	//this->Partidas[idOponente] = 0;

}

void Jugador::quitarJugador(int idOponente) {
	this->Partidas.erase(idOponente);
}

int Jugador::obtenerOponente(map<int, Jugador*>* listJugadores) {
	bool oponenteEncontrado = false;
	int idOponenteEncontrado = -1;
	//si ya esta jugando no le busco un oponente
	if (this->Jugando) {
		return 0;
	}

	//busco un jugador que no haya jugado nunca contra este (que no este jugando)
	for (map<int, Jugador*>::iterator it = (*listJugadores).begin(); it != (*listJugadores).end(); it++) {
		if ((*(*it).second).Id != this->Id && !(*(*it).second).Jugando) {
			if (this->Partidas[(*(*it).second).Id] == 0) {
				//cout << "oponente encontrado: " << (*(*it).second).Id << endl;
				idOponenteEncontrado = (*(*it).second).Id;
				break;
			}
		}
	}

	if (idOponenteEncontrado == -1) {
		//si no encuentro ninguno
		int minCantVecesEnfrentado = -1;
		map<int, int>::iterator p = this->Partidas.begin();
		//busco alguno que no este jugando para tomarlo como referencia(inicializar) de minCantVecesEnfrentado
		while (p != this->Partidas.end()) {
			if (!(*listJugadores)[p->first]->Jugando) {
				idOponenteEncontrado = p->first;
				minCantVecesEnfrentado = p->second;
				break;
			}
			p++;
		}
		//si encontro alguno que no este jugando
		if (minCantVecesEnfrentado != -1) {
			//busco en todo el listado el que menos veces jugo
			p = this->Partidas.begin();
			while (p != this->Partidas.end()) {
				if (!(*listJugadores)[p->first]->Jugando && p->second < minCantVecesEnfrentado) {
					//cout << "oponente encontrado: " << p->first << endl;
					idOponenteEncontrado = p->first;
					minCantVecesEnfrentado = p->second;
					break;
				}
				p++;
			}
		}
	}

	//no hay oponentes posibles
	if (idOponenteEncontrado == -1) {
		cout << "no hay oponentes posibles" << endl;
		return -1;
	} else {
		//seteo que los dos jugadores que van a enfrentarse estan jugando
		this->Jugando = true;
		(*listJugadores)[idOponenteEncontrado]->Jugando = true;

		//devuelvo el o los id
		return idOponenteEncontrado;
	}

}

Jugador::~Jugador() {
	// TODO Auto-generated destructor stub
}
