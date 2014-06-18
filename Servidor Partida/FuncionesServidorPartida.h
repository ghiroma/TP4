/*
 * Funciones.h
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include "Clases/CommunicationSocket.h"
#include "Edificio.h"
#include <time.h>
#include <queue>
#include <string>

using namespace std;

extern bool stop;

extern queue<string> receiver1_queue;
extern queue<string> receiver2_queue;
extern queue<string> sender_queue;

extern CommunicationSocket * cSocket1;
extern CommunicationSocket * cSocket2;

bool TimeDifference(int timeDifference, time_t startingTime);
void* timer_thread(void* argument);
void* receiver1_thread(void * argument);
void* receiver2_thread(void * argument);
void* sender_thread(void * arguments);
void* validator_thread(void * argument);
void* keepAlive_thread(void * argument);
bool validateMovement(int fila, int columna, Edificio * edificio);
bool validateWindowFix(int fila, int columna,Edificio * edificio);
void SIGINT_Handler(int inum);

#endif /* FUNCIONES_H_ */
