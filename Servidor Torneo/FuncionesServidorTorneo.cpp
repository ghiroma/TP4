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

extern unsigned int puertoTorneo;
bool timeIsUp = false;
bool comenzoConteo = false;
bool todasLasPartidasFinalizadas = false;
Semaforo sem_inicializarTemporizador((char*) "/sem_inicializarTemporizador", 0);
pthread_mutex_t mutex_listJugadores = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_timeIsUp = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_comenzoConteo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_todasLasPartidasFinalizadas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_partidasActivas = PTHREAD_MUTEX_INITIALIZER;

extern pthread_mutex_t mutex_listJugadores;
extern map<int,
Jugador*> listJugadores;
extern unsigned int puertoPartida;
extern int cantVidas;

SDL_Surface *screen, *background, *tiempo, *jugadores, *infoJugador_part1, *infoJugador_part2;
TTF_Font *font;

struct puntajesPartida {
	int idJugador1;
	int idJugador2;
	int puntajeJugador1;
	int puntajeJugador2;
	bool jugando;
	bool keepAlivePartida;
	bool keepAliveTorneo;
};

struct datosPartida {
	int idShm;
	sem_t* semaforo_pointerSem_t_Torneo;
	sem_t* semaforo_pointerSem_t_Partida;
	const char * semaforoShmName;
	int lecturasFallidasSHM_Partida;
};

list<datosPartida> partidasActivas;

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

void SIG_Handler(int inum) {
	cout << "Señal Handler" << endl;
	liberarRecursos();
	exit(1);
}

/**
 * Cuando ingresa un nuevo jugador al sistema.
 * Lo pongo en la lista de jugadores y lo sumo a la lista de Partidas de cada jugador
 */
void agregarJugador(Jugador* nuevoJugador) {
	cout << "---> agregarJugador" << endl;

	cout << "mutex agregarJugador" << endl;
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
	cout << "unmutex agregarJugador" << endl;
}

/**
 * Eliminar un jugador de la lista cuando abandona el torneo
 */
void quitarJugador(int id) {
	//cierro el socket asociado al jugador
	delete (listJugadores[id]->SocketAsociado);

	//para cada participante le quito de su lista el jugador que se da de baja
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		it->second->quitarJugador(id);
	}
	//lo quito de la lista de jugadores del torneo
	listJugadores.erase(id);
}

/**
 * Verificar el estado del torneo para saber si finalizo el tiempo de inscripcion al torneo
 */
bool torneoFinalizado() {
	bool estado;
	cout << "mutex torneoFinalizado" << endl;
	pthread_mutex_lock(&mutex_timeIsUp);
	estado = timeIsUp;
	pthread_mutex_unlock(&mutex_timeIsUp);
	cout << "unmutex torneoFinalizado --- timeisup = " << estado << endl;
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

	cout << "mutex comenzoConteo temporizadorTorneo" << endl;
	pthread_mutex_lock(&mutex_comenzoConteo);
	comenzoConteo = true;
	pthread_mutex_unlock(&mutex_comenzoConteo);
	cout << "unmutex comenzoConteo temporizadorTorneo" << endl;

	cout << "MMMMMMMMMMMMMMMMMMMMMMMM  empieza a contar" << endl;
	sleep(torneo->duracion * 20);
	cout << "MMMMMMMMMMMMMMMMMMMMMMMM  termina de contar" << endl;

	cout << "mutex_timeIsUp temporizadorTorneo --" << endl;
	pthread_mutex_lock(&mutex_timeIsUp);
	timeIsUp = true;
	cout << "TIMEISUP =" << timeIsUp << endl;
	pthread_mutex_unlock(&mutex_timeIsUp);
	cout << "unmutex_timeIsUp temporizadorTorneo --" << endl;

	//sem_inicializarTemporizador.close();
	//pthread_cancel(torneo->thEstablecerPartidas);
	pthread_cancel(torneo->thAceptarJugadores);
	cout << "HACE LOS DOS CANCEL THREAD" << endl;
	pthread_exit(NULL);
}

/**
 * THREAD -> KeepAlive partidas y torneo (tambien lee los puntajes de los juagadores)
 */
void* keepAlive(void* data) {
	puntajesPartida* resumenPartida;

	Semaforo auxSemSHMTorneo("/auxTorneo");
	sem_t * auxSemSHMToreno_Sem_t = auxSemSHMTorneo.getSem_t();
	Semaforo auxSemSHMPartida("/auxPartida");
	sem_t * auxSemSHMPartida_Sem_t = auxSemSHMPartida.getSem_t();
	bool allMatchsFinished = false;
	while (!allMatchsFinished) {
		cout << "mutex keepAlive partidasActivas" << endl;
		pthread_mutex_lock(&mutex_partidasActivas);
		for (list<datosPartida>::iterator it = partidasActivas.begin(); it != partidasActivas.end(); it++) {

			auxSemSHMPartida.setSem_t(it->semaforo_pointerSem_t_Partida);
			cout << "TIME WAIT IN" << endl;
			if (auxSemSHMPartida.timedWait(400000) == 0) {
				//Pudo acceder a la SHM
				cout << "Accedio por el Sem SHM" << endl;

				resumenPartida = (struct puntajesPartida *) shmat(it->idShm, (char *) 0, 0);
				resumenPartida->keepAliveTorneo = true;

				if (resumenPartida->keepAlivePartida == true) {
					it->lecturasFallidasSHM_Partida = 0;
					resumenPartida->keepAlivePartida = false;
				} else {
					it->lecturasFallidasSHM_Partida++;
				}

				if (it->lecturasFallidasSHM_Partida >= 5) {
					cout << "+5 Fallas -> entra por el Semaforo que lo habilito" << endl;
					cout << "mutex keepAlive listJugadores" << endl;
					pthread_mutex_lock(&mutex_listJugadores);
					if (resumenPartida->jugando == false) {
						//SERVIDOR PARTIDA termino OK
						cout << "SERVIDOR PARTIDA termino OK" << endl;
						//verificar y sumar pts y cant partidas jugadas

						//si el jugador No se desconecto le sumo su puntaje y cantPartidasJugadas
						if (listJugadores.count(resumenPartida->idJugador1) == 1) {
							listJugadores[resumenPartida->idJugador1]->Puntaje += resumenPartida->puntajeJugador1;
							listJugadores[resumenPartida->idJugador1]->CantPartidasJugadas++;

							//si el otro jugador se desconecto asumo que este gano porque el otro avandono
							if (listJugadores.count(resumenPartida->idJugador2) == 0) {
								listJugadores[resumenPartida->idJugador1]->PartidasGanadas++;
							}
						}
						//si el jugador No se desconecto le sumo su puntaje y cantPartidasJugadas
						if (listJugadores.count(resumenPartida->idJugador2) == 1) {
							listJugadores[resumenPartida->idJugador2]->Puntaje += resumenPartida->puntajeJugador2;
							listJugadores[resumenPartida->idJugador2]->CantPartidasJugadas++;

							//si el otro jugador se desconecto asumo que este gano porque el otro avandono
							if (listJugadores.count(resumenPartida->idJugador1) == 0) {
								listJugadores[resumenPartida->idJugador2]->PartidasGanadas++;
							}
						}

						//si los dos estan vivos veo quien gano
						if (listJugadores.count(resumenPartida->idJugador1) == 1 && listJugadores.count(resumenPartida->idJugador2) == 1) {
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
					} else {
						//SERVIDOR PARTIDA MURIO
						cout << "SERVIDOR PARTIDA MURIO" << endl;
						//actualizo los jugadores porque ya no estan jugando mas y asi les van a asignar una nueva partida
						listJugadores[resumenPartida->idJugador1]->Jugando = false;
						listJugadores[resumenPartida->idJugador2]->Jugando = false;
					}
					pthread_mutex_unlock(&mutex_listJugadores);
					cout << "unmutex keepAlive listJugadores" << endl;
				}

				auxSemSHMTorneo.setSem_t(it->semaforo_pointerSem_t_Torneo);
				auxSemSHMTorneo.V();
				usleep(400000);
			} else {
				//No pudo acceder a la SHM por el Semaforo
				cout << "No pudo acceder a SHM por el semaforo" << endl;
				if (it->lecturasFallidasSHM_Partida >= 5) {

					cout << "No accedio al semaforo y hay +5 fallas" << endl;

					//quitar partida el de la lista
					partidasActivas.erase(it);

					//cerrar SEMAFOROS
					auxSemSHMPartida.close();
					auxSemSHMTorneo.close();

					//limpiar memoria compartida
					shmdt((struct datosPartida *) resumenPartida);
					shmctl(it->idShm, IPC_RMID, (struct shmid_ds *) NULL);
				}

				it->lecturasFallidasSHM_Partida++;
			}

			if (partidasActivas.size() == 0) {
				usleep(400000);
			}

		}
		pthread_mutex_unlock(&mutex_partidasActivas);
		cout << "unmutex keepAlive partidasActivas" << endl;

		pthread_mutex_lock(&mutex_todasLasPartidasFinalizadas);
		cout << "keepalive torneos y partidas mutex  todasLasPartidasFinalizadas" << endl;
		allMatchsFinished = todasLasPartidasFinalizadas;
		pthread_mutex_unlock(&mutex_todasLasPartidasFinalizadas);
		cout << "keepalive torneos y partidas unmutex todasLasPartidasFinalizadas" << endl;
	}

	auxSemSHMPartida.setSem_t(auxSemSHMPartida_Sem_t);
	auxSemSHMPartida.close();
	auxSemSHMTorneo.setSem_t(auxSemSHMToreno_Sem_t);
	auxSemSHMTorneo.close();

	//hacer en algun lado o aca mismo el DELETE(semaforo)
	pthread_exit(NULL);
}

/**
 * THREAD -> Modo Grafico
 */
void* modoGrafico(void* data) {
	struct thModoGrafico_data *torneo;
	torneo = (struct thModoGrafico_data *) data;
	//son globales ahora -> SDL_Surface *screen, *background, *tiempo, *jugadores, *infoJugador_part1, *infoJugador_part2;
	//globales -> TTF_Font *font;
	SDL_Rect posDestino, posBackground, posTiempo, posJugadores;
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

	//int minutos = torneo->duracion;
	//int segundos = 0;
	int minutos = 0;
	int segundos = 20;
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
		cout << "mutex comenzoConteo modoGrafico" << endl;
		pthread_mutex_lock(&mutex_comenzoConteo);
		actualizarTiempo = comenzoConteo;
		pthread_mutex_unlock(&mutex_comenzoConteo);
		cout << "unmutex comenzoConteo modoGrafico" << endl;
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
		cout << "mutex modoGrafico ListJugadores" << endl;
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
		int i = 0;
		for (multimap<float, int>::iterator it = rankings.begin(); it != rankings.end(); it++) {
			sprintf(txtInfoJugador, "%02d           %-10.10s", (i + 1), listJugadores[it->second]->Nombre.c_str());
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
		cout << "unmutex modoGrafico listJugadores" << endl;

		//actualizar cantidad de jugadores conectados
		sprintf(txtPlayers, "Players: %d", cantPlayersConectados);
		jugadores = TTF_RenderText_Solid(font, txtPlayers, colorBlanco);
		SDL_BlitSurface(jugadores, NULL, background, &posJugadores);

		SDL_BlitSurface(background, NULL, screen, &posBackground);
		SDL_Flip(screen);

		//se acaba el tiempo y salgo del ciclo
		if (minutos == 0 && segundos == 0) {
			break;
		}
		usleep(1000000);
	}

	//esperar que todas las partidas finalicen
	int cantPartidasActivas;
	while (true) {
		cout << "mutex modoGrafico partidasActivas" << endl;
		pthread_mutex_lock(&mutex_partidasActivas);
		cantPartidasActivas = partidasActivas.size();
		pthread_mutex_unlock(&mutex_partidasActivas);
		cout << "unmutex modoGrafico partidasActivas" << endl;

		if (cantPartidasActivas == 0) {
			break;
		}
		usleep(1000000);
	}
	pthread_mutex_lock(&mutex_todasLasPartidasFinalizadas);
	cout << "mutex  modoGrafico todasLasPartidasFinalizadas" << endl;
	todasLasPartidasFinalizadas = true;
	pthread_mutex_unlock(&mutex_todasLasPartidasFinalizadas);
	cout << "unmutex modoGrafico todasLasPartidasFinalizadas" << endl;

	//mostrar pantalla final
	background = SDL_LoadBMP("Img/background.bmp");
	strcpy(txtTiempo, "Tournament finished");
	tiempo = TTF_RenderText_Solid(font, txtTiempo, colorBlanco);
	SDL_BlitSurface(tiempo, NULL, background, &posTiempo);
	cout << "mutex2 modoGrafico listJugadores" << endl;
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
	int i = 0;
	for (multimap<float, int>::iterator it = rankings.begin(); it != rankings.end(); it++) {
		sprintf(txtInfoJugador, "%02d           %-10.10s", (i + 1), listJugadores[it->second]->Nombre.c_str());
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
	cout << "unmutex2 modoGrafico listJugadores" << endl;

	SDL_BlitSurface(background, NULL, screen, &posBackground);
	SDL_Flip(screen);

	pthread_exit(NULL);
}

/**
 * THREAD -> KEEPALIVE Jugadores - Actualiza la lista de jugadores quitando los que ya no estan activos
 */
void* keepAliveJugadores(void*) {
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	bzero(buffer, sizeof(buffer));
	int readDataCode;

	string message(CD_ACK);
	string content;
	message.append(fillMessage(content));

	bool partidasTerminadas = false;
	while (!partidasTerminadas) {
		//para cada jugador ver si me responden la señal de KEEPALIVE
		cout << "mutex1 keepAliveJugadores listJugadores" << endl;
		pthread_mutex_lock(&mutex_listJugadores);
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {

			it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
			readDataCode = it->second->SocketAsociado->ReceiveBloq(buffer, (LONGITUD_CODIGO + LONGITUD_CONTENIDO));

			cout << "0000000000000-----  readDataCode: " << readDataCode << endl;
			if (readDataCode == 0) {
				//el jugador se desconecto
				cout << "0000000000000-----  Voy a eliminar a: " << it->first << endl;
				quitarJugador(it->first);
			} else {

			}
		}
		pthread_mutex_unlock(&mutex_listJugadores);
		cout << "unmutex1 keepAliveJugadores listJugadores" << endl;
		usleep(1000000);
		pthread_mutex_lock(&mutex_todasLasPartidasFinalizadas);
		cout << "mutex2 keepAliveJugadores todasLasPartidasFinalizadas" << endl;
		partidasTerminadas = todasLasPartidasFinalizadas;
		pthread_mutex_unlock(&mutex_todasLasPartidasFinalizadas);
		cout << "unmutex2 keepAliveJugadores todasLasPartidasFinalizadas" << endl;
	}

	pthread_exit(NULL);
}

/**
 * THREAD -> Aceptar las conexiones de nuevos jugadores al torneo
 */
void* aceptarJugadores(void* data) {
	int clientId = 0;
	//Crear Socket del Servidor
	cout << "aceptar jugadpres" << endl;
	ServerSocket sSocket(puertoTorneo);
	char nombreJugador[LONGITUD_CODIGO + LONGITUD_CONTENIDO];

	while (true) {
		cout << "va a bloquearse esperando al JUGADOR" << endl;
		cout << "_____________________" << endl;
		CommunicationSocket * cSocket = sSocket.Accept();
		cout << "va a bloquearse esperando mensaje" << endl;
		cout << "_____________________" << endl;
		cSocket->ReceiveBloq(nombreJugador, (LONGITUD_CODIGO + LONGITUD_CONTENIDO));
		clientId++;
		agregarJugador(new Jugador(clientId, nombreJugador, cSocket));
		cout << "Se agrega el jugador NRO:" << clientId << " NOMBRE: " << nombreJugador << endl;
		cout << "______________________________________________________________________________" << endl;
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
	string nombreSemaforo;
	int i = 1;
	while (!torneoFinalizado()) {
		cout << "pasada de busqueda nro: " << i++ << " PID:" << getpid() << endl;

		//recorro la lista de jugadores viendo a quien le puedo asignar un oponente y que comienze la partida
		cout << "mutex establecerPartidas" << endl;
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

				puertoPartida++;

				key_t key = ftok("/bin/ls", puertoPartida);
				if (key == -1) {
					cout << "Error al generar clave de memoria compartida" << endl;
					break;
				}
				int idShm = shmget(key, sizeof(struct puntajesPartida) * 1, IPC_CREAT | PERMISOS_SHM);
				if (idShm == -1) {
					cout << "Error al obtener memoria compartida" << endl;
					break;
				}
				cout << "SERV TORNEO Partida: " << puertoPartida << "  ID SHM: " << idShm << endl;

				//inicializo el BLOQUE DE SHM
				puntajesPartida* resumenPartida = (struct puntajesPartida *) shmat(idShm, (char *) 0, 0);
				resumenPartida->idJugador1 = idJugador;
				resumenPartida->idJugador2 = idOponente;
				resumenPartida->jugando = true;
				resumenPartida->keepAlivePartida = true;
				resumenPartida->keepAliveTorneo = true;
				resumenPartida->puntajeJugador1 = 0;
				resumenPartida->puntajeJugador2 = 0;

				//genero y cargo los datos de la partida en una lista
				datosPartida structuraDatosPartida;
				structuraDatosPartida.idShm = idShm;
				structuraDatosPartida.lecturasFallidasSHM_Partida = 0;

				string nombreSemaforoPartida = "/" + intToString(puertoPartida) + "_Partida";
				string nombreSemaforoTorneo = "/" + intToString(puertoPartida) + "_Torneo";
				Semaforo* semaforoPartida = new Semaforo(nombreSemaforoPartida.c_str(), 1);
				Semaforo* semaforoTorneo = new Semaforo(nombreSemaforoTorneo.c_str(), 0);
				structuraDatosPartida.semaforo_pointerSem_t_Partida = semaforoPartida->getSem_t();
				structuraDatosPartida.semaforo_pointerSem_t_Torneo = semaforoTorneo->getSem_t();

				cout << "SERV TORNEO nombre SEM Torneo" << semaforoTorneo->getName() << endl;
				cout << "SERV TORNEO nombre SEM Partida" << semaforoPartida->getName() << endl;
				cout << "SERV TORNEO Sem_t * semaforo Torneo: " << semaforoTorneo->getSem_t() << endl;
				cout << "SERV TORNEO Sem_t * semaforo Partida: " << semaforoPartida->getSem_t() << endl;

				cout << "mutex establecerPartidas partidasActivas" << endl;
				pthread_mutex_lock(&mutex_partidasActivas);
				partidasActivas.push_back(structuraDatosPartida);
				pthread_mutex_unlock(&mutex_partidasActivas);
				cout << "unmutex establecerPartidas partidasActivas" << endl;

				//Le mando a los jugadores el nro de Puerto en el que comenzara la partida
				char auxPuertoNuevaPartida[LONGITUD_CONTENIDO];
				sprintf(auxPuertoNuevaPartida, "%d", puertoPartida);
				string message(CD_PUERTO_PARTIDA);
				message.append(fillMessage(auxPuertoNuevaPartida));

				cout << "Puerto para partida " << message << endl;
				listJugadores[idJugador]->SocketAsociado->SendNoBloq(message.c_str(), message.length());
				listJugadores[idOponente]->SocketAsociado->SendNoBloq(message.c_str(), message.length());

				if ((pid = fork()) == 0) {
					//Proceso hijo. Hacer exec
					cout << "J" << idJugador << " Crear Partida en el puerto: " << (puertoPartida - 1) << " (" << idJugador << "vs" << idOponente << ")" << endl;

					char auxCantVidas[2];
					sprintf(auxCantVidas, "%d", cantVidas);

					char *argumentos[] = { auxPuertoNuevaPartida, auxCantVidas, NULL };
					execv("../Servidor Partida/Debug/Servidor Partida", argumentos);
					cout << "ERROR al ejecutar execv Nueva Partida" << endl;
					exit(1);
				} else if (pid < 0) {
					//Hubo error
					cout << "Error al forkear" << endl;
				} else {
					//Soy el padre.
				}
			} else {
				//cout << "J" << idJugador << " No puede jugar" << endl;
			}
		}
		pthread_mutex_unlock(&mutex_listJugadores);
		cout << "unmutex establecerPartidas" << endl;

		usleep(INTERVALO_ENTRE_BUSQUEDA_DE_OPONENTES);
	}

	cout << "Thread EstablecerPartidas va a hacer un Exit" << endl;
	pthread_exit(NULL);
}

void mandarPuntajes() {
	//mandar a cada cliente su puntaje y ranking
	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		string message(CD_RANKING);
		message.append(fillMessage(intToString(it->second->Ranking)));
		it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
	}
	pthread_mutex_unlock(&mutex_listJugadores);
}

void liberarRecursos() {
	//SOCKETS
	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		delete (it->second->SocketAsociado);
	}
	pthread_mutex_unlock(&mutex_listJugadores);

	//SHM
	pthread_mutex_lock(&mutex_partidasActivas);
	for (list<datosPartida>::iterator it = partidasActivas.begin(); it != partidasActivas.end(); it++) {

		/**
		 * ver como borrar la SHM
		 */

		//shmdt ((char *)Memoria);
		//shmctl (Id_Memoria, IPC_RMID, (struct shmid_ds *)NULL);

		//shmdt((char *) it);
		//shmctl(it, IPC_RMID, (struct shmid_ds *) NULL);
	}
	pthread_mutex_unlock(&mutex_partidasActivas);

	//Semaphores
	// .close();

	//Mutex
	pthread_mutex_destroy(&mutex_timeIsUp);
	pthread_mutex_destroy(&mutex_comenzoConteo);
	pthread_mutex_destroy(&mutex_todasLasPartidasFinalizadas);
	pthread_mutex_destroy(&mutex_partidasActivas);
	pthread_mutex_destroy(&mutex_listJugadores);

	//SDL
	SDL_FreeSurface(screen);
	SDL_FreeSurface(background);
	SDL_FreeSurface(tiempo);
	SDL_FreeSurface(jugadores);
	SDL_FreeSurface(infoJugador_part1);
	SDL_FreeSurface(infoJugador_part2);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
}

/*
 void asociarSegmento(int* idShm, int* variable) {
 //int idShm;
 //int *variable = NULL;

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
 }*/

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
