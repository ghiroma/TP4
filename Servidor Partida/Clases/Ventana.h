/*
 * Ventana.h
 *
 *  Created on: Jun 17, 2014
 *      Author: ghiroma
 */

#ifndef VENTANA_H_
#define VENTANA_H_

/*
 * Representa las ventanas del edificio.
 * felix: Puntero a objeto de clase Felix.
 * torta: Booleano si hay una torta en la ventana.
 * marquesina: Booleano si hay una marquesina en la ventana.
 * persiana: Booleano si la persiana esta cerrada.
 * ventanaRota: Int, que tan roto esta la ventana. Valores posibles: 0,1,2
 */

class Ventana {

public:
	char value;
	bool ocupado;
	bool torta;
	bool marquesina;
	bool persiana;
	int ventanaRota;

	Ventana();
	Ventana(bool ocupado, bool torta, bool marquesina,
			int rota);
	Ventana(bool torta, bool marquesina, int rota);
	Ventana(int rota);
	virtual ~Ventana();
};

#endif /* VENTANA_H_ */
