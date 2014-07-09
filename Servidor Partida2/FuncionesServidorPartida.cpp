/*
 * Funciones.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "FuncionesServidorPartida.h"
#include "Support/Constantes.h"
#include "Support/Helper.h"
#include "Support/Estructuras.h"
#include "Clases/Semaforo.h"
#include "Clases/Edificio.h"
#include "Clases/Timer.h"
#include "Clases/Partida.h"
#include "Clases/Mensaje.h"
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <map>

using namespace std;

int cantVidas;

pid_t ppid;

bool stop = false;

queue<Mensaje> cola_mensajes_recibir;
queue<Mensaje> cola_mensajes_enviar;

pthread_mutex_t mutex_receiver1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_receiver2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender2 = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_edificio = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_partidas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_mensajes_recibir = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_mensajes_enviar = PTHREAD_MUTEX_INITIALIZER;

struct puntajes * puntaje;
struct idsSharedResources shmIds;

map<int,
Partida*> partidas;

ServerSocket * sSocket;

void* receptorConexiones(void * args) {
	CommunicationSocket * cSocket;
	Partida * partida;
	Felix * felix;
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	int idPartida = 0;
	string message;

	if (sSocket == 0) {
		cout << "No se ha establecido un socket de servidor" << endl;
		exit(1);
	}

	while (!stop) {
		cout << "Esperando cliente" << endl;
		cSocket = sSocket->Accept();
		//Recibo el id de partida, para sber contra quien va a jugar.
		cSocket->ReceiveBloq(buffer, sizeof(buffer));
		cout << "Recibo menasje de nuevo cliente: " << buffer << endl;
		message = buffer;
		idPartida = atoi(message.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str());
		felix = new Felix(cantVidas);
		if (partidaPendiente(idPartida)) {
			pthread_mutex_lock(&mutex_partidas);
			map<int, Partida*>::iterator iterator = partidas.find(idPartida);
			iterator->second->cSocket2 = cSocket;
			felix->fila = 0;
			felix->columna = EDIFICIO_COLUMNAS - 1;
			iterator->second->felix2 = felix;
			pthread_mutex_unlock(&mutex_partidas);
			cout << "Partida nro: " << idPartida << " completada" << endl;
		} else {
			partida = new Partida(idPartida);
			partida->cSocket1 = cSocket;
			felix->fila = 0;
			felix->columna = 0;
			partida->felix1 = felix;
			pthread_mutex_lock(&mutex_partidas);
			partidas[partida->id] = partida;
			pthread_mutex_unlock(&mutex_partidas);
		}
	}

	pthread_exit(0);
}

void * escuchaClientes(void * args) {
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	string message;
	int readData = 0;
	//TODO continuo polleo de la lista de partidas.
	while (!stop) {
		pthread_mutex_lock(&mutex_partidas);
		if (!partidas.empty()) {
			for (map<int, Partida *>::iterator it = partidas.begin(); it != partidas.end(); it++) {
				//escucho los clientes.
				if (it->second->cSocket1 != NULL) {
					readData = it->second->cSocket1->ReceiveNoBloq(buffer, sizeof(buffer));
					if (readData > 0) {
						message = buffer;
						Mensaje mensaje(JUGADOR_1, message, it->second);
						cola_mensajes_recibir.push(mensaje);
					}
				} else if (readData == 0) {
					delete (it->second->cSocket1);
					//TODO se desconecto el jugador1.
				}

				if (it->second->cSocket2 != NULL) {
					readData = it->second->cSocket2->ReceiveNoBloq(buffer, sizeof(buffer));
					if (readData > 0) {
						message = buffer;
						Mensaje mensaje(JUGADOR_2, message, it->second);
						cola_mensajes_recibir.push(mensaje);
					} else if (readData == 0) {
						delete (it->second->cSocket2);
						//TODO se desconecto el jugador2
					}
				}
			}
		}
		pthread_mutex_unlock(&mutex_partidas);
		usleep(POOLING_DEADTIME);
	}
	pthread_exit(0);
}

void* procesarMensajesCliente(void * args) {
	while (!stop) {
		if (!cola_mensajes_recibir.empty()) {
			pthread_mutex_lock(&mutex_cola_mensajes_recibir);
			Mensaje mensaje = cola_mensajes_recibir.front();
			cola_mensajes_recibir.pop();
			int codigo = atoi(mensaje.codigo.c_str());
			switch (codigo) {
			case CD_MOVIMIENTO_FELIX_I:
				caseMovimientoFelix(mensaje);
				break;
			case CD_PERDIDA_VIDA_I:
				casePerdidaVida(mensaje);
				break;
			case CD_VENTANA_ARREGLADA_I:
				break;
			case CD_VENTANA_ARREGLANDO_I:
				break;
			}
			pthread_mutex_unlock(&mutex_cola_mensajes_recibir);
		}
		usleep(POOLING_DEADTIME);
	}
	pthread_exit(0);
}

void* timer_thread(void * args)
{
	Timer timer;
	string message;

	while(!stop)
	{

		pthread_mutex_lock(&mutex_partidas);
		for(map<int,Partida*>::iterator it=partidas.begin();it!=partidas.end();it++)
		{
			message = timer.keepAlive();
			if(message.length()>0)
			{
				Mensaje mensaje(BROADCAST,message,it->second);
				Helper::encolar(&mensaje,&cola_mensajes_enviar,&mutex_cola_mensajes_enviar);
			}

			message = timer.ralph(it->second->edificio->nivel);
			if(message.length()>0)
			{
				Mensaje mensaje(BROADCAST,message,it->second);
				Helper::encolar(&mensaje,&cola_mensajes_enviar,&mutex_cola_mensajes_enviar);
			}

			message = timer.paloma(it->second->edificio->nivel);
			if(message.length()>0)
			{
				Mensaje mensaje(BROADCAST,message,it->second);
				Helper::encolar(&mensaje,&cola_mensajes_enviar,&mutex_cola_mensajes_enviar);
			}
		}
		pthread_mutex_unlock(&mutex_partidas);

		usleep(POOLING_DEADTIME);
	}
	pthread_exit(0);
}

void * sharedMemory(void * args) {
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
			if (kill(ppid, 0) == -1) {
				stop = true;
			}

			pthread_mutex_lock(&mutex_partidas);
			for (map<int, Partida*>::iterator it = partidas.begin(); it != partidas.end(); it++) {
				if (it->second->estado == ESTADO_FINALIZADO && !(it->second->cSocket1 == NULL && it->second->cSocket2 == NULL)) {
					//TODO usar semaforos para sincronizar.
					//TODO borrar la partida.
					puntaje->idJugador1 = it->second->felix1->id;
					puntaje->idJugador2 = it->second->felix2->id;
					puntaje->puntajeJugador1 = it->second->felix1->puntaje_parcial;
					puntaje->puntajeJugador2 = it->second->felix2->puntaje_parcial;
					puntaje->partidaFinalizadaOk = true;
					map<int, Partida*>::iterator auxIt = it;
					++it;
					partidas.erase(auxIt);
					//End usar semaforos para sincronizar.
					//Borrar partida.
				} else if (it->second->cSocket1 == NULL && it->second->cSocket2 == NULL) {
					//Borrar Partida.
					map<int, Partida*>::iterator auxIt = it;
					++it;
					partidas.erase(auxIt);
				}
			}
			pthread_mutex_unlock(&mutex_partidas);

			usleep(POOLING_DEADTIME);
		}
	} catch (const char * err) {
		cout << "Error inesperado al mapear la memoria compartida." << endl;
		exit(1);
	}
	pthread_exit(0);
}


void* enviarMensajesCliente(void * args) {
	char aux[LONGITUD_CODIGO + LONGITUD_CONTENIDO];

	while (!stop) {
		if (!cola_mensajes_enviar.empty()) {
			pthread_mutex_lock(&mutex_cola_mensajes_enviar);
			Mensaje mensaje = cola_mensajes_enviar.front();
			cola_mensajes_enviar.pop();
			pthread_mutex_unlock(&mutex_cola_mensajes_enviar);
			strcpy(aux, mensaje.codigo.c_str());
			strcat(aux, mensaje.mensaje.c_str());
			if (mensaje.jugador == BROADCAST) //BroadCast
					{
				if (mensaje.partida->cSocket1 != NULL) {
					mensaje.partida->cSocket1->SendNoBloq(aux, sizeof(aux));
				}
				if (mensaje.partida->cSocket2 != NULL) {
					mensaje.partida->cSocket2->SendNoBloq(aux, sizeof(aux));
				}
			} else if (mensaje.jugador == JUGADOR_2 && mensaje.partida->cSocket2 != NULL) //Jugador 2
			{
				mensaje.partida->cSocket2->SendNoBloq(aux, sizeof(aux));
			} else if (mensaje.jugador == JUGADOR_1 && mensaje.partida->cSocket1 != NULL) {
				mensaje.partida->cSocket1->SendNoBloq(aux, sizeof(aux));
			}
		}
		usleep(POOLING_DEADTIME);
	}
	pthread_exit(0);
}


bool partidaPendiente(int idPartida) {
	pthread_mutex_lock(&mutex_partidas);
	for (map<int, Partida*>::iterator it = partidas.begin(); it != partidas.end(); it++) {
		if (it->first == idPartida) {
			pthread_mutex_unlock(&mutex_partidas);
			return true;
		}
	}
	pthread_mutex_unlock(&mutex_partidas);

	return false;
}


void caseMovimientoFelix(Mensaje mensaje) {
	int columna = atoi(mensaje.mensaje.substr(5, 1).c_str());
	int fila = atoi(mensaje.mensaje.substr(6, 1).c_str());
	Partida * partidaJugador = mensaje.partida;
	Felix * felixJugador = 0;
	Edificio * edificioJugador = mensaje.partida->edificio;

	if (mensaje.jugador == JUGADOR_1)
		felixJugador = partidaJugador->felix1;
	else if (mensaje.jugador == JUGADOR_2)
		felixJugador = partidaJugador->felix2;

	if (!edificioJugador->ventanas[fila][columna].ocupado && fila >= 0 && columna >= 0 && fila < edificioJugador->filas && columna < edificioJugador->columnas && !(fila == 0 && columna == 2)) {
//Puede moverse ahi.
		edificioJugador->ventanas[felixJugador->fila][felixJugador->columna].ocupado = false;
		edificioJugador->ventanas[fila][columna].ocupado = true;
		felixJugador->fila = fila;
		felixJugador->columna = columna;

//Construyo nuevo mensaje a enviar.
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
//Envio mensaje al jugador1.
		Mensaje mensajeMovimiento = mensaje;
		if (felixJugador == mensaje.partida->felix1) {
			mensajeMovimiento.jugador = JUGADOR_1;
			mensajeMovimiento.mensaje = string(CD_MOVIMIENTO_FELIX) + string(Helper::fillMessage(aux1));
			Helper::encolar(&mensajeMovimiento, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensajeMovimiento.jugador = JUGADOR_2;
			mensajeMovimiento.mensaje = string(CD_MOVIMIENTO_FELIX) + string(Helper::fillMessage(aux2));
			Helper::encolar(&mensajeMovimiento, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
		} else if (felixJugador == mensaje.partida->felix2) {
			mensajeMovimiento.jugador = JUGADOR_1;
			mensajeMovimiento.mensaje = string(CD_MOVIMIENTO_FELIX) + string(Helper::fillMessage(aux2));
			Helper::encolar(&mensajeMovimiento, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensajeMovimiento.jugador = JUGADOR_2;
			mensajeMovimiento.mensaje = string(CD_MOVIMIENTO_FELIX) + string(Helper::fillMessage(aux1));
			Helper::encolar(&mensajeMovimiento, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
		}

	}
}

void casePerdidaVida(Mensaje mensaje) {
	Felix * felixJugador;
	if (mensaje.jugador == JUGADOR_1) {
		felixJugador = mensaje.partida->felix1;
	} else if (mensaje.jugador == JUGADOR_2) {
		felixJugador = mensaje.partida->felix2;
	}

	if (!felixJugador->perderVida()) //Perdio vida
	{
		if (felixJugador == mensaje.partida->felix1) {
			mensaje.mensaje = string(CD_PERDIDA_VIDA) + Helper::fillMessage("100");
			mensaje.jugador = JUGADOR_1;
			Helper::encolar(&mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.mensaje = string(CD_PERDIDA_VIDA) + Helper::fillMessage("200");
			mensaje.jugador = JUGADOR_2;
			Helper::encolar(&mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			felixJugador->mover(0, 0, mensaje.partida->edificio);
		} else if (felixJugador == mensaje.partida->felix2) {
			mensaje.mensaje = string(CD_PERDIDA_VIDA) + Helper::fillMessage("240");
			mensaje.jugador = JUGADOR_1;
			Helper::encolar(&mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.mensaje = string(CD_PERDIDA_VIDA) + Helper::fillMessage("140");
			mensaje.jugador = JUGADOR_2;
			Helper::encolar(&mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			felixJugador->mover(0, 4, mensaje.partida->edificio);
		}
	} else //Murio
	{
		if (felixJugador == mensaje.partida->felix1) {
			mensaje.mensaje = string(CD_PERDIO) + Helper::fillMessage("1");
			mensaje.jugador = JUGADOR_1;
			Helper::encolar(&mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.mensaje = string(CD_PERDIO) + Helper::fillMessage("2");
			mensaje.jugador = JUGADOR_2;
			Helper::encolar(&mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.partida->edificio->ventanas[felixJugador->fila][felixJugador->columna].ocupado = false;
			mensaje.partida->edificio->ventanas[0][0].ocupado = false;
		} else if (felixJugador == mensaje.partida->felix2) {
			mensaje.mensaje = string(CD_PERDIO) + Helper::fillMessage("2");
			mensaje.jugador = JUGADOR_1;
			Helper::encolar(&mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.mensaje = string(CD_PERDIO) + Helper::fillMessage("1");
			mensaje.jugador = JUGADOR_2;
			Helper::encolar(&mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.partida->edificio->ventanas[felixJugador->fila][felixJugador->columna].ocupado = false;
			mensaje.partida->edificio->ventanas[0][4].ocupado = false;
		}
	}
	if (mensaje.partida->felix1->cantidad_vidas <= 0 && mensaje.partida->felix2->cantidad_vidas <= 0) {
		mensaje.partida->estado = ESTADO_FINALIZADO;
	}
}
/*
 * Thread encargado de enviar los mensajes de los sucesos del juego no pertenecientes
 * a ninguno de los dos jugadores, siendo movimiento ralph, paloma,etc.
 */
/*void*
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



 * Thread encargado del envio de mensajes al jugador 1.

 void*
 sender1_thread(void * arguments) {
 while (stop == false && cliente1_conectado) {
 if (!sender1_queue.empty()) {
 //Lo que venga del timer y validator, se replica a ambos jugadores.
 string message;
 message = Helper::desencolar(&sender1_queue, &mutex_sender1);
 if (cliente1_conectado) {
 cSocket1->SendBloq(message.c_str(), message.length());
 }
 }

 usleep(POOLING_DEADTIME);
 }

 pthread_exit(0);
 }


 * Thread encargado del envio de mensajes al jugador2.

 void*
 sender2_thread(void * arguments) {
 while (stop == false && cliente2_conectado) {
 if (!sender2_queue.empty()) {
 string message;
 message = Helper::desencolar(&sender2_queue, &mutex_sender2);
 if (cliente2_conectado) {
 cSocket2->SendBloq(message.c_str(), message.length());
 }
 }
 usleep(POOLING_DEADTIME);
 }

 pthread_exit(0);
 }


 * Thread encargado de la recepcion de mensajes del jugador1 y detectar la
 * desconexion del jugador 1.

 void*
 receiver1_thread(void * fd) {
 char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
 int readDataCode;
 bzero(buffer, sizeof(buffer));

 while (stop == false && cliente1_conectado) {
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
 }
 usleep(POOLING_DEADTIME);
 }

 pthread_exit(0);
 }


 * Thread encargado de la recepcion de mensajes del jugador 2 y detectar la
 * desconexion del jugador 2

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


 * Thread encargado de validar las acciones de los clientes y devolver mensajes
 * de confirmacion.

 void *validator_thread(void * argument) {

 while (stop == false && (cliente1_conectado || cliente2_conectado)) {
 if (!receiver1_queue.empty()) {
 string message = receiver1_queue.front();
 receiver1_queue.pop();
 string scodigo = message.substr(0, LONGITUD_CODIGO);
 int codigo = atoi(scodigo.c_str());
 switch (codigo) {
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


 * Thread encargado de verificar si el torneo esta vivo y grabar los puntajes
 * finales de los clientes.

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
 cout << "Error inesperado: " << err <<" los puntos de esta partida no se veran reflejados en el torneo."<< endl;
 exit(1);
 }
 pthread_exit(0);
 }*/

/*
 * Caso de recepcion de mensaje de movimiento de felix.
 */

/*
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


 * Caso de recepcion de mensaje de perdida de vida.


 void casePerdidaVida(int jugador) {
 if (jugador == 1) {
 if (!felix1->perderVida()) {
 string message1(CD_PERDIDA_VIDA);
 string message2(CD_PERDIDA_VIDA);
 //TODO Corregir hardcodeo.
 message1.append(Helper::fillMessage("100"));
 message2.append(Helper::fillMessage("200"));
 Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
 Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
 felix1->mover(0, 0, edificio);
 } else {
 string message1(CD_PERDIO);
 string message2(CD_PERDIO);
 message1.append(Helper::fillMessage("1"));
 message2.append(Helper::fillMessage("2"));
 Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
 Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
 pthread_mutex_lock(&mutex_edificio);
 edificio->ventanas[felix1->fila][felix1->columna].ocupado = false;
 edificio->ventanas[0][0].ocupado = false;
 pthread_mutex_unlock(&mutex_edificio);
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
 felix2->mover(0, EDIFICIO_COLUMNAS - 1, edificio);
 } else {
 string message1(CD_PERDIO);
 string message2(CD_PERDIO);
 message1.append(Helper::fillMessage("2"));
 message2.append(Helper::fillMessage("1"));
 Helper::encolar(&message1, &sender1_queue, &mutex_sender1);
 Helper::encolar(&message2, &sender2_queue, &mutex_sender2);
 pthread_mutex_lock(&mutex_edificio);
 edificio->ventanas[felix2->fila][felix2->columna].ocupado = false;
 edificio->ventanas[0][EDIFICIO_COLUMNAS - 1].ocupado = false;
 pthread_mutex_unlock(&mutex_edificio);
 cliente2_jugando = false;
 }
 }
 }


 * Caso de recepcion de mensaje de ventana arreglada


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
 if (tramoFinalizado(edificio)) {
 cliente1_jugando = false;
 cliente2_jugando = false;
 }
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
 */
void SIGINT_Handler(int inum) {
	stop = true;
}

void liberarRecursos() {
	if (puntaje != NULL)
		shmdt(puntaje);
	shmctl(shmIds.shmId, IPC_RMID, 0);
	delete (sSocket);
}

/*
 string posicionInicial1() {
 string message(CD_POSICION_INICIAL);
 int fila = 0;
 int columna = 0;
 char cFila[2];*
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
 return result == 0;
 }

 */
