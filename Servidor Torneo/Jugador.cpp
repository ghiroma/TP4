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
	this->CantPartidasJugadas = 0;
}

Jugador::Jugador(int Id, string Nombre, CommunicationSocket* SocketAsociado) {
	this->Id = Id;
	this->Nombre = Nombre;
	this->Puntaje = 0;
	this->SocketAsociado = SocketAsociado;
	this->Jugando = false;
	this->CantPartidasJugadas = 0;
}

int Jugador::sumarPuntaje(int Puntaje) {
	return this->Puntaje += Puntaje;
}

void Jugador::actualizarPartidaCantEnfrentamientos(int idOponente) {
	this->Partidas[idOponente]++;
}

void Jugador::agregarOponente(int idOponente) {
	/////////////////////////////////
	//CODIGO DE PRUEBA
	if (idOponente == 1) {
		this->Partidas[idOponente] = 2;
	}else
	if (idOponente == 2) {
		this->Partidas[idOponente] = 2;
	} else
	if (idOponente == 3) {
		this->Partidas[idOponente] = 1;
	}else
	if (idOponente == 4) {
		this->Partidas[idOponente] = 1;
	}else{
		this->Partidas[idOponente] = 1;
	}
	/////////////////////////////////

	//this->Partidas[idOponente] = 0;

}

void Jugador::quitarJugador(int idOponente) {
	this->Partidas.erase(idOponente);
}

int Jugador::obtenerOponente() {
	int idOponenteEncontrado = -1;
	//si ya esta jugando no le busco un oponente
	if (this->Jugando) {
		return 0;
	}

	//busco un jugador que no haya jugado nunca contra este (que no este jugando)
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
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
			if (!listJugadores[p->first]->Jugando) {
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
				if (!listJugadores[p->first]->Jugando && p->second < minCantVecesEnfrentado) {
					//cout << "oponente encontrado: " << p->first << endl;
					idOponenteEncontrado = p->first;
					minCantVecesEnfrentado = p->second;
					break;
				}
				p++;
			}
		}
	}

	if (idOponenteEncontrado == -1) {
		//no hay oponentes posibles
		//cout << "no hay oponentes posibles" << endl;
		return -1;
	} else {
		//seteo que los dos jugadores que van a enfrentarse estan jugando y les actualizo la cantidad enfrentamientos
		this->Jugando = true;
		listJugadores[idOponenteEncontrado]->Jugando = true;
		this->actualizarPartidaCantEnfrentamientos(idOponenteEncontrado);
		listJugadores[idOponenteEncontrado]->actualizarPartidaCantEnfrentamientos(this->Id);

		//devuelvo el id del jugador encontrado para enfrentar en una partida
		return idOponenteEncontrado;
	}

}

Jugador::~Jugador() {
	// TODO Auto-generated destructor stub
}
