/*
 * Felix.h
 *
 *  Created on: Jun 19, 2014
 *      Author: ghiroma
 */

#ifndef FELIX_H_
#define FELIX_H_

using namespace std;

#include <string>

class Felix {
public:
	string nombre;
	short int posicion_x;
	short int posicion_y;
	unsigned short int cantidad_vidas;
	unsigned int puntaje_parcial;

	Felix();
	Felix(string nombre);
	Felix(string nombre, int x, int y);
	virtual ~Felix();
};

#endif /* FELIX_H_ */
