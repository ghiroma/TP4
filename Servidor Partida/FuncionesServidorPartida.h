/*
 * Funciones.h
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include "Clases/CommunicationSocket.h"
#include "Clases/Edificio.h"
#include "Clases/Felix.h"
#include <time.h>
#include <queue>
#include <string>

using namespace std;

extern bool stop;
extern bool cliente1_conectado;
extern bool cliente2_conectado;
extern bool cliente1_jugando;
extern bool cliente2_jugando;

extern queue<string> receiver1_queue;
extern queue<string> receiver2_queue;
extern queue<string> sender1_queue;
extern queue<string> sender2_queue;

extern CommunicationSocket * cSocket1;
extern CommunicationSocket * cSocket2;

extern Felix *felix1;
extern Felix *felix2;

extern Edificio *edificio;

bool TimeDifference(int timeDifference, time_t startingTime);
void* timer_thread(void* argument);
void* receiver1_thread(void * argument);
void* receiver2_thread(void * argument);
void* sender1_thread(void * arguments);
void* sender2_thread(void * arguments);
void* validator_thread(void * argument);
void* keepAlive_thread(void * argument);
int randomRalphMovement();
int randomPaloma(int nivel);
char* randomTorta();
bool validateMovement(Felix * felix,int fila, int columna, Edificio * edificio);
bool validateWindowFix(int fila, int columna,Edificio * edificio);
bool validateLives(Felix * felix);
void SIGINT_Handler(int inum);

#endif /* FUNCIONES_H_ */
