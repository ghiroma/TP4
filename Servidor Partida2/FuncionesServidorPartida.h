/*
 * Funciones.h
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#ifndef FUNCIONESSERVIDORPARTIDA_H_
#define FUNCIONESSERVIDORPARTIDA_H_

#include "Clases/CommunicationSocket.h"
#include "Clases/ServerSocket.h"
#include "Clases/Edificio.h"
#include "Clases/Felix.h"
#include "Clases/Partida.h"
#include "Clases/Mensaje.h"
#include <time.h>
#include <queue>
#include <string>
#include <semaphore.h>

using namespace std;

extern pid_t ppid;

extern key_t shmId;

extern ServerSocket * sSocket;

extern pthread_mutex_t mutex_edificio;
extern pthread_mutex_t mutex_partidas;
extern pthread_mutex_t mutex_cola_mensajes_recibir;
extern pthread_mutex_t mutex_cola_mensajes_enviar;

extern int cantVidas;

void* receptorConexiones(void * args);
void* escuchaClientes(void * args);
void* procesarMensajesCliente(void * args);
void* enviarMensajesCliente(void * args);
void* sharedMemory(void * args);
bool partidaPendiente(int idPartida);
void caseMovimientoFelix(Mensaje mensaje);
void casePerdidaVida(Mensaje mensaje);
void caseIdJugador(Mensaje mensaje);
void caseVentanaArreglada(Mensaje mensaje);
void caseVentanaArreglando(Mensaje mensaje);
void caseJugadorListo(Mensaje mensaje);
void* timer_thread(void* argument);
string posicionInicial1(Felix *);
string posicionInicial2(Felix *);
void SIGINT_Handler(int inum);
void liberarRecursos();
bool tramoFinalizado(Edificio * edificio);


#endif /* FUNCIONESSERVIDORPARTIDA_H_ */
