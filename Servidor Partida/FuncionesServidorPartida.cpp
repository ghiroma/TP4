/*
 * Funciones.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "FuncionesServidorPartida.h"
#include <iostream>
#include "Support/Constantes.h"
#include <string.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include "Clases/Edificio.h"
#include "Support/Estructuras.h"
#include <cstdio>

using namespace std;

bool stop = false;
bool cliente1_conectado = true;
bool cliente2_conectado = true;
bool cliente1_jugando = true;
bool cliente2_jugando = true;

queue<string> receiver1_queue;
queue<string> receiver2_queue;
queue<string> sender_queue;

CommunicationSocket * cSocket1;
CommunicationSocket * cSocket2;

Felix felix1;
Felix felix2;

Edificio * edificio;

bool TimeDifference(int timeDifference, time_t startingTime) {
	if ((time(0) - startingTime) > timeDifference) {
		return true;
	} else {
		return false;
	}
}
void*
timer_thread(void* arg) {
	time_t startingTimeKeepAlive = time(0);
	time_t startingTimeRalph = time(0);
	time_t startingTimePaloma = time(0);
	time_t startingTimePersiana = time(0);
	time_t startingTimeTorta = time(0);

	//TODO sacar hardcodeo.
	while (stop == false) {

		if (TimeDifference(INTERVALOS_KEEPALIVE, startingTimeKeepAlive)
				== true) {
			string message(CD_ACK);
			sender_queue.push(message);
			startingTimeKeepAlive = time(0);
		}

		if (TimeDifference(INTERVALOS_RALPH, startingTimeRalph) == true) {
			char aux[5];
			string message(CD_MOVIMIENTO_RALPH);
			sprintf(aux,"%d",randomRalphMovement());
			message.append(aux);
			sender_queue.push(message);
			startingTimeRalph = time(0);
		}

		if (TimeDifference(INTERVALOS_PALOMA, startingTimePaloma) == true) {
			string message(CD_PALOMA);
			char aux[5];
			sprintf(aux,"%d",randomPaloma(0));
			message.append(aux);
			sender_queue.push(message);
			startingTimePaloma = time(0);
		}

		if (TimeDifference(INTERVALOS_TORTA, startingTimeTorta) == true) {
			string message(CD_TORTA);
			message.append(randomTorta());
			sender_queue.push(message);
			startingTimeTorta = time(0);
		}

		if (TimeDifference(INTERVALOS_PERSIANA, startingTimePersiana) == true) {
			struct mensaje message;

			//strcpy(message.codigo_mensaje,CD_PERSIANA);
			//strcpy(message.contenido,randomPersiana);
			//sender_queue.push(message);
			startingTimePersiana = time(0);
		}

		usleep(8000);
	}

	pthread_exit(0);
}

void*
sender_thread(void * arguments) {
	while (stop == false && (cliente1_conectado || cliente2_conectado)) {
		if (!sender_queue.empty()) {
			//Lo que venga del timer y validator, se replica a ambos jugadores.
			string message = sender_queue.front();
			sender_queue.pop();
			cout << "Mensaje a enviar: " << message.c_str() << endl;
			if(cliente1_conectado)
			{
				cSocket1->SendBloq(message.c_str(), message.length());
			}
			if(cliente2_conectado)
			{
			//cSocket2->SendBloq(message.c_str(),message.length());
			}
		}

		usleep(8000);
	}

	pthread_exit(0);
}

void*
receiver1_thread(void * fd) {
	char buffer[512];
	int readDataCode;
	bzero(buffer, sizeof(buffer));

	while (stop == false && cliente1_conectado) {
		readDataCode = cSocket1->ReceiveNoBloq(buffer, sizeof(buffer));
		if (readDataCode > 0) {
			string aux(buffer);
			receiver1_queue.push(aux);
		} else if (readDataCode == 0) {
			cliente1_conectado = false;
		}
		usleep(8000);
	}

	pthread_exit(0);
}

void*
receiver2_thread(void * fd) {
	char buffer[512];
	int readDataCode;
	bzero(buffer, sizeof(buffer));

	while (stop == false && cliente2_conectado) {
		readDataCode = cSocket2->ReceiveNoBloq(buffer, sizeof(buffer));
		if (readDataCode > 0) {
			string mensaje(buffer);
			receiver2_queue.push(mensaje);
		} else if (readDataCode == 0) {
			cliente2_conectado = false;
		}
		usleep(8000);
	}

	pthread_exit(0);
}

//TODO el validator tambien se encarga de cerrar los socket una vez que el cliente se desconecto?
void * validator_thread(void * argument) {

	while (stop == false) {
		if (!receiver1_queue.empty()) {
			string message = receiver1_queue.front();
			receiver1_queue.pop();

			//Dependiendo del codigo, voy a ver que valido.
			//switch () {
			//Escribe en el sender_queue.
			//}
		}
		if (!receiver2_queue.empty()) {
			string message = receiver2_queue.front();
			receiver2_queue.pop();

			//switch()
			//{

			//}
		}

		//Murieron los dos jugadores.
		if(cliente1_jugando == false && cliente2_jugando==false)
		{
			stop = true;
		}
	}

	pthread_exit(0);
}

void * keepAliveThread(void * argument) {
	char buffer[10];

	while (stop == false && (cliente1_conectado || cliente2_conectado)) {

		if (cliente1_conectado) {
			cSocket1->SendBloq(CD_ACK, sizeof(char) * strlen(CD_ACK));
			if (strlen(buffer) == 0) {
				cliente1_conectado = false;
			}
		}

		if (cliente2_conectado) {
			cSocket2->SendBloq(CD_ACK, sizeof(char) * strlen(CD_ACK));
			cSocket2->ReceiveBloq(buffer, sizeof(buffer));
			if (strlen(buffer)) {
				cliente2_conectado = false;
			}
		}

		usleep(10000);
	}

	pthread_exit(0);
}

int randomRalphMovement()
{
	return rand()%(EDIFICIO_COLUMNAS+1);
}

int randomPaloma(int nivel)
{
	if(nivel==0)
		return rand()%(EDIFICIO_FILAS_1+1);
	else if(nivel==1)
		return rand()%(EDIFICIO_FILAS_2+1);
	return 0;
}

char* randomTorta()
{
	char location[3];
	char aux [2];

	sprintf(aux,"%d",rand()%(EDIFICIO_FILAS_1+1));
	strcpy(location,aux);
	sprintf(aux,"%d",rand()%(EDIFICIO_COLUMNAS+1));
	strcat(location,aux);

	return location;
}

bool validateMovement(Felix * felix, int fila, int columna, Edificio * edificio) {
	if (((fila < edificio->filas || fila >= 0)
			&& (columna < edificio->columnas || columna >= 0))
			&& !edificio->ventanas[fila][columna].marquesina
			&& edificio->ventanas[fila][columna].felix==NULL) {

		edificio->ventanas[felix->posicion_x][felix->posicion_y].felix = NULL;
		edificio->ventanas[fila][columna].felix = felix;
		felix->posicion_x = fila;
		felix->posicion_y = fila;

		return true;
	}
	return false;
}

bool validateWindowFix(Felix * felix, int fila, int columna, Edificio * edificio) {
	if (edificio->ventanas[fila][columna].ventanaRota > 0) {
		felix->puntaje_parcial++;
		edificio->ventanas[fila][columna].ventanaRota--;
		return true;
	}
	return false;
}

bool validateLives(Felix * felix)
{
	return --felix->cantidad_vidas == 0;
}

void SIGINT_Handler(int inum) {
	stop = true;
}
