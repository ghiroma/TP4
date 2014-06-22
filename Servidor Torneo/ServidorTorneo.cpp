#include "../Servidor Partida/FuncionesServidorPartida.h"
#include "Clases/ServerSocket.h"
#include "Clases/CommunicationSocket.h"
#include "Jugador.h"
#include "FuncionesServidorTorneo.h"
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <list>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>


using namespace std;
extern const char* CD_ID_JUGADOR;
extern const int LONGITUD_CONTENIDO;

pthread_mutex_t mutex_listJugadores = PTHREAD_MUTEX_INITIALIZER;
map<int, Jugador*> listJugadores;
unsigned int puerto;
int cantVidas = 0;

int main(int argc, char * argv[]) {
	cout << "Comienza servidor Torneo PID:" << getpid() << endl;
	string ip = "";
	int duracionTorneo = 0;
	int tiempoInmunidad = 0;
	int clientId = 0;
	pthread_t thTemporizadorTorneo;
	int resultThTemporizador;
	thTemporizador_data temporizacion;
	pthread_t thEstablecerPartidas;
	int resultThEstablecerPartidas;

	signal(SIGINT, SIGINT_Handler);

	//Obtener configuracion
	getConfiguration(&puerto, &ip, &duracionTorneo, &tiempoInmunidad, &cantVidas);
	if (puerto == 0 || ip.compare("") == 0 || duracionTorneo == 0 || tiempoInmunidad == 0 || cantVidas == 0) {
		cout << "Error al obtener configuracion." << endl;
		exit(1);
	}
	cout << "puerto: " << puerto << endl;

	/////////////////////////////////////////////////
	//CODIGO PARA PROBAR EL ASIGNADOR DE PARTIDAS
	clientId++;
	Jugador jugador1(clientId, "pedro 1", NULL);
	clientId++;
	Jugador jugador2(clientId, "carlos 2", NULL);
	clientId++;
	Jugador jugador3(clientId, "mati 3", NULL);
	clientId++;
	Jugador jugador4(clientId, "pablo 4", NULL);
	clientId++;
	Jugador jugador5(clientId, "martin 5", NULL);
	clientId++;
	Jugador jugador6(clientId, "fernando 6", NULL);

	agregarJugador(&jugador1);
	agregarJugador(&jugador2);
	agregarJugador(&jugador3);
	agregarJugador(&jugador4);
	agregarJugador(&jugador5);
	agregarJugador(&jugador6);

	//cout << "oponente para el jugador 1: " << (*listJugadores[1]).obtenerOponente(&listJugadores) << endl;

	/**/
	for (map<int, Jugador*>::iterator it = (listJugadores).begin(); it != (listJugadores).end(); it++) {
		cout << (*(*it).second).Nombre << " - partidas:" << endl;

		for (map<int, int>::iterator itmap = (*(*it).second).Partidas.begin(); itmap != (*(*it).second).Partidas.end(); ++itmap) {
			std::cout << itmap->first << " => " << itmap->second << '\n';
		}
	}
	/**/
	//////////////////////////////////////////////////////
	//Lanzar THREAD de establecer partidas
	temporizacion.timeIsUp = false;
	temporizacion.duracion = duracionTorneo;
	resultThEstablecerPartidas = pthread_create(&thEstablecerPartidas, NULL, establecerPartidas, (void *) &temporizacion);
	if (resultThEstablecerPartidas) {
		cout << "Error no se pudo crear el thread de Establecer Partidas," << endl;
		exit(1);
	}

	//Lanzar THREAD temporizador del torneo
	temporizacion.timeIsUp = false;
	temporizacion.duracion = duracionTorneo;
	temporizacion.thEstablecerPartidas = thEstablecerPartidas;
	resultThTemporizador = pthread_create(&thTemporizadorTorneo, NULL, temporizadorTorneo, (void *) &temporizacion);
	if (resultThTemporizador) {
		cout << "Error no se pudo crear el thread de temporizacion del torneo," << endl;
		exit(1);
	}

	//usleep(10000);
	exit(1);

	//Crear Socket del Servidor
	ServerSocket sSocket(puerto, (char *) ip.c_str());
	char * nombreJugador = NULL;
	while (!temporizacion.timeIsUp) {
		CommunicationSocket * cSocket = sSocket.Accept();
		cSocket->ReceiveBloq(nombreJugador, sizeof(nombreJugador));
		clientId++;
		agregarJugador(new Jugador(clientId, nombreJugador, cSocket));

		//mandarle el ID al Jugador
		char aux[LONGITUD_CONTENIDO];
		string message(CD_ID_JUGADOR);
		sprintf(aux, "%d", clientId);
		message.append(fillMessage(aux));
		cSocket->SendNoBloq(message.c_str(), sizeof(message.c_str()));
	}

	pthread_join(thEstablecerPartidas, NULL);
	pthread_exit(NULL);
	cout << "Fin proceso Servidor Torneo" << endl;
}

