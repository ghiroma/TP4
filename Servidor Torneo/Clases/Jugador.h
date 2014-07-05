/*
 * Jugador.h
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#ifndef JUGADOR_H_
#define JUGADOR_H_

#include "CommunicationSocket.h"
#include <string>
#include <map>
#include <iostream>
#include <list>

using namespace std;


/*
 * Class Jugador:
 *  Puntaje representa el puntaje total.
 *  SocketAsociado se utilizara para la comunicacion con el jugador, esta se ira actualizando, ya que no estara en dos partidas a la vez.
 *  Nombre representa el nombre del jugador.
 *  Id representa un identificador unico del jugador.
 *  Partidas indica, el id del jugador y verdadero o falso segun si ya jugo.
 */

class Jugador {
public:
	int Id;
	string Nombre;
	bool Jugando; //true si el jugador esta jugando en una partida
	int Ranking;
	int PartidasGanadas;
	int PartidasPerdidas;
	int CantPartidasJugadas;
	int Puntaje;
	float Promedio;
	CommunicationSocket * SocketAsociado;
	map<int, int> Partidas; //<ID Jugador, cantEnfrentamientos>

	Jugador();
	Jugador(int Id, string Nombre, CommunicationSocket * SocketAsociado);
	int sumarPuntaje(int puntaje);
	void actualizarPartidaCantEnfrentamientos(int idOponente);
	void agregarOponente(int idOponente);
	void quitarJugador(int idOponente);
	int obtenerOponente();
	//int enviarMensaje();
	//int recibirMensaje();

	virtual
	~Jugador();
};

extern map<int, Jugador*> listJugadores;


#endif /* JUGADOR_H_ */
