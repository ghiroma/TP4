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
#include <list>
#include <signal.h>
#include <wait.h>
#include <sys/socket.h>

bool timeIsUp = false;
bool comenzoConteo = false;
bool todasLasPartidasFinalizadas = false;
int cantPartidasFinalizadas = 0;
Semaforo sem_inicializarTemporizador((char*) "/sem_inicializarTemporizador", 0);
Semaforo sem_ServidorTorneoSHM((char*) "/ServidorTorneoSHM", 0);
Semaforo sem_ServidorPartidaSHM((char*) "/ServidorPartidaSHM", 1);
pthread_mutex_t mutex_listJugadores = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_timeIsUp = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_comenzoConteo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_todasLasPartidasFinalizadas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_partidasActivas = PTHREAD_MUTEX_INITIALIZER;

extern pthread_mutex_t mutex_listJugadores;
extern map<int,
Jugador*> listJugadores;
extern int puertoServidorPartida;
extern int idSHM;
list<datosPartida> partidasActivas;
extern ServerSocket* sSocket;
int idPartida = 0;
puntajesPartida * resumenPartida;
extern pid_t pidServidorPartida;

SDL_Surface *screen, *background, *tiempo, *jugadores, *infoJugador_part1, *infoJugador_part2;
SDL_Rect posDestino, posBackground, posTiempo, posJugadores;
TTF_Font *font;

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

/**
 * Manejador de señales
 */
void SIG_Handler(int inum) {
	cout << "Torneo Señal Handler PID:" << getpid() << endl;
	exit(1);
}

/**
 * Detecta cuando un Servidor Partida murio
 */
void SIG_CHLD(int inum) {
	//MUERTE DE LA PARTIDA
	int childpid = wait(NULL);
	cout << "Señal Handler SIGCHLD - PID:" << childpid << endl;

	cout << "Servidor de Partida caido" << endl;
	cout << "Se terminara el Torneo" << endl;

	if (cantPartidasFinalizadas != idPartida) {
		exit(1);
	}

	/*pthread_mutex_lock(&mutex_todasLasPartidasFinalizadas);
	 todasLasPartidasFinalizadas = true;
	 pthread_mutex_unlock(&mutex_todasLasPartidasFinalizadas);

	 pthread_mutex_lock(&mutex_timeIsUp);
	 timeIsUp = true;
	 pthread_mutex_unlock(&mutex_timeIsUp);*/
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
	//cierro el socket asociado al jugador
	if (listJugadores.count(id) == 1) {
		delete (listJugadores[id]->SocketAsociado);

		//para cada participante le quito de su lista el jugador que se da de baja
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			it->second->quitarJugador(id);
		}
		//lo quito de la lista de jugadores del torneo
		listJugadores.erase(id);
	}
}

/**
 * Verificar el estado del torneo para saber si finalizo el tiempo de inscripcion al torneo
 */
bool torneoFinalizado() {
	bool estado;
	pthread_mutex_lock(&mutex_timeIsUp);
	estado = timeIsUp;
	pthread_mutex_unlock(&mutex_timeIsUp);
	return estado;
}

/**
 * Verificar el estado de las partidas
 */
bool partidasFinalizadas() {
	bool estado;
	pthread_mutex_lock(&mutex_todasLasPartidasFinalizadas);
	estado = todasLasPartidasFinalizadas;
	pthread_mutex_unlock(&mutex_todasLasPartidasFinalizadas);
	return estado;
}

/////////////////////////////// THREADS ////////////////////////////

/**
 * THREAD -> Controla el tiempo que debe durar el torneo
 */
void* temporizadorTorneo(void* data) {
	cout << "Thread temporizadorTorneo - PID:" << getpid() << endl;
	struct thTemporizador_data *torneo;
	torneo = (struct thTemporizador_data *) data;

	sem_inicializarTemporizador.P();

	pthread_mutex_lock(&mutex_comenzoConteo);
	comenzoConteo = true;
	pthread_mutex_unlock(&mutex_comenzoConteo);

	sleep(torneo->duracion * 60);

	pthread_mutex_lock(&mutex_timeIsUp);
	timeIsUp = true;
	pthread_mutex_unlock(&mutex_timeIsUp);

	if (sSocket != NULL) {
		cout << "(torneo finalizo tiempo) Torneo -> cierro socket" << endl;
		delete (sSocket);
		sSocket = NULL;
	}

	//el tiempo del Torneo llego a su fin, informar a cada cliente
	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		string message(CD_FIN_TORNEO);
		message.append(fillMessage("1"));
		it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
		cout << "mande CD_FIN_TORNEO a" << it->second->Id << " . Nombre:" << it->second->Nombre << endl;
	}
	pthread_mutex_unlock(&mutex_listJugadores);

	pthread_cancel(torneo->thAceptarJugadores);
	pthread_exit(NULL);
}

/**
 * THREAD -> Lee los resultados de las partidas
 */
void* lecturaDeResultados(void* data) {
	resumenPartida = (struct puntajesPartida *) shmat(idSHM, (char *) 0, 0);

	while (true) {

		if (cantPartidasFinalizadas == idPartida && torneoFinalizado()) {
			break;
		}
		if (sem_trywait(sem_ServidorTorneoSHM.getSem_t()) != -1) {
			pthread_mutex_lock(&mutex_listJugadores);
			//si el torneo termino ok Grabo los puntajes
			if (resumenPartida->partidaFinalizadaOK == true) {
				//si el jugador No se desconecto le sumo su puntaje y cantPartidasJugadas
				if (listJugadores.count(resumenPartida->idJugador1) == 1) {
					listJugadores[resumenPartida->idJugador1]->Puntaje += resumenPartida->puntajeJugador1;
					listJugadores[resumenPartida->idJugador1]->CantPartidasJugadas++;

					//si el otro jugador se desconecto asumo que este gano porque el otro abandono
					if (listJugadores.count(resumenPartida->idJugador2) == 0) {
						listJugadores[resumenPartida->idJugador1]->PartidasGanadas++;
					}
				}
				//si el jugador No se desconecto le sumo su puntaje y cantPartidasJugadas
				if (listJugadores.count(resumenPartida->idJugador2) == 1) {
					listJugadores[resumenPartida->idJugador2]->Puntaje += resumenPartida->puntajeJugador2;
					listJugadores[resumenPartida->idJugador2]->CantPartidasJugadas++;

					//si el otro jugador se desconecto asumo que este gano porque el otro abandono
					if (listJugadores.count(resumenPartida->idJugador1) == 0) {
						listJugadores[resumenPartida->idJugador2]->PartidasGanadas++;
					}
				}

				//si los dos estan vivos veo quien gano
				if (listJugadores.count(resumenPartida->idJugador1) == 1 && listJugadores.count(resumenPartida->idJugador2) == 1) {
					//si no es empate, veo quien gano y perdio
					if ((listJugadores[resumenPartida->idJugador1]->Puntaje / listJugadores[resumenPartida->idJugador1]->CantPartidasJugadas) != (listJugadores[resumenPartida->idJugador2]->Puntaje / listJugadores[resumenPartida->idJugador2]->CantPartidasJugadas)) {
						if ((listJugadores[resumenPartida->idJugador1]->Puntaje / listJugadores[resumenPartida->idJugador1]->CantPartidasJugadas) > (listJugadores[resumenPartida->idJugador2]->Puntaje / listJugadores[resumenPartida->idJugador2]->CantPartidasJugadas)) {
							//J1 Gana
							listJugadores[resumenPartida->idJugador1]->PartidasGanadas++;
							listJugadores[resumenPartida->idJugador2]->PartidasPerdidas++;
						} else {
							//J2 Gana
							listJugadores[resumenPartida->idJugador2]->PartidasGanadas++;
							listJugadores[resumenPartida->idJugador1]->PartidasPerdidas++;
						}
					}
				}
			}

			//pongo a los jugadores en no JUNGADO
			if (listJugadores.count(resumenPartida->idJugador1) == 1) {
				listJugadores[resumenPartida->idJugador1]->Jugando = false;
			}
			if (listJugadores.count(resumenPartida->idJugador2) == 1) {
				listJugadores[resumenPartida->idJugador2]->Jugando = false;
			}
			pthread_mutex_unlock(&mutex_listJugadores);

			///mensajes de prueba
			cout << "CONTENIDO DE SHM" << endl;
			cout << "id1:" << resumenPartida->idJugador1 << endl;
			cout << "id2:" << resumenPartida->idJugador2 << endl;
			cout << "partidaFinalizadaOK:" << resumenPartida->partidaFinalizadaOK << endl;
			cout << "puntajeJugador1:" << resumenPartida->puntajeJugador1 << endl;
			cout << "puntajeJugador2:" << resumenPartida->puntajeJugador2 << endl;
			///////////////////////////////////////////////////////////////////////////////////

			cantPartidasFinalizadas++;
			//cout<<"CantPartidasFinalizads = "<<cantPartidasFinalizadas<<endl<<" idPartida = "<<idPartida<<endl<<" torneoFinalizado = "<<torneoFinalizado()<<endl;
			//if (cantPartidasFinalizadas == idPartida && torneoFinalizado()) {
			//break;
			//}
			sem_ServidorPartidaSHM.V();
		}
		//sem_ServidorTorneoSHM.P();
		usleep(10000);
	}

	//en este punto ya se que todas las partidas finalizaron y el tiempo de torneo tambien
	kill(pidServidorPartida, SIGINT);
	cout << "KILL a partida" << endl;

	pthread_mutex_lock(&mutex_todasLasPartidasFinalizadas);
	todasLasPartidasFinalizadas = true;
	pthread_mutex_unlock(&mutex_todasLasPartidasFinalizadas);

	pthread_exit(NULL);
}

/**
 * THREAD -> Modo Grafico
 */
void* modoGrafico(void* data) {
	cout << "Thread modoGrafico - PID:" << getpid() << endl;
	struct thModoGrafico_data *torneo;
	torneo = (struct thModoGrafico_data *) data;

	SDL_Color colorNegro, colorBlanco;

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

	int minutos = torneo->duracion;
	int segundos = 0;
	//int minutos = 0;
	//int segundos = 20;
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

	char txtInfoJugador[MAX_LENGT_TXT_INFO_JUGADOR];
	int cantPlayersConectados;
	multimap<float, int> rankings;
	bool actualizarTiempo = false;
	while (true) {
		background = SDL_LoadBMP("Img/background.bmp");

		//actualizar tiempo
		pthread_mutex_lock(&mutex_comenzoConteo);
		actualizarTiempo = comenzoConteo;
		pthread_mutex_unlock(&mutex_comenzoConteo);
		if (actualizarTiempo) {
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
		} else {
			strcpy(txtTiempo, "Waiting players to start");
			tiempo = TTF_RenderText_Solid(font, txtTiempo, colorBlanco);
			SDL_BlitSurface(tiempo, NULL, background, &posTiempo);
		}

		//actualizar ranking
		pthread_mutex_lock(&mutex_listJugadores);
		cantPlayersConectados = listJugadores.size();

		rankings.clear();
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			if (it->second->CantPartidasJugadas > 0) {
				it->second->Promedio = it->second->Puntaje / it->second->CantPartidasJugadas;
				//ordena en un map los promedios con los ID
				rankings.insert(pair<float, int>(it->second->Promedio, it->second->Id));
			} else {
				rankings.insert(pair<float, int>(0, it->second->Id));
			}
		}

		posDestino.y = 220;
		int i = 1;
		for (multimap<float, int>::reverse_iterator it = rankings.rbegin(); it != rankings.rend(); ++it) {
			listJugadores[it->second]->Ranking = i;
			sprintf(txtInfoJugador, "%02d           %-10.10s", i++, listJugadores[it->second]->Nombre.c_str());
			infoJugador_part1 = TTF_RenderText_Solid(font, txtInfoJugador, colorNegro);
			posDestino.x = 45;
			SDL_BlitSurface(infoJugador_part1, NULL, background, &posDestino);
			sprintf(txtInfoJugador, "%06d        %02d      %02d", listJugadores[it->second]->Puntaje, listJugadores[it->second]->PartidasGanadas, listJugadores[it->second]->PartidasPerdidas);
			infoJugador_part2 = TTF_RenderText_Solid(font, txtInfoJugador, colorNegro);
			posDestino.x = 400;
			SDL_BlitSurface(infoJugador_part2, NULL, background, &posDestino);
			posDestino.y += 30;
		}
		pthread_mutex_unlock(&mutex_listJugadores);

		//actualizar cantidad de jugadores conectados
		sprintf(txtPlayers, "Players: %d", cantPlayersConectados);
		jugadores = TTF_RenderText_Solid(font, txtPlayers, colorBlanco);
		SDL_BlitSurface(jugadores, NULL, background, &posJugadores);

		SDL_BlitSurface(background, NULL, screen, &posBackground);
		SDL_Flip(screen);

		//se acaba el tiempo y salgo del ciclo
		if (minutos == 0 && segundos == 0) {
			//if (torneoFinalizado()) {
			break;
		}
		usleep(TIEMPO_DE_REDIBUJADO);
	}

	//esperar que todas las partidas finalicen
	while (!partidasFinalizadas()) {
		usleep(1000000);
	}

	//mostrar pantalla final
	background = SDL_LoadBMP("Img/background.bmp");
	strcpy(txtTiempo, "Tournament finished");
	tiempo = TTF_RenderText_Solid(font, txtTiempo, colorBlanco);
	SDL_BlitSurface(tiempo, NULL, background, &posTiempo);
	pthread_mutex_lock(&mutex_listJugadores);
	cantPlayersConectados = listJugadores.size();

	rankings.clear();
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		if (it->second->CantPartidasJugadas > 0) {
			it->second->Promedio = it->second->Puntaje / it->second->CantPartidasJugadas;
			//ordenar en un array los promedios con los ID
			rankings.insert(pair<float, int>(it->second->Promedio, it->second->Id));
		} else {
			rankings.insert(pair<float, int>(0, it->second->Id));
		}
	}

	posDestino.y = 220;
	int i = 1;
	for (multimap<float, int>::reverse_iterator it = rankings.rbegin(); it != rankings.rend(); ++it) {
		listJugadores[it->second]->Ranking = i;
		sprintf(txtInfoJugador, "%02d           %-10.10s", i++, listJugadores[it->second]->Nombre.c_str());
		infoJugador_part1 = TTF_RenderText_Solid(font, txtInfoJugador, colorNegro);
		posDestino.x = 45;
		SDL_BlitSurface(infoJugador_part1, NULL, background, &posDestino);
		sprintf(txtInfoJugador, "%06d        %02d      %02d", listJugadores[it->second]->Puntaje, listJugadores[it->second]->PartidasGanadas, listJugadores[it->second]->PartidasPerdidas);
		infoJugador_part2 = TTF_RenderText_Solid(font, txtInfoJugador, colorNegro);
		posDestino.x = 400;
		SDL_BlitSurface(infoJugador_part2, NULL, background, &posDestino);
		posDestino.y += 30;
	}
	pthread_mutex_unlock(&mutex_listJugadores);

	SDL_BlitSurface(background, NULL, screen, &posBackground);
	SDL_Flip(screen);

	pthread_exit(NULL);
}

/**
 * THREAD -> KEEPALIVE Jugadores - Actualiza la lista de jugadores quitando los que ya no estan activos
 */
void* keepAliveJugadores(void*) {
	cout << "Thread keepAliveJugadores - PID:" << getpid() << endl;
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	bzero(buffer, sizeof(buffer));
	int readDataCode;

	string message(CD_ACK);
	string content;
	message.append(fillMessage(content));

	while (!partidasFinalizadas()) {
		//Para cada jugador ver si me responden la señal de KEEPALIVE
		pthread_mutex_lock(&mutex_listJugadores);
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
			readDataCode = it->second->SocketAsociado->ReceiveBloq(buffer, (LONGITUD_CODIGO + LONGITUD_CONTENIDO));

			if (readDataCode == 0) {
				//El jugador se desconecto
				cout << "Voy a eliminar a: " << it->first << endl;
				quitarJugador(it->first);
			} else {

			}
		}
		pthread_mutex_unlock(&mutex_listJugadores);

		usleep(TIEMPO_DE_KEEPALIVEJUGADORES);
	}

	pthread_exit(NULL);
}

/**
 * THREAD -> Aceptar las conexiones de nuevos jugadores al torneo
 */
void* aceptarJugadores(void* data) {
	cout << "Thread aceptarJugadores - PID:" << getpid() << endl;
	int clientId = 0;

	cout << sSocket->ShowHostName() << endl;

	char nombreJugador[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	char aux[LONGITUD_CONTENIDO];

	while (true) {
		cout << "va a bloquearse esperando al JUGADOR" << endl;
		try {
		//int fd = accept(sSocket->ID, NULL, NULL);
		//if (fd <= 0)
			//break;
		//throw "Error en accept";
		//CommunicationSocket * cSocket = new CommunicationSocket(fd);
		CommunicationSocket * cSocket = sSocket->Accept();
		cout << "va a bloquearse esperando el nombre" << endl;
		cSocket->ReceiveBloq(nombreJugador, (LONGITUD_CODIGO + LONGITUD_CONTENIDO));
		cout << "se conecto:" << nombreJugador << endl;
		clientId++;

		//Mandarle el ID al Jugador
		string messageId(CD_ID_JUGADOR);
		sprintf(aux, "%d", clientId);
		messageId.append(fillMessage(aux));
		cSocket->SendNoBloq(messageId.c_str(), messageId.length());

		//Madarle puerto partida
		string messagePuertoPartida(CD_PUERTO_PARTIDA);
		sprintf(aux, "%d", puertoServidorPartida);
		messagePuertoPartida.append(fillMessage(aux));
		cSocket->SendNoBloq(messagePuertoPartida.c_str(), messagePuertoPartida.length());

		cout << "Se agregara el jugador NRO:" << clientId << " NOMBRE: " << nombreJugador << endl;
		agregarJugador(new Jugador(clientId, nombreJugador, cSocket));
		} catch (...) {
		 //cout<<"Error en accept"<<endl;
			break;
		 }
	}

	cout << "Esta por salir thread de aceptar clientes" << endl;

	pthread_exit(NULL);
}

/**
 * THREAD -> Crea los servidores de partidas
 */
void* establecerPartidas(void* data) {
	cout << "Thread establecerPartidas - PID:" << getpid() << endl;
	int idJugador;
	int idOponente;

	while (!torneoFinalizado()) {
		//recorro la lista de jugadores viendo a quien le puedo asignar un oponente y que comienze la partida
		usleep(INTERVALO_ENTRE_BUSQUEDA_DE_OPONENTES);
		pthread_mutex_lock(&mutex_listJugadores);
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			idJugador = it->first;
			idOponente = it->second->obtenerOponente();

			//si no se encuentra jugando actualmente y se encontro un oponente ("crea" una nueva partida)
			if (idOponente >= 1) {
				cout << "se creara la partida: (" << idJugador << " vs " << idOponente << ")" << endl;
				idPartida++;

				//habilito el temporizador del torneo
				if (idPartida == 1) {
					//Se crea la primer partida y doy permiso a iniciar el temporizador
					sem_inicializarTemporizador.V();
				}

				//Le mando a los jugadores el ID de la partida
				char auxNroPartida[LONGITUD_CONTENIDO];
				sprintf(auxNroPartida, "%d", idPartida);
				string message(CD_ID_PARTIDA);
				message.append(fillMessage(auxNroPartida));

				cout << "le mando a ID: " << idJugador << " - el socket:" << auxNroPartida << endl;
				listJugadores[idJugador]->SocketAsociado->SendBloq(message.c_str(), message.length());
				cout << "le mando a ID: " << idOponente << " - el socket:" << auxNroPartida << endl;
				listJugadores[idOponente]->SocketAsociado->SendBloq(message.c_str(), message.length());

				//Les mando el nombre de su oponente
				char auxnombreOponente1[LONGITUD_CONTENIDO];
				sprintf(auxnombreOponente1, "%s", listJugadores[idOponente]->Nombre.c_str());
				string nombreOponente1(CD_NOMBRE);
				nombreOponente1.append(fillMessage(auxnombreOponente1));

				char auxnombreOponente2[LONGITUD_CONTENIDO];
				sprintf(auxnombreOponente2, "%s", listJugadores[idJugador]->Nombre.c_str());
				string nombreOponente2(CD_NOMBRE);
				nombreOponente2.append(fillMessage(auxnombreOponente2));

				cout << "le mando a ID: " << idJugador << " - el nombre oponente:" << nombreOponente1 << endl;
				listJugadores[idJugador]->SocketAsociado->SendBloq(nombreOponente1.c_str(), nombreOponente1.length());
				cout << "le mando a ID: " << idOponente << " - el nombre oponente:" << nombreOponente2 << endl;
				listJugadores[idOponente]->SocketAsociado->SendBloq(nombreOponente2.c_str(), nombreOponente2.length());

				cout << "Torneo: Termino de crear la partida " << idPartida << endl;
			} else {
				//No se encontraron oponentes disponibles
			}
		}
		pthread_mutex_unlock(&mutex_listJugadores);

	}
	//cout << "Thread EstablecerPartidas va a hacer un Exit" << endl;
	pthread_exit(NULL);
}

void mandarPuntajes() {
	//mandar a cada cliente su puntaje y ranking
	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		string message(CD_RANKING);
		message.append(fillMessage(intToString(it->second->Ranking)));
		it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
		cout << "MANDO RANKING #" << it->second->Ranking << " al Jugador:" << it->second->Id << endl;
	}
	pthread_mutex_unlock(&mutex_listJugadores);
}

void liberarRecursos() {
	cout << "libero recursos de Servidor Torneo PID:" << getpid() << endl;
	//SOCKETS
	if (sSocket != NULL) {
		cout << "Torneo -> cierro socket" << endl;
		//close(sSocket->ID);
		//shutdown(sSocket->ID,2);
		delete (sSocket);
	}

	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		delete (it->second->SocketAsociado);
	}
	pthread_mutex_unlock(&mutex_listJugadores);

	//SHM
	shmdt((struct puntajesPartida*) resumenPartida);
	shmctl(idSHM, IPC_RMID, (struct shmid_ds *) NULL);

	//Semaphores
	sem_inicializarTemporizador.close();
	sem_ServidorPartidaSHM.close();
	sem_ServidorTorneoSHM.close();

	//Mutex
	pthread_mutex_destroy(&mutex_timeIsUp);
	pthread_mutex_destroy(&mutex_comenzoConteo);
	pthread_mutex_destroy(&mutex_todasLasPartidasFinalizadas);
	pthread_mutex_destroy(&mutex_partidasActivas);
	pthread_mutex_destroy(&mutex_listJugadores);

	//SDL
	TTF_Quit();
	SDL_Quit();
}

void mostrarPantalla(const char* nombrPantalla) {
	posBackground.x = 0;
	posBackground.y = 0;
	string dir = "Img/";
	dir += nombrPantalla;
	dir += ".bmp";
	background = SDL_LoadBMP(dir.c_str());
	if (background == NULL) {
		printf("Error en SDL_LoadBMP= %s\n", SDL_GetError());
		exit(1);
	}
	SDL_BlitSurface(background, NULL, screen, &posBackground);
	SDL_Flip(screen);
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
