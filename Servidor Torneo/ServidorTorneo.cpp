/*
 * ServidorTorneo.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "Clases/ServerSocket.h"
#include "Clases/CommunicationSocket.h"
#include <string.h>
#include "Funciones.h"
#include "Jugador.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <list>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

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

	signal(SIGINT, SIGINT_Handler);

	getConfiguration(&puerto, &ip, &duracionTorneo, &tiempoInmunidad, &cantVidas);
	if (puerto == 0 || ip.compare("") == 0 || duracionTorneo == 0 || tiempoInmunidad == 0 || cantVidas == 0) {
		cout << "Error al obtener configuracion." << endl;
	}

	ServerSocket sSocket(puerto, (char *) ip.c_str());

	while (true) {
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

}


