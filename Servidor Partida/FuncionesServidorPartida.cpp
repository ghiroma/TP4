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
#include "Support/Helper.h"
#include "Support/Estructuras.h"
#include <cstdio>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>

using namespace std;

bool stop = false;
bool cliente1_conectado = true;
bool cliente2_conectado = true;
bool cliente1_jugando = true;
bool cliente2_jugando = true;

queue<string> receiver1_queue;
queue<string> receiver2_queue;
queue<string> sender1_queue;
queue<string> sender2_queue;
queue<struct puntajes> puntajes_queue;

CommunicationSocket * cSocket1;
CommunicationSocket * cSocket2;

Felix *felix1;
//Felix *felix2;

Edificio *edificio;

pthread_mutex_t mutex_receiver1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_receiver2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_puntajes = PTHREAD_MUTEX_INITIALIZER;

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
			string content;
			message.append(Helper::fillMessage(content));
			//Helper::encolar(&message,&sender1_queue,&mutex_sender1);
			//Helper::encolar(&message,&sender2_queue,&mutex_sender2);
			startingTimeKeepAlive = time(0);
		}

		if (TimeDifference(INTERVALOS_RALPH, startingTimeRalph) == true) {
			char aux[5];
			string message(CD_MOVIMIENTO_RALPH);
			sprintf(aux, "%d", randomRalphMovement());
			message.append(Helper::fillMessage(aux));
			Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			//Helper::encolar(&message, &sender2_queue, &mutex_sender2);
			startingTimeRalph = time(0);
		}

		if (TimeDifference(INTERVALOS_PALOMA, startingTimePaloma) == true) {
			string message(CD_PALOMA);
			char aux[5];
			sprintf(aux, "%d", randomPaloma(0));
			message.append(Helper::fillMessage(aux));
			Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			//Helper::encolar(&message,&sender2_queue,&mutex_sender2);
			startingTimePaloma = time(0);
		}

		if (TimeDifference(INTERVALOS_TORTA, startingTimeTorta) == true) {
			string message(CD_TORTA);
			message.append(randomTorta());
			Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			//Helper::encolar(&message, &sender2_queue, &mutex_sender2);
			startingTimeTorta = time(0);
		}

		if (TimeDifference(INTERVALOS_PERSIANA, startingTimePersiana) == true) {
			//strcpy(message.codigo_mensaje,CD_PERSIANA);
			//strcpy(message.contenido,randomPersiana);
			//Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			//Helper::encolar(&message, &sender2_queue, &mutex_sender2);
			startingTimePersiana = time(0);
		}

		usleep(POLLING_DEADTIME);
		//sleep(1);
	}

	pthread_exit(0);
}

void*
sender1_thread(void * arguments) {
	while (stop == false && cliente1_conectado) {
		if (!sender1_queue.empty()) {
			//Lo que venga del timer y validator, se replica a ambos jugadores.
			string message;
			Helper::desencolar(&message, &sender1_queue, &mutex_sender1);
			cout << "Mensaje a enviar: " << message.c_str() << endl;
			if (cliente1_conectado) {
				cSocket1->SendBloq(message.c_str(), message.length());
			}
		}

		usleep(POLLING_DEADTIME);
		//sleep(1);
	}

	pthread_exit(0);
}

void*
sender2_thread(void * arguments) {
	while (stop == false && cliente2_conectado) {
		if (!sender2_queue.empty()) {
			//Lo que venga del timer y validator, se replica a ambos jugadores.
			string message;
			Helper::desencolar(&message, &sender2_queue, &mutex_sender2);
			cout << "Mensaje a enviar: " << message.c_str() << endl;
			if (cliente2_conectado) {
				cSocket2->SendBloq(message.c_str(), message.length());
			}
		}
		usleep(POLLING_DEADTIME);
		//sleep(1);
	}

	pthread_exit(0);
}

void*
receiver1_thread(void * fd) {
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	int readDataCode;
	bzero(buffer, sizeof(buffer));

	while (stop == false && cliente1_conectado) {
		readDataCode = cSocket1->ReceiveNoBloq(buffer, sizeof(buffer));
		//readDataCode = cSocket1->ReceiveNoBloq(buffer, sizeof(256));
		if (readDataCode > 0) {
			string aux(buffer);
			cout << "Recibi mensaje: " << buffer << endl;
			Helper::encolar(&aux, &receiver1_queue, &mutex_receiver1);
		} else if (readDataCode == 0) {
			cout << "Se desconecto el cliente nro 1" << endl;
			//TODO decirle al jugador nro2 que el cliente 1 se desconecto.
			cliente1_conectado = false;
		}
		usleep(POLLING_DEADTIME);
	}

	pthread_exit(0);
}

void*
receiver2_thread(void * fd) {
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	int readDataCode;
	bzero(buffer, sizeof(buffer));

	while (stop == false && cliente2_conectado) {
		readDataCode = cSocket2->ReceiveNoBloq(buffer, sizeof(buffer));
		if (readDataCode > 0) {
			string mensaje(buffer);
			Helper::encolar(&mensaje, &receiver2_queue, &mutex_receiver2);
		} else if (readDataCode == 0) {
			cliente2_conectado = false;
		}

		usleep(POLLING_DEADTIME);
	}

	pthread_exit(0);
}

//TODO el validator tambien se encarga de cerrar los socket una vez que el cliente se desconecto?
void *
validator_thread(void * argument) {

	struct puntajes puntaje;

	while (stop == false) {
		if (!receiver1_queue.empty()) {
			string message = receiver1_queue.front();
			receiver1_queue.pop();
			string scodigo = message.substr(0, 2);
			int codigo = atoi(scodigo.c_str());
			//Dependiendo del codigo, voy a ver que valido.
			switch (codigo) {
			//Escribe en el sender_queue.
			case CD_MOVIMIENTO_FELIX_I:
				int fila;
				int columna;
				cout << "Entro a movimiento felix" << endl;
				fila = atoi(message.substr(5, 1).c_str());
				columna = atoi(message.substr(6, 1).c_str());

				if (validateMovement(felix1, fila, columna, edificio)) {
					string mensaje_movimiento1 = scodigo
							+ Helper::fillMessage("1" + message.substr(2, 2));
					string mensaje_movimiento2 = scodigo
							+ Helper::fillMessage("2" + message.substr(2, 2));
					Helper::encolar(&mensaje_movimiento1, &sender1_queue,
							&mutex_sender1);
					Helper::encolar(&mensaje_movimiento2, &sender2_queue,
							&mutex_sender2);
				}
				break;
			case CD_PERDIDA_VIDA_I:
				cout << "Perdieron vida" << endl;
				if (validateLives(felix1)) {
					string message1(CD_PERDIDA_VIDA);
					string message2(CD_PERDIDA_VIDA);
					message1.append(Helper::fillMessage("1"));
					message2.append(Helper::fillMessage("2"));
					Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
					Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
				} else {
					string message1(CD_PERDIO);
					string message2(CD_PERDIO);
					message1.append(Helper::fillMessage("1"));
					message2.append(Helper::fillMessage("2"));
					Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
					Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
				}
				break;
			case CD_VENTANA_ARREGLADA_I:
				if (validateWindowFix(felix1, edificio)) {
					string message1(CD_VENTANA_ARREGLADA);
					string message2(CD_VENTANA_ARREGLADA);
					message1.append(Helper::fillMessage("1"));
					message2.append(Helper::fillMessage("2"));
					Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
					Helper::encolar(&message2, &sender2_queue, &mutex_sender2);

					puntaje.idJugador1 = felix1->id;
					//puntaje.idJugador2 = felix2->id;
					puntaje.jugando = true;
					puntaje.puntajeJugador1 = felix1->puntaje_parcial;
					//puntaje.puntajeJugador2 = felix2->puntaje_parcial;
					pthread_mutex_lock(&mutex_puntajes);
					puntajes_queue.push(puntaje);
					pthread_mutex_unlock(&mutex_puntajes);
				}
				break;
			}

		}
		if (!receiver2_queue.empty()) {
			string message = receiver2_queue.front();
			receiver2_queue.pop();

			//switch()
			//{

			//}
		}

		//Murieron los dos jugadores.
		if (cliente1_jugando == false && cliente2_jugando == false) {
			//Por race condition podria terminar antes de enviarme los  ultimos puntajes..
			puntaje.idJugador1 = felix1->id;
			//puntaje.idJugador2 = felix2->id;
			puntaje.jugando = false;
			puntaje.puntajeJugador1 = felix1->puntaje_parcial;
			//puntaje.puntajeJugador2 = felix2->puntaje_parcial;
			pthread_mutex_lock(&mutex_puntajes);
			puntajes_queue.push(puntaje);
			pthread_mutex_unlock(&mutex_puntajes);
			//Le doy sleep de 1 asi llego a mandar los puntajes?
			sleep(1);
			stop = true;
		}

		usleep(POLLING_DEADTIME);
	}

	pthread_exit(0);
}

void* sharedMemory_thread(void * arguments) {
	struct shmIds * shmIds = (struct shmIds *) arguments;
	int shmId = shmget(shmIds->shmId, 1024, 0660);
	int shmKAId = shmget(shmIds->shmKAId, 1024, 0660);
	sem_t * sem = sem_open(shmIds->semName, 0);
	struct puntajes * puntaje;
	char * buffer;
	struct timespec ts;
	ts.tv_sec = SEMAPHORE_TIMEOUT;

	puntaje = (struct puntajes *) shmat(shmId, NULL, 0);
	buffer = (char *) shmat(shmKAId, NULL, 0);

	while (stop == false) {
		//Envio de puntajes.
		if (!puntajes_queue.empty()) {
			pthread_mutex_lock(&mutex_puntajes);
			*puntaje = puntajes_queue.front();
			puntajes_queue.pop();
			//Bloquear y mandar por memo compartida los puntajes.
			pthread_mutex_unlock(&mutex_puntajes);
		}

		//Manenjo de keepalive
		if (sem_timedwait(sem, &ts) != -1) {

		} else {
			//Deberia setear alarma que si en cierto tiempo no se desbloquea
			//asumo que el servidor murio.
			//Averiguar por sem_timedwait.
			if (errno == ETIMEDOUT) {
				//TODO Murio el servidor asi que tengo que cancelar todo y cerrar todo.
				cout << "Se cerro el servidor torneo" << endl;
				sem_close(sem);
				sem_unlink(shmIds->semName);
				shmdt(puntaje);
				shmctl(shmId, IPC_RMID, 0);
				shmdt(buffer);
				shmctl(shmKAId, IPC_RMID, 0);
				stop = true;
			}
		}
		usleep(POLLING_DEADTIME);
	}

}

int randomRalphMovement() {
	return rand() % (EDIFICIO_COLUMNAS + 1);
}

int randomPaloma(int nivel) {
	if (nivel == 0)
		return rand() % (EDIFICIO_FILAS_1 + 1);
	else if (nivel == 1)
		return rand() % (EDIFICIO_FILAS_2 + 1);
	return 0;
}

char*
randomTorta() {
	char location[3];
	char aux[2];

	sprintf(aux, "%d", rand() % (EDIFICIO_FILAS_1));
	strcpy(location, aux);
	sprintf(aux, "%d", rand() % (EDIFICIO_COLUMNAS));
	strcat(location, aux);

	return location;
}

bool validateMovement(Felix * felix, int fila, int columna,
		Edificio * edificio) {
	if (((fila < edificio->filas || fila >= 0)
			&& (columna < edificio->columnas || columna >= 0))
			&& !edificio->ventanas[fila][columna].marquesina
			&& edificio->ventanas[fila][columna].felix == NULL) {

		edificio->ventanas[felix->posicion_x][felix->posicion_y].felix = NULL;
		edificio->ventanas[fila][columna].felix = felix;
		felix->posicion_x = fila;
		felix->posicion_y = fila;

		return true;
	}
	return false;
}

bool validateWindowFix(Felix * felix, Edificio * edificio) {
	if (edificio->ventanas[felix->posicion_x][felix->posicion_y].ventanaRota
			> 0) {
		felix->puntaje_parcial++;
		edificio->ventanas[felix->posicion_x][felix->posicion_y].ventanaRota--;
		return true;
	}
	return false;
}

bool validateLives(Felix * felix) {
	return --felix->cantidad_vidas == 0;
}

void SIGINT_Handler(int inum) {
	stop = true;
}
