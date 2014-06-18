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
#include <unistd.h>
#include "Edificio.h"

using namespace std;

bool stop = false;

queue<string> receiver1_queue;
queue<string> receiver2_queue;
queue<string> sender_queue;

CommunicationSocket * cSocket1;
CommunicationSocket * cSocket2;

bool TimeDifference(int timeDifference, time_t startingTime) {
	if ((time(0) - startingTime) > timeDifference) {
		return true;
	} else {
		return false;
	}
}
void*
timer_thread(void* arg) {
	time_t startingTimeRalph = time(0);
	time_t startingTimePaloma = time(0);
	time_t startingTimePersiana = time(0);
	time_t startingTimeTorta = time(0);

	//TODO sacar hardcodeo.
	while (stop == false) {
		if (TimeDifference(5, startingTimeRalph) == true) {
			string message(CD_MOVIMIENTO_RALPH);
			//Agregarle hacia donde se va a mover ralph
			sender_queue.push(message);
			startingTimeRalph = time(0);
		}

		if (TimeDifference(20, startingTimePaloma) == true) {
			string message(CD_PALOMA);
			//Agregarle donde aparece
			sender_queue.push(message);
			startingTimePaloma = time(0);
		}

		if (TimeDifference(35, startingTimeTorta) == true) {
			string message(CD_TORTA);
			//Agregarle donde aparece.
			sender_queue.push(message);
			startingTimeTorta = time(0);
		}

		if (TimeDifference(60, startingTimePersiana) == true) {
			string message(CD_PERSIANA);
			//Agregarle donde aparece
			sender_queue.push(message);
			startingTimePersiana = time(0);
		}

		usleep(8000);
	}

	pthread_exit(0);
}

void*
sender_thread(void * arguments) {
	while (stop == false) {
		if (!sender_queue.empty()) {
			//Lo que venga del timer y validator, se replica a ambos jugadores.
			string message = sender_queue.front();
			sender_queue.pop();
			cout << "Mensaje a enviar: " << message.c_str() << endl;
			cSocket1->SendBloq(message.c_str(), message.length());
			//cSocket2->SendBloq(message.c_str(),message.length());
		}

		usleep(8000);
	}

	pthread_exit(0);
}

void*
receiver1_thread(void * fd) {
	char buffer[512];
	bzero(buffer, sizeof(buffer));

	while (stop == false) {
		cSocket1->ReceiveNoBloq(buffer, sizeof(buffer));
		if (strlen(buffer) > 0) {
			string aux(buffer);
			receiver1_queue.push(aux);
		}
		usleep(8000);
	}

	pthread_exit(0);
}

void*
receiver2_thread(void * fd) {
	char buffer[512];
	bzero(buffer, sizeof(buffer));

	while (stop == false) {
		cSocket2->ReceiveNoBloq(buffer, sizeof(buffer));
		if (strlen(buffer) > 0) {
			string aux(buffer);
			receiver2_queue.push(aux);
		}
		//TODO realizar accion.
		usleep(8000);
		//sleep(1);
	}

	pthread_exit(0);
}

void * validator_thread(void * argument) {
	Edificio edificio(3, 3, 0);

	//Tomar de cola, datos para validar.
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
	}
	pthread_exit(0);
}

void * keepAliveThread(void * argument) {
	//Enviar mensajes cada cierto intervalo para saber si esta vivo
	//Ver quien es necesario que haga esto.
}

bool validateMovement(int fila, int columna, Edificio * edificio) {
	if (((fila < edificio->filas || fila >= 0)
			&& (columna < edificio->columnas || columna >= 0))
			&& !edificio->ventanas[fila][columna].marquesina) {
		//TODO Poner en cero la posicion anterior donde estaba. Puedo
		//poner en cero el booleando indicador del jugador, de todas las
		//posiciones adyacentes a la ventana que se movio.

		//TODO Ver que jugador es el que se movio. Puedo asumirlo
		//segun la cola de donde saque el mensaje.
		//edificio->ventanas[fila][columna].jugador1 = true;
		//edificio->ventanas[fila][columna].jugador2 = true;

		return true;
	}
	return false;
}

bool validateWindowFix(int fila, int columna, Edificio * edificio) {
	if (edificio->ventanas[fila][columna].ventanaRota > 0) {
		edificio->ventanas[fila][columna].ventanaRota--;
		return true;
	}
	return false;
}

void SIGINT_Handler(int inum) {
	stop = true;
}
