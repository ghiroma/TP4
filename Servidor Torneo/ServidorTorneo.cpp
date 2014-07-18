#include "Support/ConstantesServidorTorneo.h"
#include "../Servidor Partida/FuncionesServidorPartida.h"
#include "Clases/CommunicationSocket.h"
#include "Clases/Jugador.h"
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
#include "Clases/ServerSocket.h"
#include <sys/ipc.h>
#include <sys/shm.h>

using namespace std;

map<int, Jugador*> listJugadores;
unsigned int puertoServidorTorneo;
//unsigned int puertoServidorPartida;
int cantVidas = 0;
ServerSocket* sSocket;
//int cantidadDePuertosAbiertos = 0;
int idSHM;
pid_t pidServidorPartida;
SDL_Surface *screen, *background;
TTF_Font *font;
SDL_Rect posBackground;
extern SDL_Color colorNegro, colorBlanco;

int main(int argc, char * argv[]) {
	atexit(liberarRecursos);

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
	pthread_t thLecturaDeResultados;
	int resultThLecturaDeResultados;

	signal(SIGINT, SIG_Handler);
	signal(SIGCHLD, SIG_CHLD);

	//Obtener configuracion
	getConfiguration(&puertoServidorTorneo, &ip, &duracionTorneo, &tiempoInmunidad, &cantVidas);
	if (puertoServidorTorneo == 0 || duracionTorneo == 0 || tiempoInmunidad == 0 || cantVidas == 0) {
		cout << "Error al obtener configuracion." << endl;
		exit(1);
	}
	if (puertoServidorTorneo < MIN_PUERTO_REGISTRADO || puertoServidorTorneo >= MAX_PUERTO_REGISTRADO) {
		cout << "El puerto del servidor de partida no se encuentra en el rango permitido. Pruebe otro valor en el rango " << MIN_PUERTO_REGISTRADO << " - " << MAX_PUERTO_REGISTRADO << endl;
		exit(1);
	}

	//inicilizarMapPartidasActivas();

	//Creo el bloque de memoria compartida
	key_t key = ftok("/bin/ls", 666);
	if (key == -1) {
		cout << "Error al generar clave de memoria compartida" << endl;
		exit(1);
	}
	idSHM = shmget(key, sizeof(struct puntajesPartida) * 1, IPC_CREAT | PERMISOS_SHM);
	if (idSHM == -1) {
		cout << "Error al obtener memoria compartida" << endl;
		exit(1);
	}

	//inicializo el BLOQUE DE SHM
	puntajesPartida* resumenPartida = (struct puntajesPartida *) shmat(idSHM, (char *) 0, 0);
	resumenPartida->idJugador1 = -1;
	resumenPartida->idJugador2 = -1;
	resumenPartida->puntajeJugador1 = 0;
	resumenPartida->puntajeJugador2 = 0;
	resumenPartida->partidaFinalizadaOK = false;

	//Crear Socket del Servidor
	try {
		sSocket = new ServerSocket(puertoServidorTorneo);
	} catch (...) {
		cout << "No se pudo conectar al puerto solicitado. Pruebe otro." << endl;
		exit(1);
	}

	///Lanzar Servidor Partida
	/*if ((pidServidorPartida = fork()) == 0) {
		//Proceso hijo
		char auxCantVidas[2];
		sprintf(auxCantVidas, "%d", cantVidas);
		char auxPuertoServidorPartida[LONGITUD_CONTENIDO];
		sprintf(auxPuertoServidorPartida, "%d", puertoServidorPartida);
		char nombreEjecutable[100];
		strcpy(nombreEjecutable, "ServidorPartida");

		char *argumentos[] = { nombreEjecutable, auxPuertoServidorPartida, auxCantVidas, NULL };
		//execv("../Servidor Partida2/Debug/Servidor Partida2", argumentos);
		execv("../Servidor Partida/ServidorPartida", argumentos);
		cout << "ERROR al ejecutar execv Nueva Partida" << endl;
		exit(1);
	} else if (pidServidorPartida < 0) {
		cout << "Error al forkear" << endl;
		exit(1);
	}*/

	//Lanzar THREAD establecer partidas
	resultThEstablecerPartidas = pthread_create(&thEstablecerPartidas, NULL, establecerPartidas, NULL);
	if (resultThEstablecerPartidas) {
		cout << "Error no se pudo crear el thread de Establecer Partidas" << endl;
		exit(1);
	}

	//Lanzar THREAD actualizar lista de jugadores (KEEPALIVE)
	resultThkeepAliveJugadores = pthread_create(&thkeepAliveJugadores, NULL, keepAliveJugadores, (void*) NULL);
	if (resultThkeepAliveJugadores) {
		cout << "Error no se pudo crear el thread keepAliveJugadores" << endl;
		exit(1);
	}


	/////////////////// Preparacion de la parte Grafica /////////////////////
	//Colores
	colorNegro.r = colorNegro.g = colorNegro.b = 0;
	colorBlanco.r = colorBlanco.g = colorBlanco.b = 255;

	background = SDL_LoadBMP("Img/background.bmp");
	//verificamos si ha ocurrido algun error cargando la imagen
	if (background == NULL) {
		printf("Error en SDL_LoadBMP= %s\n", SDL_GetError());
		pthread_exit(NULL);
	}

	posBackground.x = 0;
	posBackground.y = 0;

	//Inicio modo video
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		printf("Error al iniciar SDL: %s\n", SDL_GetError());
		pthread_exit(NULL);
	}
	//Inicio modo texto grafico
	if (TTF_Init() < 0) {
		printf("Error al iniciar SDL_TTF\n");
		pthread_exit(NULL);
	}
	

	//Defino las propiedades de la pantalla del juego
	screen = SDL_SetVideoMode(ANCHO_PANTALLA_SERVIDOR, ALTO_PANTALLA_SERVIDOR, BPP_SERVIDOR, SDL_HWSURFACE);
	if (screen == NULL) {
		printf("Error estableciendo el modo de video: %s\n", SDL_GetError());
		pthread_exit(NULL);
	}

	//Seteo el titulo de la pantalla
	SDL_WM_SetCaption("Ralph Tournament SERVIDOR", NULL);

	//Cargo la fuente
	font = TTF_OpenFont("./Img/DejaVuSans.ttf", 24);
	if (font == NULL) {
		printf("Error abriendo la fuente ttf: %s\n", SDL_GetError());
		pthread_exit(NULL);
	}
	/////////////////////////////////////////////////////////////////////////

	//Lanzar THREAD ModoGrafico SDL
	modoGraficoData.duracion = duracionTorneo;
	resultThModoGrafico = pthread_create(&thModoGrafico, NULL, modoGrafico, (void*) &modoGraficoData);
	if (resultThModoGrafico) {
		cout << "Error no se pudo crear el thread de Modo Grafico" << endl;
		exit(1);
	}

	//Lanzar THREAD aceptar jugadores
	resultThAceptarJugadores = pthread_create(&thAceptarJugadores, NULL, aceptarJugadores, (void *) &ip);
	if (resultThAceptarJugadores) {
		cout << "Error no se pudo crear el thread de Aceptar Jugadores" << endl;
		exit(1);
	}

	//Lanzar THREAD temporizador del torneo
	temporizacion.duracion = duracionTorneo;
	temporizacion.thAceptarJugadores = thAceptarJugadores;
	temporizacion.thEstablecerPartidas = thEstablecerPartidas;
	resultThTemporizador = pthread_create(&thTemporizadorTorneo, NULL, temporizadorTorneo, (void *) &temporizacion);
	if (resultThTemporizador) {
		cout << "Error no se pudo crear el thread de temporizacion del torneo" << endl;
		exit(1);
	}

	//Lanzar THREAD lectura de resultados de las partidas
	resultThLecturaDeResultados = pthread_create(&thLecturaDeResultados, NULL, lecturaDeResultados, (void *) NULL);
	if (resultThLecturaDeResultados) {
		cout << "Error no se pudo crear el thread de lectura de resultados de las partidas" << endl;
		exit(1);
	}

	pthread_join(thModoGrafico, NULL);

	//mandar a cada cliente su puntaje y ranking
	cout<<"Mandando rankings"<<endl;
	mandarPuntajes();


	//Bloqueo en espera de que ingrese una tecla para cerrar la pantalla
	cout << "Ingrese una tecla para finalizar: ";
	getchar();
	exit(0);
}
