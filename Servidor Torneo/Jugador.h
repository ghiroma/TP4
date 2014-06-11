/*
 * Jugador.h
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#ifndef JUGADOR_H_
#define JUGADOR_H_

#include <string>

using namespace std;

class Jugador
{
public:
  int Puntaje;
  int SocketAsociado;
  string Nombre;
  int Id;

  Jugador ();
  Jugador(int Id,string Nombre);
  int SumarPuntaje(int puntaje);
  virtual
  ~Jugador ();
};

#endif /* JUGADOR_H_ */
