/*
 * Funciones.h
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#ifndef FUNCIONESSERVIDORPARTIDA_H_
#define FUNCIONESSERVIDORPARTIDA_H_

#include "Clases/CommunicationSocket.h"
#include "Clases/Edificio.h"
#include "Clases/Felix.h"
#include <time.h>
#include <queue>
#include <string>
#include <semaphore.h>

using namespace std;

extern pid_t pid;

extern CommunicationSocket * cSocket1;
extern CommunicationSocket * cSocket2;

extern Felix * felix1;
extern Felix * felix2;

extern Edificio *edificio;

extern pthread_mutex_t mutex_receiver1;
extern pthread_mutex_t mutex_receiver2;
extern pthread_mutex_t mutex_sender1;
extern pthread_mutex_t mutex_sender2;

extern struct idsSharedResources shmIds;

void* timer_thread(void* argument);
void* receiver1_thread(void * argument);
void* receiver2_thread(void * argument);
void* sender1_thread(void * arguments);
void* sender2_thread(void * arguments);
void* validator_thread(void * argument);
void* sharedMemory_thread(void * arguments);
void caseMovimientoFelix (int jugador, string *message);
void casePerdidaVida(int nroJugador);
void caseVentanaArreglada(int nroJugador);
void SIGINT_Handler(int inum);
void liberarRecursos();

#endif /* FUNCIONESSERVIDORPARTIDA_H_ */
