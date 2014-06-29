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

using namespace std;

struct args_struct {
	int fd1;
	int fd2;
};

int main(int argc, char * argv[]) {

	cout << "Servidor Partida iniciado" << endl;

	int puerto;
	int cantVidas;
	int response = 0;
	int cantClientes = 0;

	struct idsSharedResources ids;
	struct timeval timeout;
	struct pollfd ufds[2];

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

	if (argc == 2) {
		cout << "Recibi la cantidad de parametros correcta" << endl;
		cout << "Parametro 1 " << argv[0] << endl;
		cout << "Parametro 2 " << argv[1] << endl;
		puerto = atoi(argv[0]);
		cantVidas = atoi(argv[1]);
		cout << "Numero de puerto " << puerto << endl;
		cout << "Cantidad de vidas " << cantVidas << endl;
		ids.shmId = ftok("/bin/ls", puerto);
		cout<<"SRV PARTIDA CREO SHMID: "<<ids.shmId<<endl;

		string nombreSemaforoPartida = "/" + intToString(puerto)+"_Partida";
		string nombreSemaforoTorneo = "/" + intToString(puerto)+"_Torneo";
		ids.semNamePartida =nombreSemaforoPartida.c_str();
		ids.semNameTorneo = nombreSemaforoTorneo.c_str();

		cout<<"SERVIDOR PARTIDA --> sem name partida"<<ids.semNamePartida<<endl;
		cout<<"SERVIDOR PARTIDA --> sem name torneo"<<ids.semNameTorneo<<endl;

		/*char * aux;

		sprintf(aux, "%d", puerto);
		strcpy(ids.semNameTorneo, "/");
		strcat(ids.semNameTorneo, aux);
		strcat(ids.semNameTorneo, "_");
		strcat(ids.semNameTorneo, "Torneo");
		cout<<"SERV PARTIDA nombre SEM Torneo"<<ids.semNameTorneo<<endl;

		strcpy(ids.semNamePartida, "/");
		strcat(ids.semNamePartida, aux);
		strcat(ids.semNamePartida, "_");
		strcat(ids.semNamePartida, "Partida");
		cout<<"SERV PARTIDA nombre SEM Partida"<<ids.semNamePartida<<endl;*/

	} else if (argc == 3) {
		cout << "Recibi la cantidad de parametros correcta" << endl;
		cout << "Parametro 1 " << argv[1] << endl;
		cout << "Parametro 2 " << argv[2] << endl;
		puerto = atoi(argv[1]);
		cantVidas = atoi(argv[2]);
		cout << "Numero de puerto " << puerto << endl;
		cout << "Cantidad de vidas " << cantVidas << endl;
		ids.shmId = ftok("/bin/ls", puerto);

		/*string nombreSemaforoPartida = "/" + intToString(puerto)+"_Partida";
		string nombreSemaforoTorneo = "/" + intToString(puerto)+"_Torneo";
		strcpy(ids.semNamePartida, nombreSemaforoPartida.c_str());
		strcpy(ids.semNameTorneo, nombreSemaforoTorneo.c_str());

		cout<<"SERVIDOR PARTIDA --> sem name partida"<<ids.semNamePartida<<endl;
		cout<<"SERVIDOR PARTIDA --> sem name torneo"<<ids.semNameTorneo<<endl;*/
	} else {
		//TODO Error y cerrar servidor partida porque faltan datos.
		cout << "No recibi cant correcta datos" << endl;
		puerto = 5556;
		cantVidas = 3;
	}

	cout << "Parametros seteados" << endl;
//TODO Temporalmente hago que el servidor de partida sea un servidor de torneo.
	ServerSocket sSocket(puerto);
	cout << "Puerto servidor creado" << endl;
	cSocket1 = NULL;
	cSocket2 = NULL;
	timeout.tv_sec = SERVERSOCKET_TIMEOUT;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(sSocket.ID, &fds);

	//TODO Hacer algo si estoy mucho tiempo en el accept y no se conecta nadie.
	do {
		cout << "Esperando IDS" << endl;
		if (int response = select(sSocket.ID + 1, &fds, NULL, NULL, &timeout)
				> 0) {
			if (cSocket1 == NULL) {
				cSocket1 = sSocket.Accept();
				cout << "Conexion recibida 1" << endl;
			} else {
				cSocket2 = sSocket.Accept();
				cout << "Conexion recibida 2" << endl;
			}
			cantClientes++;
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
	} while (cantClientes < 2);

	//TODO dependiendo del nivel abria que ver lo del edificio.
	//TODO Recibir los ids de los jugadores
	ufds[0].fd = cSocket1->ID;
	ufds[0].events = POLLIN;
	ufds[1].fd = cSocket2->ID;
	ufds[1].events = POLLIN;

	//Solo mandarme el codigo, nada mas.
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];

	while (felix1 == NULL && felix2 == NULL) {
		if (int result = poll(ufds, 2, CLIENT_ID_TIMEOUT) > 0) {
			if (ufds[0].revents & POLLIN) {
				cSocket1->ReceiveNoBloq(buffer, sizeof(buffer));
				string message(buffer);
				cout << "SERVIDOR PARTIDA: id1 " << buffer << endl;
				if (message.substr(0, LONGITUD_CODIGO) == CD_ID_JUGADOR)
					felix1 = new Felix(cantClientes,
							atoi(
									message.substr(LONGITUD_CODIGO,
											LONGITUD_CONTENIDO).c_str()));
			} else if (ufds[1].revents & POLLIN) {
				cSocket2->ReceiveNoBloq(buffer, sizeof(buffer));
				string message(buffer);
				cout << "SERVIDOR PARTIDA: id2 " << buffer << endl;
				if (message.substr(0, LONGITUD_CODIGO) == CD_ID_JUGADOR)
					felix2 = new Felix(cantClientes,
							atoi(
									message.substr(LONGITUD_CODIGO,
											LONGITUD_CONTENIDO).c_str()));
			}
		} else if (result == 0)		//timeout
				{
			//TODO Cerrar todo.
			cout << "No se ha recibido los ids del cliente" << endl;
			//Enviar mensaje al cliente de que se desconecte y vuelva al torneo.
			delete (cSocket1);
			delete (cSocket2);
			if (felix1 != NULL)
				delete (felix1);
			if (felix2 != NULL)
				delete (felix2);
			exit(1);
		}
	}

	edificio = new Edificio(EDIFICIO_FILAS_1, EDIFICIO_COLUMNAS, 0);

	//Creo los 4 thread.
	pthread_create(&thread_timer, NULL, timer_thread, NULL);
	pthread_create(&thread_receiver1, NULL, receiver1_thread, NULL);
	pthread_create(&thread_receiver2, NULL, receiver2_thread, NULL);
	pthread_create(&thread_sender1, NULL, sender1_thread, NULL);
	pthread_create(&thread_sender2, NULL, sender2_thread, NULL);
	pthread_create(&thread_validator, NULL, validator_thread, NULL);
	pthread_create(&thread_sharedMemory,NULL,sharedMemory_thread,(void *)&ids);

	pthread_join(thread_timer, NULL);
	pthread_join(thread_receiver1, NULL);
	pthread_join(thread_receiver2, NULL);
	pthread_join(thread_sender1, NULL);
	pthread_join(thread_sender2, NULL);
	pthread_join(thread_validator, NULL);
	pthread_join(thread_sharedMemory,NULL);

	//TODO finalizada la partida, enviar los puntajes actualizados.

	pthread_mutex_destroy(&mutex_receiver1);
	pthread_mutex_destroy(&mutex_receiver2);
	pthread_mutex_destroy(&mutex_sender1);
	pthread_mutex_destroy(&mutex_sender2);

	cout << "Se finalizara la partida" << endl;

	delete (cSocket1);
	delete (cSocket2);
	return 0;
}

