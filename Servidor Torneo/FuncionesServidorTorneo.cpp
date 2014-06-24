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

///////////////////////////////////////////////////////////////////////////////
void cargarImagenes();
void esperarConexiones();
void limpiarPantalla();
void limpiarPantalla2();
void dibujarImagen(SDL_Surface *, int, int);
void escribir(const char *, int, int);
void mostrarJugadores();
/**
 * THREAD -> Modo Grafico
 */
void* modoGrafico(void*) {
	SDL_Rect pantalla_juego, pantalla_texto;

	SDL_Color color_texto;
	TTF_Font *fuente;

	short int caracter = 0;
	char nombre[10] = { ' ' };

	const char pared_bmp[] = "Img/listado.bmp";

	SDL_Surface *superficie, *pared, *texto;
	pared = SDL_LoadBMP(pared_bmp);

	//Inicio modo video
	SDL_Init(SDL_INIT_VIDEO);
	//Inicio modo texto grafico
	TTF_Init();

	//Defino las propiedades de la pantalla del juego
	superficie = SDL_SetVideoMode(ANCHO_PANTALLA_SERVIDOR, ALTO_PANTALLA_SERVIDOR, BPP_SERVIDOR, SDL_HWSURFACE);
	//Seteo el titulo de la pantalla
	SDL_WM_SetCaption("Rahlp Tournament", NULL);
	//Cargo la fuente
	fuente = TTF_OpenFont("Img/letra.ttf", 24);
	//Color del texto
	color_texto.r = color_texto.g = color_texto.b = 245;

	//Dimensiones rectangulo donde irá el texto
	pantalla_texto.x = 10;
	pantalla_texto.y = 10;

	//Texto del texto
	nombre[caracter] = '\0';
	texto = TTF_RenderText_Solid(fuente, nombre, color_texto);
	SDL_BlitSurface(texto, NULL, superficie, &pantalla_texto);
	SDL_Flip(superficie);

	pthread_exit(NULL);
}

//**************************
//***dibujarImagen**********
//**************************
/*
void dibujarImagen(SDL_Surface *image, int x, int y) {
	SDL_Rect src, dest;
	src.x = 0;
	src.y = 0;
	src.w = image->w;
	src.h = image->h;
	dest.x = x;
	dest.y = y;
	dest.w = image->w;
	dest.h = image->h;
	SDL_BlitSurface(image, &src, screen, &dest);
}*/

//**************************
//***escribir***************
//**************************
/*
void escribir(const char *texto, int x, int y) {
	SDL_Surface *image;
	SDL_Color colorTexto = { 0, 174, 0 };
	image = TTF_RenderText_Solid(fuente, texto, colorTexto);
	if (image == NULL) {
		cout << "TTF_RenderText_Solid() fallo." << endl;
		salir(0);
	}
	dibujarImagen(image, x, y);
}*/

//**************************
//***mostrarJugadores*******
//**************************
/*
void mostrarJugadores() {
	int posicion;
	while (true) {
		pthread_mutex_lock (&posiciones);
		for (int i = 0; i < lista.size(); i++)
			lista[i] = shmBuffer[i];
		limpiarPantalla2();
		int posicionY = 130;
		int posicionX = fondo2->w / 2 - 300;
		char aux[TAMBUF];

		sprintf(aux, "%d", (int) lista.size());
		escribir(aux, 320, fondo->h - 78);
		sprintf(aux, "%d", srvTorneo.numRonda);
		escribir(aux, fondo->w / 2 + 112, fondo->h - 78);
		sprintf(aux, "%d", srvTorneo.cantRondas - srvTorneo.numRonda);
		escribir(aux, fondo->w - 95, fondo->h - 78);
		sort(lista.rbegin(), lista.rend());

		posicion = 1;
		for (int i = 0; i < lista.size(); i++) {
			if (i != 0)
				if (lista[i].puntaje != lista[i - 1].puntaje || lista[i].cantRanas != lista[i - 1].cantRanas)
					posicion = i + 1;
			shmBuffer[i].posicion = posicion;
			posicionY += TAMFUENTE + 20;
			sprintf(aux, "%d", posicion);
			escribir(aux, posicionX - (3 * TAMFUENTE), posicionY);
			strncpy(aux, lista[i].nombre, 10);
			escribir(aux, posicionX, posicionY);
			sprintf(aux, "%d", lista[i].puntaje);
			escribir(aux, posicionX + 215, posicionY);
			sprintf(aux, "%d", lista[i].victorias);
			escribir(aux, posicionX + 415, posicionY);
			sprintf(aux, "%d", lista[i].derrotas);
			escribir(aux, posicionX + 615, posicionY);

		}
		SDL_Flip (screen);
	}
}*/
//////////////////////////////////////////////////

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
	/*clientId++;
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

	 for (map<int, Jugador*>::iterator it = (listJugadores).begin(); it != (listJugadores).end(); it++) {
	 cout << (*(*it).second).Nombre << " - partidas:" << endl;

	 for (map<int, int>::iterator itmap = (*(*it).second).Partidas.begin(); itmap != (*(*it).second).Partidas.end(); ++itmap) {
	 std::cout << itmap->first << " => " << itmap->second << '\n';
	 }
	 }
	 /**/
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
