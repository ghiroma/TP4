/*
 * ServidorTorneo.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */
#include "Clases/ServerSocket.h"
#include "Clases/CommunicationSocket.h"
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <list>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "Funciones.h"
#include "Jugador.h"

struct threadTemporizador_data {
	bool timeIsUp;
	int duracion;
};
pthread_mutex_t mutex_timeIsUp = PTHREAD_MUTEX_INITIALIZER;
//Controla el tiempo que debe durar el torneo
void* temporizadorTorneo(void* data) {
	struct threadTemporizador_data *torneo;
	torneo = (struct threadTemporizador_data *) data;

	sleep(torneo->duracion * 60);
	pthread_mutex_lock(&mutex_timeIsUp);
	torneo->timeIsUp = true;
	pthread_mutex_unlock(&mutex_timeIsUp);

	pthread_exit(NULL);
}
using namespace std;

int main(int argc, char * argv[]) {
	unsigned int puerto = 0;
	string ip = "";
	int duracionTorneo = 0;
	int tiempoInmunidad = 0;
	int cantVidas = 0;
	int clientId = 1;
	pid_t pid;
	list<Jugador> listJugadores;
	pthread_t thTemporizadorTorneo;
	int resultThread;
	threadTemporizador_data temporizacion;

	signal(SIGINT, SIGINT_Handler);

	//Obtener configuracion
	getConfiguration(&puerto, &ip, &duracionTorneo, &tiempoInmunidad, &cantVidas);
	if (puerto == 0 || ip.compare("") == 0 || duracionTorneo == 0 || tiempoInmunidad == 0 || cantVidas == 0) {
		cout << "Error al obtener configuracion." << endl;
		exit(1);
	}

	//Lanzar temporizador del torneo
	temporizacion.timeIsUp = false;
	temporizacion.duracion = duracionTorneo;
	resultThread = pthread_create(&thTemporizadorTorneo, NULL, temporizadorTorneo, (void *) &temporizacion);
	if (resultThread) {
		cout << "Error no se pudo crear el thread de temporizacion del torneo," << resultThread << endl;
		exit(1);
	}


	/////////////////////////////////////////////////
	//CODIGO PARA PROBAR EL ASIGNADOR DE PARTIDAS
	Jugador jugador1(clientId, "pedro 1");
	jugador1.agregarJugador(2);
	jugador1.agregarJugador(3);
	listJugadores.push_back(jugador1);
	clientId++;

	Jugador jugador2(clientId, "carlos 2");
	jugador2.agregarJugador(1);
	jugador2.agregarJugador(3);
	listJugadores.push_back(jugador2);
	clientId++;

	Jugador jugador3(clientId, "mati 3");
	jugador3.agregarJugador(1);
	jugador3.agregarJugador(2);
	listJugadores.push_back(jugador3);
	clientId++;

	listJugadores.front().obtenerOponente(&listJugadores);

	for (list<Jugador>::iterator it = (listJugadores).begin(); it != (listJugadores).end(); it++) {

		cout << (*it).Nombre << " - partidas:" << endl;

		for (map<int, int>::iterator itmap = (*it).Partidas.begin(); itmap != (*it).Partidas.end(); ++itmap) {
			std::cout << itmap->first << " => " << itmap->second << '\n';
		}
	}

	exit(1);
	//////////////////////////////////////////////////////

	//Crear Socket del Servidor
	ServerSocket sSocket(puerto, (char *) ip.c_str());

	while (!temporizacion.timeIsUp) {
		CommunicationSocket * cSocket = sSocket.Accept();

		//Verifico si es un jugador nuevo, si es asi lo agrego a mi lista de jugadores y actualizo a todos los jugadores.
		//El cliente tambien podria tener la clase jugador, y tener siempre sus datos ahi, asi de paso puede consultarlos.

		pid = fork();
		if ((pid) == 0) {
			//Proceso hijo. Hacer exec

		} else if (pid < 0) {
			//Hubo error
			cout << "Error al forkear" << endl;
		} else {
			//Soy el padre.
			delete (cSocket);
			string nombreTemp = "juancito"; //TODO Sacar el hardcodeo y obtener nombre.
			Jugador jugador(clientId, nombreTemp);
			listJugadores.push_back(jugador);
			clientId++;
		}
	}

	pthread_exit(NULL);
}

