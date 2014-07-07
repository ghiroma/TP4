/*
 * Funciones.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "FuncionesServidorPartida.h"
#include <iostream>
#include "Support/Constantes.h"
#include "Clases/Semaforo.h"
#include <string.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include "Clases/Edificio.h"
#include "Clases/Timer.h"
#include "Support/Helper.h"
#include "Support/Estructuras.h"
#include <cstdio>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

using namespace std;

pid_t pid;

int filaPreviaPersiana = 0;
int columnaPreviaPersiana = 0;

bool stop = false;
bool cliente1_conectado = true;
bool cliente2_conectado = true;
bool cliente1_jugando = true;
bool cliente2_jugando = true;

queue<string> receiver1_queue;
queue<string> receiver2_queue;
queue<string> sender1_queue;
queue<string> sender2_queue;

CommunicationSocket * cSocket1;
CommunicationSocket * cSocket2;

Felix *felix1;
Felix *felix2;

Edificio *edificio;

pthread_mutex_t mutex_receiver1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_receiver2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender2 = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_edificio = PTHREAD_MUTEX_INITIALIZER;

struct puntajes * puntaje;
struct idsSharedResources shmIds;

/*
 * Thread encargado de enviar los mensajes de los sucesos del juego no pertenecientes
 * a ninguno de los dos jugadores, siendo movimiento ralph, paloma,etc.
 */
void*
timer_thread(void* arg) {
	string message;
	Timer timer;

	while (!stop && (cliente1_conectado || cliente2_conectado)) {

		message = timer.keepAlive();
		if (message.length() > 0) {
			Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message, &sender2_queue, &mutex_sender2);
		}

		message = timer.ralph(edificio->nivel);
		if (message.length() > 0) {
			Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message, &sender2_queue, &mutex_sender2);
		}

		message = timer.paloma(edificio->nivel);
		if (message.length() > 0) {
			Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message, &sender2_queue, &mutex_sender2);
		}

		usleep(POOLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado del envio de mensajes al jugador 1.
 */
void*
sender1_thread(void * arguments) {
	while (stop == false && cliente1_conectado) {
		if (!sender1_queue.empty()) {
			//Lo que venga del timer y validator, se replica a ambos jugadores.
			string message;
			message = Helper::desencolar(&sender1_queue, &mutex_sender1);
			cout << "Mensaje a enviar: " << message << endl;
			if (cliente1_conectado) {
				cSocket1->SendBloq(message.c_str(), message.length());
			}
		}

		usleep(POOLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado del envio de mensajes al jugador2.
 */
void*
sender2_thread(void * arguments) {
	while (stop == false && cliente2_conectado) {
		if (!sender2_queue.empty()) {
			//Lo que venga del timer y validator, se replica a ambos jugadores.
			string message;
			message = Helper::desencolar(&sender2_queue, &mutex_sender2);
			cout << "Mensaje a enviar: " << message << endl;
			if (cliente2_conectado) {
				cSocket2->SendBloq(message.c_str(), message.length());
			}
		}
		usleep(POOLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado de la recepcion de mensajes del jugador1 y detectar la
 * desconexion del jugador 1.
 */
void*
receiver1_thread(void * fd) {
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	int readDataCode;
	bzero(buffer, sizeof(buffer));

	while (stop == false && cliente1_conectado) {
		readDataCode = cSocket1->ReceiveBloq(buffer, sizeof(buffer));
		if (readDataCode > 0) {
			string aux(buffer);
			cout << "Recibi mensaje: " << buffer << endl;
			Helper::encolar(&aux, &receiver1_queue, &mutex_receiver1);
		} else if (readDataCode == 0) {
			//TODO decirle al jugador nro2 que el cliente 1 se desconecto.
			string message(CD_OPONENTE_DESCONECTADO);
			message.append(Helper::fillMessage("0"));
			Helper::encolar(&message, &sender2_queue, &mutex_sender2);
			cliente1_conectado = false;
			cliente1_jugando = false;
			delete (cSocket1);
		}
		usleep(POOLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado de la recepcion de mensajes del jugador 2 y detectar la
 * desconexion del jugador 2
 */
void*
receiver2_thread(void * fd) {
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	int readDataCode;
	bzero(buffer, sizeof(buffer));

	while (stop == false && cliente2_conectado) {
		readDataCode = cSocket2->ReceiveNoBloq(buffer, sizeof(buffer));
		if (readDataCode > 0) {
			string mensaje(buffer);
			cout << "Recibi mensaje: " << mensaje << endl;
			Helper::encolar(&mensaje, &receiver2_queue, &mutex_receiver2);
		} else if (readDataCode == 0) {
			string message(CD_OPONENTE_DESCONECTADO);
			message.append(Helper::fillMessage("0"));
			Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			cliente2_conectado = false;
			cliente2_jugando = false;
			delete (cSocket2);
		}

		usleep(POOLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado de validar las acciones de los clientes y devolver mensajes
 * de confirmacion.
 */
void *validator_thread(void * argument) {

	while (stop == false && (cliente1_conectado || cliente2_conectado)) {
		if (!receiver1_queue.empty()) {
			string message = receiver1_queue.front();
			receiver1_queue.pop();
			string scodigo = message.substr(0, LONGITUD_CODIGO);
			int codigo = atoi(scodigo.c_str());
			switch (codigo) {
			//Escribe en el sender_queue.
			case CD_MOVIMIENTO_FELIX_I:
				caseMovimientoFelix(1, &message);
				break;
			case CD_PERDIDA_VIDA_I:
				casePerdidaVida(1);
				break;
			case CD_VENTANA_ARREGLADA_I:
				caseVentanaArreglada(1);
				break;
			case CD_VENTANA_ARREGLANDO_I:
				caseVentanaArreglando(1);
				break;
			}

		}

		if (!receiver2_queue.empty()) {
			string message = receiver2_queue.front();
			receiver2_queue.pop();
			string scodigo = message.substr(0, LONGITUD_CODIGO);
			cout << "Codigo leido " << scodigo << endl;
			int codigo = atoi(scodigo.c_str());
			switch (codigo) {
			case CD_MOVIMIENTO_FELIX_I:
				caseMovimientoFelix(2, &message);
				break;
			case CD_PERDIDA_VIDA_I:
				casePerdidaVida(2);
				break;

			case CD_VENTANA_ARREGLADA_I:
				caseVentanaArreglada(2);
				break;
			case CD_VENTANA_ARREGLANDO_I:
				caseVentanaArreglando(2);
				break;
			}
		}

		usleep(POOLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado de verificar si el torneo esta vivo y grabar los puntajes
 * finales de los clientes.
 */
void*
sharedMemory_thread(void * arguments) {
	try {
		int shmId = shmget(shmIds.shmId, sizeof(struct puntajes), PERMISOS_SHM);
		if (shmId < 0) {
			throw "Error al obtener memoria compartida";
		}
		puntaje = (struct puntajes *) shmat(shmId, (void *) 0, 0);
		if (puntaje == (void *) -1) {
			throw "Error al mapear la memoria compartida";
		}

		while (stop == false) {

			//Verifico si el padre esta vivo
			if (kill(pid, 0) == -1) {
				stop = true;
			}

			//Perdieron ambos, asi que finalmente cierro.
			if (!cliente1_jugando && !cliente2_jugando) {

				puntaje->idJugador1 = felix1->id;
				puntaje->idJugador2 = felix2->id;
				puntaje->puntajeJugador1 = felix1->puntaje_parcial;
				puntaje->puntajeJugador2 = felix2->puntaje_parcial;
				puntaje->partidaFinalizadaOk = true;

				cout << "SRV PARTIDA: Puntajes escritos" << endl;

				stop = true;
			}

			usleep(POOLING_DEADTIME);
		}
	} catch (const char * err) {
		cout << "Error inesperado: " << err << endl;
		exit(1);
	}
	pthread_exit(0);
}

/*
 * Caso de recepcion de mensaje de movimiento de felix.
 */

void caseMovimientoFelix(int jugador, string *message) {
	int fila;
	int columna;
	columna = atoi(message->substr(5, 1).c_str());
	fila = atoi(message->substr(6, 1).c_str());

	if (jugador == 1) {
		if (felix1->mover(columna, fila, edificio)) {
			char auxFila[2];
			char auxColumna[2];
			char aux1[5] = { "1" };
			char aux2[5] = { "2" };

			sprintf(auxFila, "%d", fila);
			sprintf(auxColumna, "%d", columna);

			strcat(aux1, auxColumna);
			strcat(aux1, auxFila);
			strcat(aux2, auxColumna);
			strcat(aux2, auxFila);

			string mensaje_movimiento1 = message->substr(0, LONGITUD_CODIGO) + Helper::fillMessage(aux1);
			string mensaje_movimiento2 = message->substr(0, LONGITUD_CODIGO) + Helper::fillMessage(aux2);

			Helper::encolar(&mensaje_movimiento1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&mensaje_movimiento2, &sender2_queue, &mutex_sender2);
		}
	} else {
		if (felix2->mover(columna, fila, edificio)) {
			char auxFila[2];
			char auxColumna[2];
			char aux1[5] = { "2" };
			char aux2[5] = { "1" };

			sprintf(auxFila, "%d", fila);
			sprintf(auxColumna, "%d", columna);

			strcat(aux1, auxColumna);
			strcat(aux1, auxFila);
			strcat(aux2, auxColumna);
			strcat(aux2, auxFila);

			string mensaje_movimiento1 = message->substr(0, LONGITUD_CODIGO) + Helper::fillMessage(aux1);
			string mensaje_movimiento2 = message->substr(0, LONGITUD_CODIGO) + Helper::fillMessage(aux2);
			Helper::encolar(&mensaje_movimiento1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&mensaje_movimiento2, &sender2_queue, &mutex_sender2);
		}

	}
}

/*
 * Caso de recepcion de mensaje de perdida de vida.
 */

void casePerdidaVida(int jugador) {
	if (jugador == 1) {
		if (!felix1->perderVida()) {
			cout << "Felix1 perdio vida, vidas actuales: " << felix1->cantidad_vidas << endl;
			string message1(CD_PERDIDA_VIDA);
			string message2(CD_PERDIDA_VIDA);
			//TODO Corregir hardcodeo.
			message1.append(Helper::fillMessage("100"));
			message2.append(Helper::fillMessage("200"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
			cout << "Encole mensaje perdida vida" << endl;
			felix1->mover(0, 0, edificio);
		} else {
			cout << "Felix1 perdio EL juego, vidas actuales" << felix1->cantidad_vidas << endl;
			string message1(CD_PERDIO);
			string message2(CD_PERDIO);
			message1.append(Helper::fillMessage("1"));
			message2.append(Helper::fillMessage("2"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
			//pthread_mutex_lock(&mutex_edificio);
			edificio->ventanas[felix1->fila][felix1->columna].ocupado = false;
			edificio->ventanas[0][0].ocupado = false;
			//pthread_mutex_unlock(&mutex_edificio);
			cout << "Jugador 1 muerto, posicion liberada fila: "<<felix1->fila<<" columna: "<<felix1->columna << endl;
			cliente1_jugando = false;
		}
	} else {
		if (!felix2->perderVida()) {
			cout << "Felix2 perdio vida, vidas actuales" << felix2->cantidad_vidas << endl;
			//TODO Sacar harcodeo.
			string message1(CD_PERDIDA_VIDA);
			string message2(CD_PERDIDA_VIDA);
			message1.append(Helper::fillMessage("240"));
			message2.append(Helper::fillMessage("140"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
			cout << "Encole mensaje perdida vida" << endl;
			felix2->mover(0, EDIFICIO_COLUMNAS - 1, edificio);
		} else {
			cout << "Felix2 perdio el Juego, vidas actuales" << felix2->cantidad_vidas << endl;
			string message1(CD_PERDIO);
			string message2(CD_PERDIO);
			message1.append(Helper::fillMessage("2"));
			message2.append(Helper::fillMessage("1"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
			//pthread_mutex_lock(&mutex_edificio);
			edificio->ventanas[felix2->fila][felix2->columna].ocupado = false;
			edificio->ventanas[0][EDIFICIO_COLUMNAS - 1].ocupado = false;
			//pthread_mutex_unlock(&mutex_edificio);
			cout << "Jugador 2 muerto, posicion liberada" << endl;
			cliente2_jugando = false;
		}
	}
}

/*
 * Caso de recepcion de mensaje de ventana arreglada
 */

void caseVentanaArreglada(int jugador) {
	//pthread_mutex_lock(&mutex_edificio);
	if (jugador == 1) {
		if (felix1->reparar(edificio)) {
			string message1(CD_VENTANA_ARREGLADA);
			string message2(CD_VENTANA_ARREGLADA);
			message1.append(Helper::fillMessage("1"));
			message2.append(Helper::fillMessage("2"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
		}
	} else {
		if (felix2->reparar(edificio)) {
			string message1(CD_VENTANA_ARREGLADA);
			string message2(CD_VENTANA_ARREGLADA);
			message1.append(Helper::fillMessage("2"));
			message2.append(Helper::fillMessage("1"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
		}
		//pthread_mutex_unlock(&mutex_edificio);

		if (tramoFinalizado(edificio)) {
			cout << "Se arreglaron todas las ventanas" << endl;
			cliente1_jugando = false;
			cliente2_jugando = false;
		}
	}
}

void caseVentanaArreglando(int jugador) {
	string message(CD_VENTANA_ARREGLANDO);
	message.append(Helper::fillMessage("0"));
	if (jugador == 1) {
		Helper::encolar(&message, &sender2_queue, &mutex_sender2);
	} else {
		Helper::encolar(&message, &sender1_queue, &mutex_sender1);
	}
}

void SIGINT_Handler(int inum) {
	stop = true;
}

void liberarRecursos() {
	if (puntaje != NULL)
		shmdt(puntaje);
	shmctl(shmIds.shmId, IPC_RMID, 0);
	delete (cSocket1);
	delete (cSocket2);
	cout << "Partida -> terimino liberrarRecursos" << endl;
}

string posicionInicial1() {
	string message(CD_POSICION_INICIAL);
	int fila = 0;
	int columna = 0;
	char cFila[2];
	char cColumna[2];
	char cPos[3];
	sprintf(cFila, "%d", fila);
	sprintf(cColumna, "%d", columna);
	strcpy(cPos, cColumna);
	strcat(cPos, cFila);
	message.append(Helper::fillMessage(cPos));
	felix1->fila = fila;
	felix1->columna = columna;
	return message;
}

string posicionInicial2() {
	string message(CD_POSICION_INICIAL);
	int fila = 0;
	int columna = EDIFICIO_COLUMNAS - 1;
	char cFila[2];
	char cColumna[2];
	char cPos[3];
	sprintf(cFila, "%d", fila);
	sprintf(cColumna, "%d", columna);
	strcpy(cPos, cColumna);
	strcat(cPos, cFila);
	message.append(Helper::fillMessage(cPos));
	if(felix2==NULL)
		cout<<"Felix 2 es NULOO"<<endl;
	felix2->fila = fila;
	felix2->columna = columna;
	cout<<"Posicion inicial 2: "<<message<<endl;
	return message;
}

bool tramoFinalizado(Edificio * edificio) {
	int result = 0;
	//pthread_mutex_lock(&mutex_edificio);
	for (int fila = 0; fila < edificio->filas; fila++) {
		for (int columna = 0; columna < edificio->columnas; columna++) {
			if (!(fila == 0 && columna == 2))
				result += edificio->ventanas[fila][columna].ventanaRota;
		}
	}
	//pthread_mutex_unlock(&mutex_edificio);
	return result == 0;
}
