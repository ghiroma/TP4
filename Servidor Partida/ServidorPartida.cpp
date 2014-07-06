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
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/shm.h>
#include <errno.h>

using namespace std;

struct args_struct {
	int fd1;
	int fd2;
};

int main(int argc, char * argv[]) {
	unsigned int puerto;
	int cantVidas;
	int cantClientes = 0;

	struct timeval timeout;

	fd_set fds;

	cSocket1 = NULL;
	cSocket2 = NULL;

	felix1 = NULL;
	felix2 = NULL;

	pthread_t thread_timer;
	pthread_t thread_receiver1;
	pthread_t thread_receiver2;
	pthread_t thread_sender1;
	pthread_t thread_sender2;
	pthread_t thread_validator;
	pthread_t thread_sharedMemory;

	signal(SIGINT, SIGINT_Handler);
	signal(SIGTERM,SIGINT_Handler);

	srand(time(NULL));

	pid = getpid();
	cout<<"PARTIDA -> getppid() = "<<getppid<<endl;
	cout<<"PARTIDA -> getpid() = "<<getpid()<<endl;

	/*
	 * Obtengo puertos y cantidad de vidas de felix por parametros.
	 */
	if (argc == 2) {
		puerto = atoi(argv[0]);
		cantVidas = atoi(argv[1]);
		shmIds.shmId = ftok("/bin/ls", puerto);
		if (shmIds.shmId == -1) {
			cout << "Error al generar el shmId el error es: " << endl;
			/* if (errno == EACCES)
			 cout << "Error de permisos" << endl;
			 if (errno == ENOENT)
			 cout << "Path inexistente" << endl;*/

			exit(1);
		}
	} else {
		exit(1);
	}

	ServerSocket sSocket(puerto);

	timeout.tv_sec = SERVERSOCKET_TIMEOUT;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(sSocket.ID, &fds);

	/*
	 * Espero la conexion de los clientes, dado tiempo SERVERSOCKET_TIMEDOUT
	 * si no se conectan, cierra lo partida.
	 */

	cout<<"espero conexion de los clientes"<<endl;
	do {
		if (int response = select(sSocket.ID + 1, &fds, NULL, NULL, &timeout)
				> 0) {
			if (cSocket1 == NULL) {
				cSocket1 = sSocket.Accept();
			} else {
				cSocket2 = sSocket.Accept();
			}
			cantClientes++;
		} else if (response == 0) {
			//Timeout
			if (cSocket1 != NULL) {
				delete (cSocket1);
			}
			exit(1);
		}
	} while (cantClientes < 2);

	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];

	/*
	 * Estoy a la espera de que los clientes me envien sus ids.
	 */
	cout<<"Espero IDs de los clientes"<<endl;
	while (felix1 == NULL && felix2 == NULL) {
		cSocket1->ReceiveNoBloq(buffer, sizeof(buffer));
		string message(buffer);
		if (message.substr(0, LONGITUD_CODIGO) == CD_ID_JUGADOR)
			felix1 =
					new Felix(cantClientes,
							atoi(
									message.substr(LONGITUD_CODIGO,
											LONGITUD_CONTENIDO).c_str()));
		cSocket2->ReceiveBloq(buffer, sizeof(buffer));
		string message2(buffer);
		if (message2.substr(0, LONGITUD_CODIGO) == CD_ID_JUGADOR)
			felix2 =
					new Felix(cantClientes,
							atoi(
									message2.substr(LONGITUD_CODIGO,
											LONGITUD_CONTENIDO).c_str()));
	}

	//Envio las posiciones iniciales a cada jugador.
	cout<<"Envio las posiciones iniciales a cada jugador"<<endl;
	cSocket1->SendNoBloq(posicionInicial1().c_str(),LONGITUD_CODIGO + LONGITUD_CONTENIDO);
	cSocket2->SendNoBloq(posicionInicial2().c_str(),LONGITUD_CODIGO + LONGITUD_CONTENIDO);

	edificio = new Edificio(EDIFICIO_FILAS_1, EDIFICIO_COLUMNAS, 0);

//Creo los 4 thread.
	pthread_create(&thread_timer, NULL, timer_thread, NULL);
	pthread_create(&thread_receiver1, NULL, receiver1_thread, NULL);
	pthread_create(&thread_receiver2, NULL, receiver2_thread, NULL);
	pthread_create(&thread_sender1, NULL, sender1_thread, NULL);
	pthread_create(&thread_sender2, NULL, sender2_thread, NULL);
	pthread_create(&thread_validator, NULL, validator_thread, NULL);
	pthread_create(&thread_sharedMemory, NULL, sharedMemory_thread, NULL);

	pthread_join(thread_timer, NULL);
	pthread_join(thread_receiver1, NULL);
	pthread_join(thread_receiver2, NULL);
	pthread_join(thread_sender1, NULL);
	pthread_join(thread_sender2, NULL);
	pthread_join(thread_validator, NULL);
	pthread_join(thread_sharedMemory, NULL);

//TODO finalizada la partida, enviar los puntajes actualizados.

	pthread_mutex_destroy(&mutex_receiver1);
	pthread_mutex_destroy(&mutex_receiver2);
	pthread_mutex_destroy(&mutex_sender1);
	pthread_mutex_destroy(&mutex_sender2);

	atexit(liberarRecursos);
	return 0;
}

