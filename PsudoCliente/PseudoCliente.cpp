/*
 * PseudoCliente.cpp
 *
 *  Created on: Jul 8, 2014
 *      Author: ghiroma
 */

#include <iostream>
#include "Clases/CommunicationSocket.h"
#include <string.h>
#include <unistd.h>
using namespace std;

int main(int argc, char * argv) {
	char buffer[7];
	CommunicationSocket cSocket(5555, "127.0.0.1");
	cSocket.SendBloq("0000001", 7); //Mando id de partida.
	cout << "Id Partida enviado" << endl;
	cSocket.SendBloq("0000001", 7);
	cout << "Id Jugador enviado" << endl;
	cSocket.ReceiveBloq(buffer, sizeof(buffer));
	cout << "Recibi mi posicion inicial: " << buffer << endl;
	cSocket.SendBloq("6100000", sizeof(buffer));
	cout << "Enviado mensaje de listo" << endl;
	do {
		cSocket.ReceiveBloq(buffer, sizeof(buffer));
		sleep(1);
	} while (strcmp(buffer, "6200000") != 0);
	cout << "Recibi mensaje de iniciar partida: " << buffer << endl;
}

