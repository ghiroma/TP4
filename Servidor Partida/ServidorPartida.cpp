/*
 * ServidorPartida.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "Clases/Semaforo.h"
#include "Clases/CommunicationSocket.h"
#include "Clases/ServerSocket.h"
#include "FuncionesServidorPartida.h"
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>

using namespace std;

struct args_struct {
	int fd1;
	int fd2;
};

int main(int argc, char * argv[]) {
	int fdJugador1;
	int fdJugador2;

	struct args_struct args;

	pthread_t thread_timer;
	pthread_t thread_receiver1;
	pthread_t thread_receiver2;
	pthread_t thread_sender;
	pthread_t thread_validator;

	signal(SIGINT, SIGINT_Handler);

	srand(time(NULL));

	//TODO Temporalmente hago que el servidor de partida sea un servidor de torneo.
	ServerSocket sSocket(5555);

	while (true) {
		cSocket1 = sSocket.Accept();
		//cSocket2 = sSocket.Accept();
		cout << "Conexion Recibida" << endl;

		//Fin TODO Temporalmente.
		//Recibo todos los datos del servidortorneo.

		//Creo los 4 thread.
		pthread_create(&thread_timer, NULL, timer_thread, NULL);
		pthread_create(&thread_receiver1, NULL, receiver1_thread,
				(void *) &cSocket1->ID);
		//pthread_create(&thread_receiver2,NULL,receiver2_thread,NULL);
		pthread_create(&thread_sender, NULL, sender_thread, NULL);
		pthread_create(&thread_validator, NULL, validator_thread, NULL);
		cout<<"Threads creados"<<endl;

		cSocket1->SendBloq("Ack", sizeof("Ack"));

		pthread_join(thread_timer, NULL);
		pthread_join(thread_receiver1, NULL);
		//pthread_join(thread_receiver2,NULL);
		pthread_join(thread_sender, NULL);
		pthread_join(thread_validator,NULL);
	}

	delete (cSocket1);
	//delete(cSocket2);
	return 0;
	//TODO envio puntaje al padre.
}

