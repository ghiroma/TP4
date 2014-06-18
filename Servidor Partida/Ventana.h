/*
 * Ventana.h
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#ifndef VENTANA_H_
#define VENTANA_H_

class Ventana {

public:
	bool jugador1;
	bool jugador2;
	bool torta;
	bool marquesina;
	int ventanaRota;

	Ventana();
	Ventana(bool jugador1, bool jugador2, bool torta, bool marquesina,
			int rota);
	Ventana(bool torta, bool marquesina, int rota);
	Ventana(int rota);
	virtual ~Ventana();
};

#endif /* VENTANA_H_ */
