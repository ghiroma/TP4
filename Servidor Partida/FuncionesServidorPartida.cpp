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

pid_t ppid;

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

struct puntajes * puntaje;
struct idsSharedResources shmIds;

/*
 * Retorna true si la diferencia de tiempo se da.
 */
bool TimeDifference(int timeDifference, time_t startingTime) {
	if ((time(0) - startingTime) > timeDifference) {
		return true;
	} else {
		return false;
	}
}

/*
 * Thread encargado de enviar los mensajes de los sucesos del juego no pertenecientes
 * a ninguno de los dos jugadores, siendo movimiento ralph, paloma,etc.
 */
void*
timer_thread(void* arg) {

	time_t startingTimeKeepAlive = time(0);
	time_t startingTimeRalph = time(0);
	time_t startingTimePaloma = time(0);
	time_t startingTimePersiana = time(0);
	time_t startingTimeTorta = time(0);

	//TODO Agregar variacion segun el nivel.
	while (stop == false && (cliente1_conectado || cliente2_conectado)) {
		if (TimeDifference(INTERVALOS_KEEPALIVE, startingTimeKeepAlive)
				== true) {
			string message(CD_ACK);
			string content;
			message.append(Helper::fillMessage(content));
			if (cliente1_conectado)
				Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			if (cliente2_conectado)
				Helper::encolar(&message, &sender2_queue, &mutex_sender2);
			startingTimeKeepAlive = time(0);
		}

		if (TimeDifference(INTERVALOS_RALPH, startingTimeRalph) == true) {
			char aux[5];
			string message(CD_MOVIMIENTO_RALPH);
			sprintf(aux, "%d", randomRalphMovement());
			message.append(Helper::fillMessage(aux));
			if (cliente1_conectado)
				Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			if (cliente2_conectado)
				Helper::encolar(&message, &sender2_queue, &mutex_sender2);
			startingTimeRalph = time(0);
		}

		if (TimeDifference(INTERVALOS_PALOMA, startingTimePaloma) == true) {
			string message(CD_PALOMA);
			char aux[5];
			sprintf(aux, "%d", randomPaloma(0));
			message.append(Helper::fillMessage(aux));
			if (cliente1_conectado)
				Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			if (cliente2_conectado)
				Helper::encolar(&message, &sender2_queue, &mutex_sender2);
			startingTimePaloma = time(0);
		}

		if (TimeDifference(INTERVALOS_TORTA, startingTimeTorta) == true) {
			//string message(CD_TORTA);
			//message.append(randomTorta());
			//Helper::encolar(&message, &sender1_queue, &mutex_sender1);
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
			Helper::desencolar(&message, &sender1_queue, &mutex_sender1);
			cout << "Mensaje a enviar: " << message.c_str() << endl;
			if (cliente1_conectado) {
				cSocket1->SendBloq(message.c_str(), message.length());
			}
		}

		usleep(POLLING_DEADTIME);
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
			Helper::desencolar(&message, &sender2_queue, &mutex_sender2);
			cout << "Mensaje a enviar: " << message.c_str() << endl;
			if (cliente2_conectado) {
				cSocket2->SendBloq(message.c_str(), message.length());
			}
		}
		usleep(POLLING_DEADTIME);
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
			cliente1_conectado = false;
			delete (cSocket1);
		}
		usleep(POLLING_DEADTIME);
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
			Helper::encolar(&mensaje, &receiver2_queue, &mutex_receiver2);
		} else if (readDataCode == 0) {
			cliente2_conectado = false;
			delete (cSocket2);
		}

		usleep(POLLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado de validar las acciones de los clientes y devolver mensajes
 * de confirmacion.
 */
void *
validator_thread(void * argument) {

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
				caseVentanaArreglada(2);
				break;
			}

		}

		if (!receiver2_queue.empty()) {
			string message = receiver2_queue.front();
			receiver2_queue.pop();
			string scodigo = message.substr(0, LONGITUD_CODIGO);
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
			}
		}

		usleep(POLLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado de verificar si el torneo esta vivo y grabar los puntajes
 * finales de los clientes.
 */
void*
sharedMemory_thread(void * arguments) {
	int shmId = shmget(shmIds.shmId, sizeof(struct puntajes), PERMISOS_SHM);
	if (shmId < 0) {
		/*cout << "SRV Partida error en shmget" << endl;
		 if (errno == ENOENT)
		 cout << "No existe egmento de memoria para dicho key" << endl;
		 if (errno == EACCES)
		 cout << "No se tienen permisos" << endl;
		 if (errno == EINVAL)
		 cout << "No se puede crear porque ya existe." << endl;
		 */
		exit(1);
	}
	puntaje = (struct puntajes *) shmat(shmId, (void *) 0, 0);
	if (puntaje == (void *) -1) {
		exit(1);
	}

	while (stop == false) {

		//Verifico si el padre esta vivo
		if (kill(ppid, 0) == -1) {
			stop = true;
		}

		//Perdieron ambos, asi que finalmente cierro.
		if (!cliente1_jugando && !cliente2_jugando)
		//if (cliente1_jugando && cliente2_jugando)
				{

			//sleep (21);

			puntaje->idJugador1 = felix1->id;
			puntaje->idJugador2 = felix2->id;
			puntaje->puntajeJugador1 = felix1->puntaje_parcial;
			//puntaje->puntajeJugador1 = 500;
			//puntaje->puntajeJugador2 = 750;
			puntaje->puntajeJugador2 = felix2->puntaje_parcial;
			puntaje->partidaFinalizadaOk = true;

			stop = true;
		}

		usleep(POLLING_DEADTIME);
	}
}

/*
 * Genero movimiento random de ralph.
 */
int randomRalphMovement() {
	return rand() % (EDIFICIO_COLUMNAS);
}

/*
 * Genero aparicion random de paloma.
 */
int randomPaloma(int nivel) {
	if (nivel == 0)
		return rand() % (EDIFICIO_FILAS_1);
	else if (nivel == 1)
		return rand() % (EDIFICIO_FILAS_2);
	return 0;
}

/*
 * Genero aparicion random de torta.
 */
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

/*
 * Genero aparicion random de persiana.
 */
char *
randomPersiana() {
	char location[3];
	char aux[2];

	//No hay persiana cerrada. Actualizo el edificio.
	if (filaPreviaPersiana != 0 && columnaPreviaPersiana != 0) {
		sprintf(aux, "%d", rand() % (EDIFICIO_FILAS_1));
		strcpy(location, aux);
		filaPreviaPersiana = atoi(aux);
		sprintf(aux, "%d", rand() % (EDIFICIO_COLUMNAS));
		strcat(location, aux);
		columnaPreviaPersiana = atoi(aux);
		return location;
	} else //Hay persiana cerrada. Debo abrirla.
	{
		sprintf(aux, "%d", filaPreviaPersiana);
		strcpy(location, aux);
		sprintf(aux, "%d", columnaPreviaPersiana);
		strcat(location, aux);
		filaPreviaPersiana = 0;
		columnaPreviaPersiana = 0;
		return location;
	}

}

/*
 * Valido si es posible realizar el movimiento de felix pedido.
 */
bool validateMovement(Felix * felix, int fila, int columna,
		Edificio * edificio) {

	cout << "Entro validacion movimiento" << endl;
	if (felix == NULL)
		cout << "Felix es nulo" << endl;
	if (edificio == NULL)
		cout << "El edificio es nulo" << endl;
	cout << "fila: " << fila << endl;
	cout << "columna: " << columna << endl;

	if (((fila < edificio->filas || fila >= 0)
			&& (columna < edificio->columnas || columna >= 0))
			&& !edificio->ventanas[fila][columna].marquesina
			&& edificio->ventanas[fila][columna].felix == NULL
			&& !edificio->ventanas[fila][columna].persiana) {

		edificio->ventanas[felix->posicion_x][felix->posicion_y].felix = NULL;
		edificio->ventanas[fila][columna].felix = felix;
		felix->posicion_x = fila;
		felix->posicion_y = fila;

		cout << "Salgo validacion movimiento true" << endl;
		return true;
	}
	cout << "Salgo validacion movimiento false" << endl;
	return false;
}

/*
 * Valido si se arreglo una ventana.
 */
bool validateWindowFix(Felix * felix, Edificio * edificio) {
	if (edificio->ventanas[felix->posicion_x][felix->posicion_y].ventanaRota > 0
			&& !edificio->ventanas[felix->posicion_x][felix->posicion_y].persiana) {
		felix->puntaje_parcial++;
		edificio->ventanas[felix->posicion_x][felix->posicion_y].ventanaRota--;
		return true;
	}
	return false;
}

/*
 * Valido si se perdio una vida y murio felix.
 */
bool validateLives(Felix * felix) {
	return --felix->cantidad_vidas == 0;
}

/*
 * Caso de recepcion de mensaje de movimiento de felix.
 */

void caseMovimientoFelix(int jugador, string *message) {
	int fila;
	int columna;
	cout << "Entro a movimiento felix, mensaje = " << *message << endl;
	fila = atoi(message->substr(5, 1).c_str());
	columna = atoi(message->substr(6, 1).c_str());

	if (jugador == 1) {
		if (validateMovement(felix1, fila, columna, edificio)) {
			char auxFila[2];
			char auxColumna[2];
			char aux1[5] = { "1" };
			char aux2[5] = { "2" };

			cout << "Por guardar auxfila y auxcoluna" << endl;

			sprintf(auxFila, "%d", fila);
			sprintf(auxColumna, "%d", columna);

			cout << "Por concatenar" << endl;

			strcat(aux1, auxFila);
			strcat(aux1, auxColumna);
			strcat(aux2, auxFila);
			strcat(aux2, auxColumna);

			cout<<"mensaje movimiento a enviar: "<<aux1<<endl;

			string mensaje_movimiento1 = message->substr(0, LONGITUD_CODIGO)
					+ Helper::fillMessage(aux1);
			string mensaje_movimiento2 = message->substr(0, LONGITUD_CODIGO)
					+ Helper::fillMessage(aux2);
			Helper::encolar(&mensaje_movimiento1, &sender1_queue,
					&mutex_sender1);
			Helper::encolar(&mensaje_movimiento2, &sender2_queue,
					&mutex_sender2);
			cout << "Mensaje encolado: " << mensaje_movimiento1 << endl;
			cout << "Encole mensaje de movimiento felix" << endl;
		}
	} else {
		if (validateMovement(felix2, fila, columna, edificio)) {
			char auxFila[2];
			char auxColumna[2];
			char aux1[5] = { "2" };
			char aux2[5] = { "1" };

			sprintf(auxFila, "%d", fila);
			sprintf(auxColumna, "%d", columna);

			strcat(aux1, auxFila);
			strcat(aux1, auxColumna);
			strcat(aux2, auxFila);
			strcat(aux2, auxColumna);

			cout<<"mensaje movimiento a enviar: "<<aux1<<endl;

			string mensaje_movimiento1 = message->substr(0, LONGITUD_CODIGO)
					+ Helper::fillMessage(aux1);
			string mensaje_movimiento2 = message->substr(0, LONGITUD_CODIGO)
					+ Helper::fillMessage(aux2);
			Helper::encolar(&mensaje_movimiento1, &sender1_queue,
					&mutex_sender1);
			Helper::encolar(&mensaje_movimiento2, &sender2_queue,
					&mutex_sender2);
		}

	}
}

/*
 * Caso de recepcion de mensaje de perdida de vida.
 */

void casePerdidaVida(int jugador) {
	if (jugador == 1) {
		cout << "Perdieron vida" << endl;
		if (!validateLives(felix1)) {
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
			cliente1_jugando = false;
		}
	} else {
		cout << "Perdieron vida" << endl;
		if (!validateLives(felix2)) {
			string message1(CD_PERDIDA_VIDA);
			string message2(CD_PERDIDA_VIDA);
			message1.append(Helper::fillMessage("2"));
			message2.append(Helper::fillMessage("1"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
		} else {
			string message1(CD_PERDIO);
			string message2(CD_PERDIO);
			message1.append(Helper::fillMessage("2"));
			message2.append(Helper::fillMessage("1"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
			cliente2_jugando = false;
		}
	}
}

/*
 * Caso de recepcion de mensaje de ventana arreglada
 */

void caseVentanaArreglada(int jugador) {
	if (jugador == 1) {
		if (validateWindowFix(felix1, edificio)) {
			string message1(CD_VENTANA_ARREGLADA);
			string message2(CD_VENTANA_ARREGLADA);
			message1.append(Helper::fillMessage("1"));
			message2.append(Helper::fillMessage("2"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
		}
	} else {
		if (validateWindowFix(felix2, edificio)) {
			string message1(CD_VENTANA_ARREGLADA);
			string message2(CD_VENTANA_ARREGLADA);
			message1.append(Helper::fillMessage("2"));
			message2.append(Helper::fillMessage("1"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
		}
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
}
