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
	atexit(liberarRecursos);

	unsigned int puerto=0;
	int cantClientes = 0;

	struct timeval timeout;

	fd_set fds;

	cSocket1 = NULL;
	cSocket2 = NULL;
	sSocket = NULL;
	felix1 = NULL;
	felix2 = NULL;
	edificio = NULL;
	semaforoPartida = NULL;
	semaforoTorneo = NULL;

	pthread_t thread_timer;
	pthread_t thread_receiver1;
	pthread_t thread_receiver2;
	pthread_t thread_sender1;
	pthread_t thread_sender2;
	pthread_t thread_validator1;
	pthread_t thread_validator2;
	pthread_t thread_sharedMemory;

	signal(SIGINT, SIGINT_Handler);
	signal(SIGTERM, SIGINT_Handler);

	srand(time(NULL));

	pid = getppid();

	/*
	 * Obtengo puertos y cantidad de vidas de felix por parametros.
	 */
	if (argc == 3) {
		puerto = atoi(argv[1]);
		cantVidas = atoi(argv[2]);
		//shmId = ftok("/bin/ls", puerto);
		shmId = ftok("/bin/ls", 666);
		if (shmId == -1) {
			cout << "Error al generar el shmId. " << endl;
			exit(1);
		}
	} else {
		exit(1);
	}

	sSocket = new ServerSocket(puerto);

	timeout.tv_sec = SERVERSOCKET_TIMEOUT;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(sSocket->ID, &fds);

	/*
	 * Espero la conexion de los clientes, dado tiempo SERVERSOCKET_TIMEDOUT
	 * si no se conectan, cierra lo partida.
	 */

	do {
		if (int response = select(sSocket->ID + 1, &fds, NULL, NULL, &timeout) > 0) {
			if (cSocket1 == NULL) {
				cSocket1 = sSocket->Accept();
			} else {
				cSocket2 = sSocket->Accept();
			}
			cantClientes++;
		} else if (response == 0) {
			//Timeout
			if (cSocket1 != NULL) {
				delete (cSocket1);
			}
			cout<<"Los clientes no se conectaron. TimeOut"<<endl;
			exit(1);
		}
	} while (cantClientes < 2);

	delete(sSocket);
	sSocket=NULL;

	/*
	 * Estoy a la espera de que los clientes me envien sus ids.
	 */
	/*while (felix1 == NULL || felix2 == NULL) {
		cSocket1->ReceiveNoBloq(buffer, sizeof(buffer));
		string message(buffer);
		if (message.substr(0, LONGITUD_CODIGO) == CD_ID_JUGADOR && felix1==NULL)
			felix1 = new Felix(cantVidas, atoi(message.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str()));
		cSocket2->ReceiveNoBloq(buffer, sizeof(buffer));
		string message2(buffer);
		if (message2.substr(0, LONGITUD_CODIGO) == CD_ID_JUGADOR && felix2==NULL)
			felix2 = new Felix(cantVidas, atoi(message2.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str()));
	}*/

	//Envio las posiciones iniciales a cada jugador.

/*
	cSocket1->SendBloq(posicionInicial1().c_str(), LONGITUD_CODIGO + LONGITUD_CONTENIDO);
	cSocket2->SendBloq(posicionInicial2().c_str(), LONGITUD_CODIGO + LONGITUD_CONTENIDO);
*/

	//Envio la cantidad de vida a cada jugador.
/*
	string message(CD_CANTIDAD_VIDAS);
	char aux[3];
	sprintf(aux,"%d",cantVidas);
	message.append(Helper::fillMessage(aux));
	cSocket1->SendBloq(message.c_str(),LONGITUD_CODIGO+LONGITUD_CONTENIDO);
	cSocket2->SendBloq(message.c_str(),LONGITUD_CODIGO+LONGITUD_CONTENIDO);
*/

	edificio = new Edificio(EDIFICIO_FILAS_1, EDIFICIO_COLUMNAS, 0);

	if(cSocket1!=NULL && cSocket2!=NULL)
		cout<<"Se conectaron ambos clientes"<<endl;
	else
		cout<<"no se conectaron todos"<<endl;

//Creo los threads.
	pthread_create(&thread_timer, NULL, timer_thread, NULL);
	pthread_create(&thread_receiver1, NULL, receiver1_thread, NULL);
	pthread_create(&thread_receiver2, NULL, receiver2_thread, NULL);
	pthread_create(&thread_sender1, NULL, sender1_thread, NULL);
	pthread_create(&thread_sender2, NULL, sender2_thread, NULL);
	pthread_create(&thread_validator1,NULL,validator1_thread,NULL);
	pthread_create(&thread_validator2, NULL, validator2_thread, NULL);
	pthread_create(&thread_sharedMemory, NULL, sharedMemory_thread, NULL);

	pthread_join(thread_sharedMemory, NULL);

	pthread_mutex_destroy(&mutex_receiver1);
	pthread_mutex_destroy(&mutex_receiver2);
	pthread_mutex_destroy(&mutex_sender1);
	pthread_mutex_destroy(&mutex_sender2);
	pthread_mutex_destroy(&mutex_edificio);

	cout << "Partida finalizada." << endl;
	return 0;
}

