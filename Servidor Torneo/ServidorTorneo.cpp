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

map<int, Jugador*> listJugadores;
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
	pthread_t thkeepAliveJugadores;
	int resultThkeepAliveJugadores;
	pthread_t thModoGrafico;
	int resultThModoGrafico;
	thModoGrafico_data modoGraficoData;
	pthread_t thKeepAlive;
	int resultThKeepAlive;

	signal(SIGINT, SIG_Handler);
	signal(SIGTERM, SIG_Handler);

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
	resultThkeepAliveJugadores = pthread_create(&thkeepAliveJugadores, NULL, keepAliveJugadores, (void*) NULL);
	if (resultThkeepAliveJugadores) {
		cout << "Error no se pudo crear el thread keepAliveJugadores" << endl;
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
	/*
	 //Lanzar THREAD KEEPALIVE (partidas)
	 resultThKeepAlive = pthread_create(&thKeepAlive, NULL, keepAlive, (void *) NULL);
	 if (resultThKeepAlive) {
	 cout << "Error no se pudo crear el thread de KeepAlive" << endl;
	 return 1;
	 }*/

	//Lanzar THREAD temporizador del torneo
	temporizacion.duracion = duracionTorneo;
	temporizacion.thAceptarJugadores = thAceptarJugadores;
	temporizacion.thEstablecerPartidas = thEstablecerPartidas;
	resultThTemporizador = pthread_create(&thTemporizadorTorneo, NULL, temporizadorTorneo, (void *) &temporizacion);
	if (resultThTemporizador) {
		cout << "Error no se pudo crear el thread de temporizacion del torneo" << endl;
		return 1;
	}



	//pthread_join(thTemporizadorTorneo, NULL);//no hace falta
	//pthread_join(thEstablecerPartidas, NULL);//no hace falta
	pthread_join(thModoGrafico, NULL);
	pthread_join(thkeepAliveJugadores, NULL);

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
	mandarPuntajes();

	//bloqueo en espera de que ingrese una tecla para cerrar la pantalla
	cout << "Ingrese una tecla para finalizar: ";
	getchar();
	liberarRecursos();
	cout << "Fin proceso Servidor Torneo" << endl;
	//exit(0);
}
