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

key_t shmId;

int cantVidas;
int filaPreviaPersiana = 0;
int columnaPreviaPersiana = 0;
int shm=0;

bool stop = false;
bool cliente1_conectado = true;
bool cliente2_conectado = true;
bool cliente1_jugando = true;
bool cliente2_jugando = true;
bool cliente1_listo = false;
bool cliente2_listo = false;
bool torneoMuerto = false;

queue<string> receiver1_queue;
queue<string> receiver2_queue;
queue<string> sender1_queue;
queue<string> sender2_queue;

CommunicationSocket * cSocket1;
CommunicationSocket * cSocket2;

ServerSocket * sSocket;

Felix *felix1;
Felix *felix2;

Edificio *edificio;

Semaforo * semaforoPartida;
Semaforo * semaforoTorneo;

pthread_mutex_t mutex_receiver1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_receiver2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender2 = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_edificio = PTHREAD_MUTEX_INITIALIZER;

struct puntajes * puntaje;

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

		if (cliente1_listo == true && cliente2_listo == true) {

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
	while (!stop && cliente1_conectado) {
		if (!sender1_queue.empty()) {
			//Lo que venga del timer y validator, se replica a ambos jugadores.
			string message;
			message = Helper::desencolar(&sender1_queue, &mutex_sender1);
			if (cliente1_conectado && cSocket1 != NULL) {
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
	while (!stop && cliente2_conectado) {
		if (!sender2_queue.empty()) {
			string message;
			message = Helper::desencolar(&sender2_queue, &mutex_sender2);
			if (cliente2_conectado && cSocket2 != NULL) {
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

//TODO si se desconecto y nunca estuvo listo, entonces decirle al otro cliente que se desconecte.
void*
receiver1_thread(void * fd) {
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	int readDataCode;
	bzero(buffer, sizeof(buffer));

	while (!stop && cliente1_conectado && cSocket1 != NULL) {
		readDataCode = cSocket1->ReceiveBloq(buffer, sizeof(buffer));
		if (readDataCode > 0) {
			string aux(buffer);
			Helper::encolar(&aux, &receiver1_queue, &mutex_receiver1);
		} else if (readDataCode == 0) {
			//TODO decirle al jugador nro2 que el cliente 1 se desconecto.
			string message(CD_OPONENTE_DESCONECTADO);
			message.append(Helper::fillMessage("0"));
			Helper::encolar(&message, &sender2_queue, &mutex_sender2);
			cliente1_conectado = false;
			cliente1_jugando = false;
			delete (cSocket1);
			cSocket1 = NULL;
			cout << "Se desconecto el cliente 1" << endl;
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

	while (!stop && cliente2_conectado && cSocket2 != NULL) {
		readDataCode = cSocket2->ReceiveNoBloq(buffer, sizeof(buffer));
		if (readDataCode > 0) {
			string mensaje(buffer);
			Helper::encolar(&mensaje, &receiver2_queue, &mutex_receiver2);
		} else if (readDataCode == 0) {
			string message(CD_OPONENTE_DESCONECTADO);
			message.append(Helper::fillMessage("0"));
			Helper::encolar(&message, &sender1_queue, &mutex_sender1);
			cliente2_conectado = false;
			cliente2_jugando = false;
			delete (cSocket2);
			cSocket2 = NULL;
			cout << "Se desconecto el cliente 2" << endl;
		}

		usleep(POOLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado de validar las acciones de los clientes y devolver mensajes
 * de confirmacion.
 */

void *validator2_thread(void* arguments) {
	while (!stop && cliente2_conectado) {
		if (!receiver2_queue.empty()) {
			string message = Helper::desencolar(&receiver2_queue, &mutex_receiver2);
			if (message.length() == (LONGITUD_CODIGO + LONGITUD_CONTENIDO)) {
				string scodigo = message.substr(0, LONGITUD_CODIGO);
				int codigo = atoi(scodigo.c_str());
				switch (codigo) {
				case CD_MOVIMIENTO_FELIX_I:
					caseMovimientoFelix(JUGADOR_2, &message);
					break;
				case CD_PERDIDA_VIDA_I:
					casePerdidaVida(JUGADOR_2);
					break;
				case CD_VENTANA_ARREGLADA_I:
					caseVentanaArreglada(JUGADOR_2);
					break;
				case CD_VENTANA_ARREGLANDO_I:
					caseVentanaArreglando(JUGADOR_2);
					break;
				case CD_ID_JUGADOR_I: {
					int id = atoi(message.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str());
					//cout<<"Servidor Partida: recibi id: "<<id<<endl;
					caseIdJugador(JUGADOR_2, id);
					break;
				}
				case CD_JUGADOR_LISTO_I:
					caseJugadorListo(JUGADOR_2);
					break;
				}
			}
		}
		usleep(POOLING_DEADTIME);
	}
}

void *validator1_thread(void * argument) {

	while (!stop && cliente1_conectado) {
		if (!receiver1_queue.empty()) {
			string message = Helper::desencolar(&receiver1_queue, &mutex_receiver1);
			if (message.length() == (LONGITUD_CODIGO + LONGITUD_CONTENIDO)) {
				string scodigo = message.substr(0, LONGITUD_CODIGO);
				int codigo = atoi(scodigo.c_str());
				switch (codigo) {
				case CD_MOVIMIENTO_FELIX_I:
					caseMovimientoFelix(JUGADOR_1, &message);
					break;
				case CD_PERDIDA_VIDA_I:
					casePerdidaVida(JUGADOR_1);
					break;
				case CD_VENTANA_ARREGLADA_I:
					caseVentanaArreglada(JUGADOR_1);
					break;
				case CD_VENTANA_ARREGLANDO_I:
					caseVentanaArreglando(JUGADOR_1);
					break;
				case CD_ID_JUGADOR_I: {
					int id = atoi(message.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str());
					//cout<<"Servidor Partida: recibi id: "<<id<<endl;
					caseIdJugador(JUGADOR_1, id);
					break;
				}
				case CD_JUGADOR_LISTO_I:
					caseJugadorListo(JUGADOR_1);
					break;
				}
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
		shm = shmget(shmId, sizeof(struct puntajes), PERMISOS_SHM);
		if (shm < 0) {
			throw "Error al obtener memoria compartida";
		}
		puntaje = (struct puntajes *) shmat(shm, (void *) 0, 0);
		if (puntaje == (void *) -1) {
			throw "Error al mapear la memoria compartida";
		}

		semaforoPartida = new Semaforo(SEMAFORO_PARTIDA);
		semaforoTorneo = new Semaforo(SEMAFORO_TORNEO);

		while (!stop) {

			//Verifico si el padre esta vivo
			if (kill(pid, 0) == -1) {
				stop = true;
				torneoMuerto = true;
			}

			//Perdieron ambos, asi que finalmente cierro.
			if (!cliente1_jugando && !cliente2_jugando && (cliente1_conectado || cliente2_conectado)) {

				//Termino la partida asi que envio mensaje a clientes de terminar.
				string message(CD_FIN_PARTIDA);
				message.append(Helper::fillMessage("0"));
				if (cSocket1 != NULL)
					cSocket1->SendBloq(message.c_str(), message.length());
				if (cSocket2 != NULL)
					cSocket2->SendBloq(message.c_str(), message.length());

				//cout << "Menasje fin de partida enviado" << endl;

				cout << "SRV PARTIDA: esperando semaforo partida" << endl;
				semaforoPartida->P();
				puntaje->idJugador1 = felix1->id;
				puntaje->idJugador2 = felix2->id;
				puntaje->puntajeJugador1 = felix1->puntaje_parcial;
				puntaje->puntajeJugador2 = felix2->puntaje_parcial;
				puntaje->partidaFinalizadaOk = true;

				cout << "SRV PARTIDA: Puntajes escritos" << endl;

				stop = true;
				sleep(1);
				semaforoTorneo->V();
			} else if (!cliente1_conectado && !cliente2_conectado) {
				cout << "SRV PARTIDA: clientes desconectados" << endl;
				semaforoPartida->P();
				puntaje->idJugador1 = felix1->id;
				puntaje->idJugador2 = felix2->id;
				puntaje->partidaFinalizadaOk = false;
				puntaje->puntajeJugador1 = 0;
				puntaje->puntajeJugador2 = 0;

				stop = true;
				sleep(1);
				semaforoTorneo->V();
			}

			usleep(POOLING_DEADTIME);
		}
	} catch (const char * err) {
		cout << "Error inesperado: " << err << " los puntos de esta partida no se veran reflejados en el torneo." << endl;
		exit(1);
	}
	pthread_exit(0);
}

/*
 * Caso de recepcion de mensaje de movimiento de felix.
 */

void caseMovimientoFelix(int jugador, string *message) {
	int columna = atoi(message->substr(3, 2).c_str());
	int fila = atoi(message->substr(5, 2).c_str());

	Felix * felixJugador = 0;

	if (jugador == JUGADOR_1)
		felixJugador = felix1;
	else if (jugador == JUGADOR_2)
		felixJugador = felix2;

	int nuevaFila = felixJugador->fila + fila;
	int nuevaColumna = felixJugador->columna + columna;

	if (felixJugador->mover(nuevaColumna, nuevaFila, edificio)) {

//Construyo nuevo mensaje a enviar.
		char auxFila[2];
		char auxColumna[2];
		char aux1[5] = { "1" };
		char aux2[5] = { "2" };

		sprintf(auxFila, "%d", felixJugador->fila);
		sprintf(auxColumna, "%d", felixJugador->columna);

		strcat(aux1, auxColumna);
		strcat(aux1, auxFila);
		strcat(aux2, auxColumna);
		strcat(aux2, auxFila);
//Envio mensaje al jugador1.
		string message1(CD_MOVIMIENTO_FELIX);
		string message2(CD_MOVIMIENTO_FELIX);
		if (felixJugador == felix1) {
			message1.append(Helper::fillMessage(aux1));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			message2.append(Helper::fillMessage(aux2));
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
		} else if (felixJugador == felix2) {
			message1.append(Helper::fillMessage(aux2));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			message2.append(Helper::fillMessage(aux1));
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
		}

	}
}

/*
 * Caso de recepcion de mensaje de perdida de vida.
 */

void casePerdidaVida(int jugador) {
	int perderVida = 0;
	if (jugador == 1) {
		perderVida = felix1->perderVida();
		if (perderVida == 1) {
			string message1(CD_PERDIDA_VIDA);
			string message2(CD_PERDIDA_VIDA);
			//TODO Corregir hardcodeo.
			message1.append(Helper::fillMessage("100"));
			message2.append(Helper::fillMessage("200"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
			//pthread_mutex_lock(&mutex_edificio);
			felix1->mover(0, 0, edificio);
			//pthread_mutex_unlock(&mutex_edificio);
		} else if (perderVida == -1) {
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
			cliente1_jugando = false;
		}
	} else {
		perderVida = felix2->perderVida();
		if (perderVida == 1) {
			//TODO Sacar harcodeo.
			string message1(CD_PERDIDA_VIDA);
			string message2(CD_PERDIDA_VIDA);
			message1.append(Helper::fillMessage("240"));
			message2.append(Helper::fillMessage("140"));
			Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
			Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
			//pthread_mutex_lock(&mutex_edificio);
			felix2->mover(EDIFICIO_COLUMNAS - 1, 0, edificio);
			//pthread_mutex_unlock(&mutex_edificio);
		} else if (perderVida == -1) {
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
			cliente2_jugando = false;
		}
	}
}

/*
 * Caso de recepcion de mensaje de ventana arreglada
 */

void caseVentanaArreglada(int jugador) {
	pthread_mutex_lock(&mutex_edificio);
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
	}

	if (tramoFinalizado(edificio)) {
		/*		string message(CD_SUBIR_TRAMO);
		 message.append(Helper::fillMessage("0"));
		 Helper::encolar(&message,&sender1_queue,&mutex_sender1);
		 Helper::encolar(&message,&sender2_queue,&mutex_sender2);
		 edificio->CambiarTramo();
		 if(cliente1_jugando)
		 felix1->mover(0,0,edificio);
		 if(cliente2_jugando)
		 felix2->mover(EDIFICIO_COLUMNAS-1,0,edificio);*/
		cliente1_jugando = false;
		cliente2_jugando = false;
	}
	pthread_mutex_unlock(&mutex_edificio);

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

void caseIdJugador(int jugador, int id) {
	string message(CD_CANTIDAD_VIDAS);
	char aux[5];
	sprintf(aux, "%d", cantVidas);
	message.append(Helper::fillMessage(aux));

	if (jugador == JUGADOR_1) {
		felix1 = new Felix(cantVidas, id);
		string posicion = posicionInicial1();
		Helper::encolar(&posicion, &sender1_queue, &mutex_sender1);
		Helper::encolar(&message, &sender1_queue, &mutex_sender1);
	} else if (jugador == JUGADOR_2) {
		felix2 = new Felix(cantVidas, id);
		string posicion = posicionInicial2();
		Helper::encolar(&posicion, &sender2_queue, &mutex_sender1);
		Helper::encolar(&message, &sender2_queue, &mutex_sender1);
	}
}

void caseJugadorListo(int jugador) {
	if (jugador == JUGADOR_1) {
		cliente1_listo = true;
	} else if (jugador == JUGADOR_2) {
		cliente2_listo = true;
	}

	if (cliente1_listo == true && cliente2_listo == true) {
		string message(CD_EMPEZAR_PARTIDA);
		message.append(Helper::fillMessage("0"));
		Helper::encolar(&message, &sender1_queue, &mutex_sender1);
		Helper::encolar(&message, &sender2_queue, &mutex_sender2);
	}
}

void SIGINT_Handler(int inum) {
	stop = true;
}

void liberarRecursos() {
	if (puntaje != NULL)
	{
		shmdt(puntaje);
	}
	if (torneoMuerto) {
		shmctl(shm, IPC_RMID, NULL);
		const char * semName =semaforoPartida->getName();
		delete(semaforoPartida);
		sem_unlink(semName);
		semName = semaforoTorneo->getName();
		delete(semaforoTorneo);
		sem_unlink(semName);
	} else {
		delete (semaforoPartida);
		semaforoPartida = NULL;
		delete (semaforoTorneo);
		semaforoTorneo = NULL;
	}
	delete (sSocket);
	sSocket = NULL;
	delete (cSocket1);
	cSocket1 = NULL;
	delete (cSocket2);
	cSocket2 = NULL;
	delete (felix1);
	felix1 = NULL;
	delete (felix2);
	felix2 = NULL;

	//cout << "Recursos liberados" << endl;
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
	felix2->fila = fila;
	felix2->columna = columna;
	return message;
}

bool tramoFinalizado(Edificio * edificio) {
	int result = 0;
	for (int fila = 0; fila < edificio->filas; fila++) {
		for (int columna = 0; columna < edificio->columnas; columna++) {
			if (!(fila == 0 && columna == 2))
				result += edificio->ventanas[fila][columna].ventanaRota;
		}
	}
	//cout<<"Faltan arreglar "<<result<<" ventanas"<<endl;
	return result == 0;
}
