/*
 * Estructuras.h
 *
 *  Created on: Jul 20, 2014
 *      Author: ghiroma
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_


struct ventana {
	short int x;
	short int y;
	short int fila;
	short int columna;
	short int numero;
	short int sana;
	char tipo_ventana;
	char ocupado;
};

struct posicion {
	unsigned short int fila;
	unsigned short int columna;
};

struct desplazamiento {
	short int x;
	short int y;
};



#endif /* ESTRUCTURAS_H_ */
