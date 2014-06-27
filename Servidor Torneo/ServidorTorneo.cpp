#include "Support/ConstantesServidorTorneo.h"
#include "../Servidor Partida/FuncionesServidorPartida.h"
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
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <list>

#include "Clases/Semaforo.h"

using namespace std;

extern pthread_mutex_t mutex_partidasActivas;
extern pthread_mutex_t mutex_todasLasPartidasFinalizadas;
extern bool todasLasPartidasFinalizadas;

pthread_mutex_t mutex_listJugadores = PTHREAD_MUTEX_INITIALIZER;
map<int,
Jugador*> listJugadores;
unsigned int puertoTorneo;
unsigned int puertoPartida;
int cantVidas = 0;

int main(int argc, char * argv[]) {
	cout << "Comienza servidor Torneo PID:" << getpid() << endl;
	string ip = "";
	int duracionTorneo = 0;
	int tiempoInmunidad = 0;
	//int clientId = 0;
	pthread_t thTemporizadorTorneo;
	int resultThTemporizador;
	thTemporizador_data temporizacion;
	pthread_t thEstablecerPartidas;
	int resultThEstablecerPartidas;
	pthread_t thAceptarJugadores;
	int resultThAceptarJugadores;
	pthread_t thActualizarListaJugadores;
	int resultThActualizarListaJugadores;
	pthread_t thModoGrafico;
	int resultThModoGrafico;
	thModoGrafico_data modoGraficoData;
	pthread_t thKeepAlive;
	int resultThKeepAlive;

	signal(SIGINT, SIGINT_Handler);

	//Obtener configuracion
	getConfiguration(&puertoTorneo, &ip, &duracionTorneo, &tiempoInmunidad, &cantVidas);
	if (puertoTorneo == 0 || ip.compare("") == 0 || duracionTorneo == 0 || tiempoInmunidad == 0 || cantVidas == 0) {
		cout << "Error al obtener configuracion." << endl;
		return 1;
	}
	puertoPartida = puertoTorneo;
	cout << "puertoTorneo: " << puertoTorneo << endl;

	//Lanzar THREAD establecer partidas
	resultThEstablecerPartidas = pthread_create(&thEstablecerPartidas, NULL, establecerPartidas, NULL);
	if (resultThEstablecerPartidas) {
		cout << "Error no se pudo crear el thread de Establecer Partidas" << endl;
		return 1;
	}

	//Lanzar THREAD actualizar lista de jugadores (KEEPALIVE)
	resultThActualizarListaJugadores = pthread_create(&thActualizarListaJugadores, NULL, actualizarListaJugadores, (void*) NULL);
	if (resultThActualizarListaJugadores) {
		cout << "Error no se pudo crear el thread de Actualizar Lista de Jugadores" << endl;
		return 1;
	}

	//Lanzar THREAD ModoGrafico
	modoGraficoData.duracion = duracionTorneo;
	resultThModoGrafico = pthread_create(&thModoGrafico, NULL, modoGrafico, (void*) &modoGraficoData);
	if (resultThModoGrafico) {
		cout << "Error no se pudo crear el thread de Modo Grafico" << endl;
		return 1;
	}

	//Lanzar THREAD aceptar jugadores
	resultThAceptarJugadores = pthread_create(&thAceptarJugadores, NULL, aceptarJugadores, (void *) &ip);
	if (resultThAceptarJugadores) {
		cout << "Error no se pudo crear el thread de Aceptar Jugadores" << endl;
		return 1;
	}

	//Lanzar THREAD KEEPALIVE (partidas)
	resultThKeepAlive = pthread_create(&thKeepAlive, NULL, keepAlive, (void *) NULL);
	if (resultThKeepAlive) {
		cout << "Error no se pudo crear el thread de KeepAlive" << endl;
		return 1;
	}

	//Lanzar THREAD temporizador del torneo
	temporizacion.duracion = duracionTorneo;
	temporizacion.thAceptarJugadores = thAceptarJugadores;
	temporizacion.thEstablecerPartidas = thEstablecerPartidas;
	resultThTemporizador = pthread_create(&thTemporizadorTorneo, NULL, temporizadorTorneo, (void *) &temporizacion);
	if (resultThTemporizador) {
		cout << "Error no se pudo crear el thread de temporizacion del torneo" << endl;
		return 1;
	}

	
	pthread_join(thEstablecerPartidas, NULL);

	//esperar que todas las partidas finalicen
	int cantPartidasActivas;
	while(true){
		cout << "mutex partidasActivas" << endl;
		pthread_mutex_lock(&mutex_partidasActivas);
		cantPartidasActivas = partidasActivas.size();
		pthread_mutex_unlock(&mutex_partidasActivas);
		cout << "unmutex partidasActivas" << endl;

		if (cantPartidasActivas == 0) {
			break;
		}	
		usleep(1000000);
	}
	pthread_mutex_lock(&mutex_todasLasPartidasFinalizadas);
	cout << "mutex main todasLasPartidasFinalizadas" << endl;
	todasLasPartidasFinalizadas = true;
	pthread_mutex_unlock(&mutex_todasLasPartidasFinalizadas);
	cout << "unmutex main todasLasPartidasFinalizadas" << endl;
				

	///////////////////
	//ver si hace falta??????????
	//el tiempo del Torneo llego a su fin, informar a cada cliente
	/*pthread_mutex_lock(&mutex_listJugadores);
	 for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
	 string message(CD_FIN_TORNEO);
	 message.append(fillMessage("1"));
	 it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
	 }
	 pthread_mutex_unlock(&mutex_listJugadores);*/

	//mandar a cada cliente su puntaje y ranking
	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		string message(CD_FIN_TORNEO);
		message.append(fillMessage("1"));
		it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
	}
	pthread_mutex_unlock(&mutex_listJugadores);

	//hacer destroy de los pthread_mutex, semaforos, etc sockets
	///
	//
	pthread_cancel(thKeepAlive);
	pthread_mutex_destroy(&mutex_listJugadores);
	pthread_mutex_destroy(&mutex_puertoPartida);
	pthread_mutex_destroy(&mutex_timeIsUp);
	
	pthread_exit(NULL);
	//bloqueo en espera de que ingrese una tecla para cerrar la pantalla
	getchar();
	cout << "Fin proceso Servidor Torneo" << endl;
}

