/*
 * Socket.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "ServerSocket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <iostream>
#include <errno.h>

#define MAX_PENDING_CONNECTIONS 20

ServerSocket::ServerSocket(unsigned int port, bool blocking) {
	if (blocking)
		this->ID = socket(AF_INET, SOCK_STREAM, 0);
	else
		this->ID = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

	int iSetOption = 1;
	setsockopt(this->ID, SOL_SOCKET, SO_REUSEADDR, (char*) &iSetOption, sizeof(iSetOption));

	struct linger linger = {0};
	linger.l_onoff=1;
	linger.l_linger = 0;
	setsockopt(this->ID,SOL_SOCKET,SO_LINGER,(char *)&linger,sizeof(linger));

	if (this->ID >= 0) {
		struct sockaddr_in laddress;
		laddress.sin_addr.s_addr = INADDR_ANY;
		laddress.sin_port = htons(port);
		laddress.sin_family = AF_INET;
		if (bind(this->ID, (struct sockaddr*) &laddress, sizeof(sockaddr)) == 0) {
			if ((listen(this->ID, MAX_PENDING_CONNECTIONS)) != 0) //Error listen.
					{
				close(this->ID);
				throw "Error en listen";
			}
		} else //Error en bind.
		{
			close(this->ID);
			throw "Error en bind";
		}
	} else //Error en socket
	{
		throw "Error en socket";
	}
}

ServerSocket::ServerSocket(unsigned int port) {
	this->ID = socket(AF_INET, SOCK_STREAM, 0);

	int iSetOption = 1;
	setsockopt(this->ID, SOL_SOCKET, SO_REUSEADDR, (char*) &iSetOption, sizeof(iSetOption));

	if (this->ID >= 0) {
		struct sockaddr_in laddress;
		laddress.sin_addr.s_addr = INADDR_ANY;
		laddress.sin_port = htons(port);
		laddress.sin_family = AF_INET;
		if (bind(this->ID, (struct sockaddr*) &laddress, sizeof(sockaddr)) == 0) {
			if ((listen(this->ID, MAX_PENDING_CONNECTIONS)) != 0) //Error listen.
					{
				close(this->ID);
				throw "Error en listen";
			}
		} else //Error en bind.
		{
			close(this->ID);
			throw "Error en bind";
		}
	} else //Error en socket
	{
		throw "Error en socket";
	}
}

CommunicationSocket* ServerSocket::Accept() {
	struct sockaddr_in clientData;
	unsigned int clientDataLen = sizeof(clientData);
	int csSocket = accept(this->ID, (struct sockaddr*) &clientData, &clientDataLen);
	if (csSocket <= 0) {
		if (errno == EAGAIN) {
			return NULL;
		} else {
			throw "Error en accept";
		}
	}
	return new CommunicationSocket(csSocket, &clientData);
}

char * ServerSocket::ShowHostName() {
	char aux[1024];
	if (int result = gethostname(aux, sizeof(aux)) == 0)
		return aux;
	else
		throw "Error al obtener el hostname.";
}

ServerSocket::~ServerSocket() {
	// TODO Auto-generated destructor stub
	close(this->ID);
	std::cout << "Destructor ssocket ejecutado" << std::endl;
}
