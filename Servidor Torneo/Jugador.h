/*
 * Jugador.h
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#ifndef JUGADOR_H_
#define JUGADOR_H_

#include <string>
#include <map>

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
	int Puntaje;
	int SocketAsociado;
	string Nombre;
	int Id;
	map<int, int> Partidas; //Agregar a constants. 0 No jugado, -1 Perdido, 1 Ganado.

	Jugador();
	Jugador(int Id, string Nombre);
	int sumarPuntaje(int puntaje);
	void actualizarPartida(int idOponente, int ganado);
	void agregarJugador(int idOponente);
	void quitarJugador(int idOponente);
	//int enviarMensaje();
	//int recibirMensaje();

	virtual
	~Jugador();
};

#endif /* JUGADOR_H_ */
