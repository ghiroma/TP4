/*
 * Ventana.h
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#ifndef VENTANA_H_
#define VENTANA_H_

#include "Felix.h"

class Ventana {

public:
	Felix * felix;
	bool torta;
	bool marquesina;
	bool persiana;
	int ventanaRota;

	Ventana();
	Ventana(Felix * felix, bool torta, bool marquesina,
			int rota);
	Ventana(bool torta, bool marquesina, int rota);
	Ventana(int rota);
	virtual ~Ventana();
};

#endif /* VENTANA_H_ */
