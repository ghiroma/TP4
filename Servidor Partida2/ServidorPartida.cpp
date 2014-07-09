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

int main(int argc, char * argv[]) {

	pthread_t thread_receptorConexiones;

	atexit(liberarRecursos);

	unsigned int puerto;

	signal(SIGINT, SIGINT_Handler);
	signal(SIGTERM, SIGINT_Handler);

	srand(time(NULL));

	ppid = getppid();

	/*
	 * Obtengo puertos y cantidad de vidas de felix por parametros.
	 */
	if (argc == 3) {
		puerto = atoi(argv[1]);
		cantVidas = atoi(argv[2]);
		shmIds.shmId = ftok("/bin/ls", puerto);
		if (shmIds.shmId == -1) {
			cout << "Error al generar el shmId. " << endl;
			exit(1);
		}
	} else {
		cout << "Cantidad de parametros invalida." << endl;
		exit(1);
	}

	try {
		sSocket = new ServerSocket(puerto);
	} catch (const char* err) {
		cout << "Error al querer conectar al puerto: " << puerto << endl;
	}

	pthread_create(&thread_receptorConexiones,NULL,receptorConexiones,NULL);

	pthread_join(thread_receptorConexiones,NULL);

	cout << "Partida finalizada." << endl;
	return 0;
}

