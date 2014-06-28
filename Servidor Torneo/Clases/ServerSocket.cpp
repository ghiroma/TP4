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

#include <string.h>
#include <iostream>

#define MAX_PENDING_CONNECTIONS 20
using namespace std;

ServerSocket::ServerSocket(unsigned int port, char * ip) {
	this->ID = socket(AF_INET, SOCK_STREAM, 0);
	if (this->ID >= 0) {
		struct sockaddr_in laddress;
		laddress.sin_addr.s_addr = inet_addr(ip);
		laddress.sin_port = htons(port);
		laddress.sin_family = AF_INET;
		if (bind(this->ID, (struct sockaddr*) &laddress, sizeof(sockaddr)) == 0) {
			if ((listen(this->ID, MAX_PENDING_CONNECTIONS)) != 0) {
				throw "Error en bind";
			}
		} else {
			throw "Error en bind";
		}
	} else {
		throw "Error en socket";
	}
}

ServerSocket::ServerSocket(unsigned int port) {
	this->ID = socket(AF_INET, SOCK_STREAM, 0);
	if (this->ID >= 0) {
		struct sockaddr_in laddress;
		laddress.sin_addr.s_addr = INADDR_ANY;
		laddress.sin_port = htons(port);
		laddress.sin_family = AF_INET;
		if (bind(this->ID, (struct sockaddr*) &laddress, sizeof(sockaddr)) == 0) {
			if ((listen(this->ID, MAX_PENDING_CONNECTIONS)) != 0) {
				throw "Error en listen";
			}
		} else {
			throw "Error en bind";
		}
	} else {
		throw "Error en socket";
	}
}

CommunicationSocket* ServerSocket::Accept() {
	struct sockaddr_in clientData;
	unsigned int clientDataLen = sizeof(clientData);
	int csSocket = accept(this->ID, (struct sockaddr*) &clientData, &clientDataLen);
	return new CommunicationSocket(csSocket, &clientData);
}

ServerSocket::~ServerSocket() {
	// TODO Auto-generated destructor stub

	cout<<"---------------------EJECUTO EL DESTRUCTOR DE ServerSocket "<<endl;
	close(this->ID);
}
