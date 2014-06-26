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

pthread_mutex_t mutex_listJugadores = PTHREAD_MUTEX_INITIALIZER;
map<int,
Jugador*> listJugadores;
unsigned int puertoTorneo;
unsigned int puertoPartida;
int cantVidas = 0;

int main(int argc, char * argv[]) {
	/*
	 Semaforo s("/borrar1",1);
	 while(true)
	 {
	 Semaforo semer("/ff1",1);
	 s.setSem_t(semer.getSem_t());
	 int j;
	 for (j = 0; j < 10; ++j) {
	 semer.V();
	 }

	 s.V();
	 s.V();
	 cout<<semer.getValue()<<endl;
	 usleep(1000000);
	 break;
	 }
	 sleep(100);
	 exit(1);*/
	/*
	 map<int, string> Employees;
	 // 1) Assignment using array index notation
	 Employees[1] = "1";
	 Employees[2] = "2";
	 Employees[3] = "3";
	 Employees[4] = "4";
	 Employees[5] = "5";
	 cout << "Map size: " << Employees.size() << endl;

	 for (map<int, string>::iterator ii = Employees.begin(); ii != Employees.end(); ii++) {
	 cout << (*ii).first << ": " << (*ii).second << endl;
	 if ((*ii).first == 3 || (*ii).first==4) {
	 Employees.erase((*ii).first);
	 }
	 }
	 cout << "Map size: " << Employees.size() << endl;
	 for (map<int, string>::iterator ii = Employees.begin(); ii != Employees.end(); ++ii) {
	 cout << (*ii).first << ": " << (*ii).second << endl;
	 }
	 exit(1);*/

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

	//////////////////////////////////////////////////////

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

//hacer destroy de los pthread_mutex, semaforos, etc

	pthread_join(thEstablecerPartidas, NULL);
	pthread_mutex_destroy(&mutex_listJugadores);

	pthread_exit(NULL);
	cout << "Fin proceso Servidor Torneo" << endl;
}

