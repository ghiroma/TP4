/*
 * ServidorPartida.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "Clases/Semaforo.h"
#include "Clases/CommunicationSocket.h"
#include "Clases/ServerSocket.h"
#include "Clases/Felix.h"
#include "Clases/Edificio.h"
#include "Support/Constantes.h"
#include "FuncionesServidorPartida.h"
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>

using namespace std;

struct args_struct {
	int fd1;
	int fd2;
};

int main(int argc, char * argv[]) {
	int puerto;
	int cantVidas;

	struct args_struct args;

	pthread_t thread_timer;
	pthread_t thread_receiver1;
	pthread_t thread_receiver2;
	pthread_t thread_sender1;
	pthread_t thread_sender2;
	pthread_t thread_validator;

	signal(SIGINT, SIGINT_Handler);

	srand(time(NULL));

	if (argc == 3) {
		puerto = atoi(argv[1]);
		cantVidas = atoi(argv[2]);
	} else {
		puerto = 5556;
		cantVidas = 3;
	}

//TODO Temporalmente hago que el servidor de partida sea un servidor de torneo.
	ServerSocket sSocket(puerto);

	while (true) {
		cSocket1 = sSocket.Accept();
		cout << "Conexion 1 Recibida" << endl;
		//cSocket2 = sSocket.Accept();
		//TODO dependiendo del nivel abria que ver lo del edificio.

		//Nivel 1:
		edificio = new Edificio(EDIFICIO_FILAS_1, EDIFICIO_COLUMNAS, 0);
		felix1 = new Felix(cantVidas);
		felix2 = new Felix(cantVidas);
		//Fin TODO Temporalmente.

		//Creo los 4 thread.
		pthread_create(&thread_timer, NULL, timer_thread, NULL);
		pthread_create(&thread_receiver1, NULL, receiver1_thread,
				(void *) &cSocket1->ID);
		//pthread_create(&thread_receiver2,NULL,receiver2_thread,NULL);
		pthread_create(&thread_sender1, NULL, sender1_thread, NULL);
		//pthread_create(&thread_sender2,NULL,sender2_thread,NULL);
		pthread_create(&thread_validator, NULL, validator_thread, NULL);

		pthread_join(thread_timer, NULL);
		pthread_join(thread_receiver1, NULL);
		//pthread_join(thread_receiver2,NULL);
		pthread_join(thread_sender1, NULL);
		//pthread_join(thread_sender2,NULL);
		pthread_join(thread_validator, NULL);
	}

	delete (cSocket1);
//delete(cSocket2);
	//TODO envio puntaje al padre.
	return 0;
}

