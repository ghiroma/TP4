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
	if(idOponente==1){
		this->Partidas[idOponente] = 1;
	}else{
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
				cout << "oponente encontrado: " << (*(*it).second).Id << endl;
				idOponenteEncontrado = (*(*it).second).Id;
				break;
			}
		}
	}

	//si no encuentro ninguno
	int idJugadorMenosVecesEnfrentado;
	int minCantVecesEnfrentado = -1;
	map<int, int>::iterator p = this->Partidas.begin();
	//busco alguno que no este jugando para tomarlo como referencia(inicializar) de minCantVecesEnfrentado
	while(p != this->Partidas.end()) {
		if(!this->Jugando){
			idJugadorMenosVecesEnfrentado = p->first;
			minCantVecesEnfrentado = p->second;
			break;
		}
		p++;
	}
	//si encontro alguno que no este jugando
	if(minCantVecesEnfrentado != -1){
		//busco en todo el listado el que menos veces jugo
		p = this->Partidas.begin();
		while (p != this->Partidas.end()) {
			//////
			////// cambiara a MAP sino no puedo acceder al jugador para ver si esta jungando

			if(!(*(*listJugadores)[p->first]).Jugando && p->second < minCantVecesEnfrentado){
				cout << "oponente encontrado: " << p->first << endl;
				idJugadorMenosVecesEnfrentado = p->first;
				minCantVecesEnfrentado = p->second;
				break;
			}


			p++;
		}
	}

	/*
	if (idOponenteEncontrado == -1) {
		//contabilizo los enfrentamientos y veo con quien se enfrento menos veces (que no este jugando)
		for (list<Jugador>::iterator it = (*listJugadores).begin(); it != (*listJugadores).end(); it++) {
			if ((*it).Id != this->Id && !(*it).jugando) {

				if ((*it).Partidas[]) {

				}

			}
		}
	}*/

	//si no encuentro ninguno
	/*if (idOponenteEncontrado == -1) {
		//asigno al primer jugador que encuentre (que no este jugando)
		for (map<int,Jugador>::iterator it = (*listJugadores).begin(); it != (*listJugadores).end(); it++) {
			if ((*it).Id != this->Id && !(*it).Jugando) {
				cout << "oponente encontrado: " << (*it).Id << endl;
				idOponenteEncontrado = (*it).Id;
				break;
			}
		}
	}*/

	//no hay oponentes posibles
	if (idOponenteEncontrado == -1){
		cout<<"no hay oponentes posibles"<<endl;
		return 0;
	}else{
		//seteo que los dos jugadores que van a enfrentarse estan jugando

		//devuelvo los id
		return 1;
	}

}

Jugador::~Jugador() {
	// TODO Auto-generated destructor stub
}
