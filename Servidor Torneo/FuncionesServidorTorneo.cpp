#include "Support/ConstantesServidorTorneo.h"
#include "Clases/ServerSocket.h"
#include "FuncionesServidorTorneo.h"
#include "./Clases/Semaforo.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <stdlib.h>
#include <sstream>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

extern unsigned int puertoTorneo;
bool timeIsUp = false;
Semaforo sem_inicializarTemporizador((char*) "/sem_inicializarTemporizador", 0);
Semaforo sem_jugadoresKeepAlive((char*) "/sem_jugadoresKeepAlive", 1);
pthread_mutex_t mutex_puertoPartida = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_timeIsUp = PTHREAD_MUTEX_INITIALIZER;
using namespace std;

/**
 * Obtener la configuracion inicial del torneo
 */
void getConfiguration(unsigned int* port, string* ip, int* duracionTorneo, int* tiempoInmunidad, int* cantVidas) {
	string content;
	string line;
	fstream configFile("configFile", fstream::in | fstream::out);
	while (getline(configFile, line)) {
		if (line.find("Puerto") == 0) {
			int pos = line.find(":");
			string sport = line.substr(pos + 1, line.length());
			*port = atoi(sport.c_str());
		} else if (line.find("IP") == 0) {
			int pos = line.find(":");
			string auxip = line.substr(pos + 1, line.length());
			*ip = auxip.c_str();
		} else if (line.find("Duracion del Torneo[min]") == 0) {
			int pos = line.find(":");
			string sduracionTorneo = line.substr(pos + 1, line.length());
			*duracionTorneo = atoi(sduracionTorneo.c_str());
		} else if (line.find("Tiempo de Inmunidad[seg]") == 0) {
			int pos = line.find(":");
			string stiempoInmunidad = line.substr(pos + 1, line.length());
			*tiempoInmunidad = atoi(stiempoInmunidad.c_str());
		} else if (line.find("Cantidad de Vidas") == 0) {
			int pos = line.find(":");
			string scantVida = line.substr(pos + 1, line.length());
			*cantVidas = atoi(scantVida.c_str());
		}
	}
}

void SIGINT_Handler(int inum) {

}

/**
 * Cuando ingresa un nuevo jugador al sistema.
 * Lo pongo en la lista de jugadores y lo sumo a la lista de Partidas de cada jugador
 */
void agregarJugador(Jugador* nuevoJugador) {
	pthread_mutex_lock(&mutex_listJugadores);
	//le inicializo la lista de partidas al nuevo jugador
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		nuevoJugador->agregarOponente(it->second->Id);
	}
	//lo agrego a la lista de jugadores
	listJugadores[nuevoJugador->Id] = nuevoJugador;

	//para cada jugador le agrego el nuevo jugador a su lista de oponentes
	for (map<int, Jugador*>::iterator it2 = listJugadores.begin(); it2 != listJugadores.end(); it2++) {
		if (it2->second->Id != nuevoJugador->Id) {
			it2->second->agregarOponente(nuevoJugador->Id);
		}
	}
	pthread_mutex_unlock(&mutex_listJugadores);
}

/**
 * Eliminar un jugador de la lista cuando abandona el torneo
 */
void quitarJugador(int id) {
	//pthread_mutex_lock(&mutex_listJugadores);

	//para cada participante le quito de su lista el jugador que se da de baja
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		it->second->quitarJugador(id);
	}
	//lo quito de la lista de jugadores del torneo
	listJugadores.erase(id);

	//pthread_mutex_unlock(&mutex_listJugadores);
}

/**
 * Obtener un nuevo puerto para asginarlo a una nueva partida
 */
unsigned int getNewPort() {
	int nuevoPuerto;
	pthread_mutex_lock(&mutex_puertoPartida);
	puertoPartida++;
	nuevoPuerto = puertoPartida;
	pthread_mutex_unlock(&mutex_puertoPartida);
	return nuevoPuerto;
}

/**
 * Verificar el estado del torneo para saber si finalizo el tiempo de inscripcion al torneo
 */
bool torneoFinalizado() {
	bool estado = false;
	pthread_mutex_lock(&mutex_timeIsUp);
	estado = timeIsUp;
	pthread_mutex_unlock(&mutex_timeIsUp);
	return estado;
}

/////////////////////////////// THREADS ////////////////////////////

/**
 * THREAD -> Controla el tiempo que debe durar el torneo
 */
void* temporizadorTorneo(void* data) {
	struct thTemporizador_data *torneo;
	torneo = (struct thTemporizador_data *) data;

	sem_inicializarTemporizador.P();
	cout << "Comienza el temporizador" << endl;
	sleep(torneo->duracion * 30);

	pthread_mutex_lock(&mutex_timeIsUp);
	timeIsUp = true;
	pthread_mutex_unlock(&mutex_timeIsUp);

	pthread_cancel(torneo->thAceptarJugadores);
	//pthread_cancel(torneo->thEstablecerPartidas);
	pthread_exit(NULL);
}

/**
 * THREAD -> Modo Grafico
 */
void* modoGrafico(void* data) {
	struct thModoGrafico_data *torneo;
	torneo = (struct thModoGrafico_data *) data;

	SDL_Surface *screen, *background, *tiempo, *jugadores, *infoJugador;
	SDL_Rect posDestino, posBackground, posTiempo, posJugadores, posInfoJugadores;
	SDL_Color colorNegro, colorBlanco;
	TTF_Font *font;

	//Colores
	colorNegro.r = colorNegro.g = colorNegro.b = 0;
	colorBlanco.r = colorBlanco.g = colorBlanco.b = 255;

	background = SDL_LoadBMP("Img/background.bmp");
	//verificamos si ha ocurrido algun error cargando la imagen
	if (background == NULL) {
		printf("Error en SDL_LoadBMP= %s\n", SDL_GetError());
		pthread_exit(NULL);
	}

	//creo una copia del background
	posBackground.x = 0;
	posBackground.y = 0;
	//SDL_BlitSurface(backgroundOriginal, NULL, background, &posBackground);

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
	font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
	if (font == NULL) {
		printf("Error abriendo la fuente ttf: %s\n", SDL_GetError());
		pthread_exit(NULL);
	}

	int minutos = torneo->duracion;
	int segundos = 0;
	posTiempo.x = 50;
	posTiempo.y = 140;
	char txtTiempo[10];
	sprintf(txtTiempo, "TIME %02d:%02d", minutos, segundos);
	tiempo = TTF_RenderText_Solid(font, txtTiempo, colorBlanco);
	SDL_BlitSurface(tiempo, NULL, background, &posTiempo);

	posJugadores.x = 530;
	posJugadores.y = 140;
	char txtPlayers[10];
	sprintf(txtPlayers, "Players: %d", 0);
	jugadores = TTF_RenderText_Solid(font, txtPlayers, colorBlanco);
	SDL_BlitSurface(jugadores, NULL, background, &posJugadores);

	posBackground.x = 0;
	posBackground.y = 0;
	SDL_BlitSurface(background, NULL, screen, &posBackground);
	SDL_Flip(screen);

	sleep(3);

	char txtInfoJugador[50];
	int cantPlayersConectados;
	//varaible timeIsUp habilitar cuanto comienze la partida
	while (true) {
		background = SDL_LoadBMP("Img/background.bmp");
		//actualizar tiempo
		if (minutos > 0 && segundos == 0) {
			minutos--;
			segundos = 59;
		} else if (segundos > 0) {
			segundos--;
		}
		if (minutos == 0 && segundos == 0) {
			strcpy(txtTiempo, "Tournament finished, waiting matchs...");
		} else {
			sprintf(txtTiempo, "TIME %02d:%02d", minutos, segundos);
		}
		tiempo = TTF_RenderText_Solid(font, txtTiempo, colorBlanco);
		SDL_BlitSurface(tiempo, NULL, background, &posTiempo);

		//actualizar ranking
		//sacar promedios
		pthread_mutex_lock(&mutex_listJugadores);
		cantPlayersConectados = listJugadores.size();

		/*this->Puntaje = 0;
		 this->SocketAsociado = SocketAsociado;
		 this->Jugando = false;
		 this->PartidasGanadas = 0;
		 this->PartidasPerdidas = 0;
		 this->CantPartidasJugadas = 0;
		 this->Ranking = 0;*/
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			if (it->second->CantPartidasJugadas > 0) {
				it->second->Promedio = it->second->Puntaje / it->second->CantPartidasJugadas;
			}
		}

		//ordenar en un array los puntajes con los ID        //podria actualizar el ranking
		//
		//

		pthread_mutex_unlock(&mutex_listJugadores);
		posDestino.x = 115;
		posDestino.y = 180;
		int i;
		for (i = 1; i <= 10; ++i) {
			sprintf(txtInfoJugador, "%02d    %-10.10s    %06d    %02d    %02d", i, "peepee7891", 5500, 4, 3);
			infoJugador = TTF_RenderText_Solid(font, txtInfoJugador, colorBlanco);
			SDL_BlitSurface(infoJugador, NULL, background, &posDestino);
			posDestino.y += 30;
		}

		//actualizar cantidad de jugadores conectados
		sprintf(txtPlayers, "Players: %d", cantPlayersConectados);
		jugadores = TTF_RenderText_Solid(font, txtPlayers, colorBlanco);
		SDL_BlitSurface(jugadores, NULL, background, &posJugadores);

		SDL_BlitSurface(background, NULL, screen, &posBackground);
		SDL_Flip(screen);

		//cambiar esto por algo mejor
		if (minutos == 0 && segundos == 0) {
			break;
		}

		usleep(1000000);
	}

	sleep(40);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(background);
	SDL_FreeSurface(tiempo);
	SDL_FreeSurface(jugadores);
	SDL_FreeSurface(infoJugador);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();

	pthread_exit(NULL);
}

/**
 * THREAD -> KEEPALIVE Jugadores - Actualiza la lista de jugadores quitando los que ya no estan activos
 */
void* actualizarListaJugadores(void*) {
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	bzero(buffer, sizeof(buffer));
	int readDataCode;

	string message(CD_ACK);
	string content;
	message.append(fillMessage(content));

	while (true) {
		//para cada jugador ver si me responden la señal de KEEPALIVE
		pthread_mutex_lock(&mutex_listJugadores);
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {

			it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
			readDataCode = it->second->SocketAsociado->ReceiveBloq(buffer, (LONGITUD_CODIGO + LONGITUD_CONTENIDO));

			if (readDataCode == 0) {
				//el jugador se desconecto
				quitarJugador(it->first);
			} else {

			}
		}
		pthread_mutex_unlock(&mutex_listJugadores);
		usleep(50000);
	}

	pthread_exit(NULL);
}

/**
 * THREAD -> Aceptar las conexiones de nuevos jugadores al torneo
 */
void* aceptarJugadores(void* data) {
	int clientId = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CODIGO PARA PROBAR EL ASIGNADOR DE PARTIDAS
	/*pthread_mutex_lock(&mutex_listJugadores);
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
	 pthread_mutex_unlock(&mutex_listJugadores);
	 //cout << "oponente para el jugador 1: " << (*listJugadores[1]).obtenerOponente(&listJugadores) << endl;

	 for (map<int, Jugador*>::iterator it = (listJugadores).begin(); it != (listJugadores).end(); it++) {
	 cout << (*(*it).second).Nombre << " - partidas:" << endl;

	 for (map<int, int>::iterator itmap = (*(*it).second).Partidas.begin(); itmap != (*(*it).second).Partidas.end(); ++itmap) {
	 std::cout << itmap->first << " => " << itmap->second << '\n';
	 }
	 }
	 */
//FIN CODIGO PARA PROBAR EL ASIGNADOR DE PARTIDAS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Crear Socket del Servidor
	ServerSocket sSocket(puertoTorneo);
	char nombreJugador[LONGITUD_CODIGO + LONGITUD_CONTENIDO];

	while (true) {
		CommunicationSocket * cSocket = sSocket.Accept();
		cSocket->ReceiveBloq(nombreJugador, (LONGITUD_CODIGO + LONGITUD_CONTENIDO));
		clientId++;
		agregarJugador(new Jugador(clientId, nombreJugador, cSocket));

		//mandarle el ID al Jugador
		char aux[LONGITUD_CONTENIDO];
		string message(CD_ID_JUGADOR);
		sprintf(aux, "%d", clientId);
		message.append(fillMessage(aux));
		cSocket->SendNoBloq(message.c_str(), message.length());
	}

	pthread_exit(NULL);
}

/**
 * THREAD -> Crea los servidores de partidas
 */
void* establecerPartidas(void* data) {
	pid_t pid;
	int idJugador;
	int idOponente;
	int nroPartida = 1;

	int i = 1;
	while (!torneoFinalizado()) {
		cout << "pasada de busqueda nro: " << i++ << endl;

		//recorro la lista de jugadores viendo a quien le puedo asignar un oponente y que comienze la partida
		pthread_mutex_lock(&mutex_listJugadores);
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			idJugador = it->first;
			idOponente = it->second->obtenerOponente();

			//si no se encuentra jugando actualmente y se encontro un oponente lanzo el servidor de partida
			if (idOponente > 0) {
				//habilito el temporizador del torneo
				if (nroPartida == 1) {
					cout << "Se crea la primer partida y doy permiso a iniciar el temporizador" << endl;
					sem_inicializarTemporizador.V();
				}
				nroPartida++;

				int puertoNuevaPartida = getNewPort();

				//Le mando a los jugadores el nro de Puerto en el que comenzara la partida
				char auxPuertoNuevaPartida[10];
				sprintf(auxPuertoNuevaPartida, "%d", puertoNuevaPartida);
				string message(CD_PUERTO_PARTIDA);
				message.append(fillMessage(auxPuertoNuevaPartida));

				listJugadores[idJugador]->CantPartidasJugadas++;
				listJugadores[idOponente]->CantPartidasJugadas++;
				listJugadores[idJugador]->SocketAsociado->SendNoBloq(message.c_str(), message.length());
				listJugadores[idOponente]->SocketAsociado->SendNoBloq(message.c_str(), message.length());

				if ((pid = fork()) == 0) {
					//Proceso hijo. Hacer exec
					cout << "J" << idJugador << " Crear Partida en el puerto: " << puertoNuevaPartida << " (" << idJugador << "vs" << idOponente << ")" << endl;

					char auxCantVidas[2];
					sprintf(auxCantVidas, "%d", cantVidas);

					char *argumentos[] = { auxPuertoNuevaPartida, auxCantVidas };
					execv("/Servidor Partida/Servidor Partida", argumentos);
					//return 1;
				} else if (pid < 0) {
					//Hubo error
					cout << "Error al forkear" << endl;
				} else {
					//Soy el padre.
					//delete (cSocket);
				}
			} else {
				cout << "J" << idJugador << " No puede jugar" << endl;
			}
		}
		pthread_mutex_unlock(&mutex_listJugadores);
		usleep(INTERVALO_ENTRE_BUSQUEDA_DE_OPONENTES);
	}

//ver si hace falta??????????
//el tiempo del Torneo llego a su fin, informar a cada cliente
	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		string message(CD_FIN_TORNEO);
		message.append(fillMessage("1"));
		it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
	}
	pthread_mutex_unlock(&mutex_listJugadores);

	/**
	 * hacer calculos del ganador y mandar a cada cliente su puntaje y ranking
	 *
	 */

//pthread_mutex_destroy(&mutex_listJugadores);
	pthread_mutex_destroy(&mutex_puertoPartida);
	pthread_mutex_destroy(&mutex_timeIsUp);
	cout << "Thread EstablecerPartidas va a hacer un Exit" << endl;
	pthread_exit(NULL);
}

void asociarSegmento(int* idShm, int* variable) {
//int idShm;
//int *variable = NULL;

	/*
	 bool jugadoresKeepAlive[MAX_JUGADORES_SOPORTADOS];
	 int i;
	 for (i=0; i<MAX_JUGADORES_SOPORTADOS; i++) { jugadoresKeepAlive[i] = true; }
	 */

	key_t key = ftok("/bin/ls", CLAVE_MEMORIA_COMPARTIDA);
	if (key == -1) {
		cout << "Error al generar clave de memoria compartida" << endl;
		return;
	}

	*idShm = shmget(key, sizeof(int) * 1, IPC_CREAT | PERMISOS_SHM);
	if (*idShm == -1) {
		cout << "Error al obtener memoria compartida" << endl;
		return;
	}

	variable = (int*) shmat(*idShm, 0, 0);
	if (variable == NULL) {
		cout << "Error al asignar memoria compartida reservada" << endl;
		return;
	}
}

void eliminarMemoriaCompartida(void * bloqueCompartido, int IdBloqueCompartido) {
	shmdt((char *) bloqueCompartido);
	shmctl(IdBloqueCompartido, IPC_RMID, (struct shmid_ds *) NULL);
}

//AUXILIARES
/**
 * Formatea el mensaje para mandarlo por socket
 */
string fillMessage(string message) {
	string content;
	int cantCeros = LONGITUD_CONTENIDO - message.length();
	content.assign(cantCeros, '0');
	return content.append(message);
}

/**
 * Convertir un numero de tipo int a String
 */
string intToString(int number) {
	stringstream ss;
	ss << number;
	return ss.str();
}
