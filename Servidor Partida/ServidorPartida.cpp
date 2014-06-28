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
#include "Support/Estructuras.h"
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
	int idSM;
	int idSMKA;
	int response=0;
	int cantClientes=0;

	char * semaphore;

	struct shmIds ids;
	struct timeval timeout;

	fd_set fds;

	pthread_t thread_timer;
	pthread_t thread_receiver1;
	pthread_t thread_receiver2;
	pthread_t thread_sender1;
	pthread_t thread_sender2;
	pthread_t thread_validator;
	pthread_t thread_sharedMemory;

	signal(SIGINT, SIGINT_Handler);

	srand(time(NULL));

	if (argc == 5) {
		puerto = atoi(argv[1]);
		cantVidas = atoi(argv[2]);
		idSM = atoi(argv[3]);
		idSMKA = atoi(argv[4]);
		semaphore = argv[5];

		ids.semName = semaphore;
		ids.shmId = idSM;
		ids.shmKAId = idSMKA;

	} else {
		//TODO Error y cerrar servidor partida porque faltan datos.
		puerto = 5556;
		cantVidas = 3;
	}

//TODO Temporalmente hago que el servidor de partida sea un servidor de torneo.
	ServerSocket sSocket(puerto);
	cSocket1 = NULL;
	cSocket2 = NULL;
	timeout.tv_sec = SERVERSOCKET_TIMEOUT;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(sSocket.ID, &fds);
	while (true) {
		//TODO Hacer algo si estoy mucho tiempo en el accept y no se conecta nadie.
		do{
			if (int response = select(sSocket.ID + 1, &fds, NULL, NULL,
					&timeout) > 0) {
				if (cSocket1 == NULL) {
					cSocket1 = sSocket.Accept();
					cout << "Conexion recibida 1" << endl;
				} else {
					cSocket2 = sSocket.Accept();
					cout << "Conexion recibida 2" << endl;
				}
				cantClientes ++;
			} else if (response == 0) {
				//Timeout
				cout
						<< "El cliente no se ha conectado,no se pudo iniciar la partida."
						<< endl;
				if (cSocket1 != NULL) {
					delete (cSocket1);
				}
				//TODO Enviar mensaje al cliente de que su oponente no se conecto.
				exit(1);
			}
		}while(cantClientes<2);

		/*cSocket1 = sSocket.Accepxt();
		 cout << "Conexion 1 Recibida" << endl;
		 cSocket2 = sSocket.Accept();
		 cout<<"Conexion 2 Recibida"<<endl;*/
		//TODO dependiendo del nivel abria que ver lo del edificio.
		//Nivel 1:
		edificio = new Edificio(EDIFICIO_FILAS_1, EDIFICIO_COLUMNAS, 0);
		felix1 = new Felix(cantVidas);
		felix2 = new Felix(cantVidas);
		//Fin TODO Temporalmente.

		//Creo los 4 thread.
		pthread_create(&thread_timer, NULL, timer_thread, NULL);
		pthread_create(&thread_receiver1, NULL, receiver1_thread, NULL);
		pthread_create(&thread_receiver2, NULL, receiver2_thread, NULL);
		pthread_create(&thread_sender1, NULL, sender1_thread, NULL);
		pthread_create(&thread_sender2, NULL, sender2_thread, NULL);
		pthread_create(&thread_validator, NULL, validator_thread, NULL);
		//pthread_create(&thread_sharedMemory,NULL,sharedMemory_thread,(void *)&ids);

		pthread_join(thread_timer, NULL);
		pthread_join(thread_receiver1, NULL);
		pthread_join(thread_receiver2, NULL);
		pthread_join(thread_sender1, NULL);
		pthread_join(thread_sender2, NULL);
		pthread_join(thread_validator, NULL);
		//pthread_join(thread_sharedMemory,NULL);

		//TODO finalizada la partida, enviar los puntajes actualizados.

		pthread_mutex_destroy(&mutex_receiver1);
		pthread_mutex_destroy(&mutex_receiver2);
		//pthread_mutex_destroy(&mutex_puntajes);
		pthread_mutex_destroy(&mutex_sender1);
		pthread_mutex_destroy(&mutex_sender2);
	}

	delete (cSocket1);
	delete (cSocket2);
	return 0;
}

