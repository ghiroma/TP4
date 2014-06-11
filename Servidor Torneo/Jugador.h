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
class Jugador
{
public:
  int Puntaje;
  int SocketAsociado;
  string Nombre;
  int Id;
  map<int,int> Partidas; //Agregar a constants. 0 No jugado, -1 Perdido, 1 Ganado.

  Jugador ();
  Jugador(int Id,string Nombre);
  int SumarPuntaje(int puntaje);
  void ActualizarPartida(int idOponente,int ganado);
  void AgregarJugador(int idOponente);
  void QuitarJugador(int idOponente);
  //int EnviarMensaje();
  //int RecibirMensaje();

  virtual
  ~Jugador ();
};

#endif /* JUGADOR_H_ */
