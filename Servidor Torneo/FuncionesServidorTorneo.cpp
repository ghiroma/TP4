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
pthread_mutex_t mutex_seguirAceptandoJugadores = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex_murioServidorPartida = PTHREAD_MUTEX_INITIALIZER;

//extern pthread_mutex_t mutex_listJugadores;
		extern map<int,
Jugador*> listJugadores;
//extern unsigned int puertoServidorPartida;
extern unsigned int puertoServidorTorneo;
extern int cantVidas;
extern int idSHM;
map<unsigned int, datosPartida> partidasActivas;
extern ServerSocket* sSocket;
int idPartida = 0;
puntajesPartida * resumenPartida;
bool seguirAceptandoJugadores = true;
//bool murioServidorPartida = false;
bool servidorTorneoSIGINT = false;

extern SDL_Surface *screen, *background;
SDL_Surface *tiempo, *jugadores, *infoJugador_part1, *infoJugador_part2;
SDL_Rect posDestino, posTiempo, posJugadores;
extern SDL_Rect posBackground;
extern TTF_Font *font;

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
void SIG_INT(int inum) {
	servidorTorneoSIGINT = true;
	cout << "Torneo: Señal SIGINT" << endl;
	exit(1);
}
void SIG_PIPE(int inum) {
	cout << "Torneo: Señal SIGPIPE" << endl;
}
/*
 void inicilizarMapPartidasActivas() {
 int puertoPartida = puertoServidorTorneo + 1;
 datosPartida datosNuevaPartida;
 datosNuevaPartida.pidPartida = 0;
 datosNuevaPartida.libre = true;

 pthread_mutex_lock(&mutex_partidasActivas);
 //for (int i = 0; i < MAX_PUERTOS_UTILIZABLES; ++i) {
 //	partidasActivas[puertoPartida++] = datosNuevaPartida;
 //}

 partidasActivas[puertoPartida] = datosNuevaPartida;
 pthread_mutex_unlock(&mutex_partidasActivas);
 }
 */

/**
 * Devuelve un puerto que no este siendo utilizado para crear la partida
 */
unsigned int encontrarPuertoLibreParaPartida() {
	unsigned int puertoDePartidaLibre;
	bool encontroPuertoDisponible = false;
	unsigned int ultimoPuertoUtilizado;

	pthread_mutex_lock(&mutex_partidasActivas);
	/*
	 //limpiar lista de puertos fuera del maximo aceptado
	 if (partidasActivas.size() > MAX_PUERTOS_UTILIZABLES) {
	 map<unsigned int, datosPartida>::iterator it1;
	 it1 = partidasActivas.begin();
	 advance(it1, (MAX_PUERTOS_UTILIZABLES));
	 do {
	 if (it1->second.libre) {
	 partidasActivas.erase(it1++);
	 } else {
	 ++it1;
	 }
	 } while (it1 != partidasActivas.end());

	 }*/

	if (partidasActivas.size() > 0) {
		//busco en la lista algun puerto que no este siendo usado
		for (map<unsigned int, datosPartida>::iterator it = partidasActivas.begin(); it != partidasActivas.end(); it++) {
			if (it->second.libre) {
				puertoDePartidaLibre = it->first;
				it->second.libre = false;
				it->second.pidPartida = 0;
				encontroPuertoDisponible = true;
				break;
			}
		}
		ultimoPuertoUtilizado = partidasActivas.rbegin()->first;
	} else {
		ultimoPuertoUtilizado = puertoServidorTorneo;
	}

	//si no encontro ninguno agrego uno mas
	if (!encontroPuertoDisponible) {
		//unsigned int ultimoPuertoUtilizado = partidasActivas.rbegin()->first;
		unsigned int nuevoPuerto = (ultimoPuertoUtilizado + 1);
		datosPartida datosNuevaPartida;
		datosNuevaPartida.pidPartida = 0;
		datosNuevaPartida.libre = false;

		partidasActivas[nuevoPuerto] = datosNuevaPartida;
		puertoDePartidaLibre = nuevoPuerto;
	}
	pthread_mutex_unlock(&mutex_partidasActivas);

	return puertoDePartidaLibre;
}

/**
 * Detecta cuando un Servidor Partida termina
 */
void SIG_CHLD(int inum) {
	int childpid = wait(NULL);
	cout << "Señal Handler SIGCHLD - PID:" << childpid << endl;

	//habilito el puerto la lista de partidas activas
	pthread_mutex_lock(&mutex_partidasActivas);
	int idJugador1 = 0;
	int idJugador2 = 0;
	for (map<unsigned int, datosPartida>::iterator it = partidasActivas.begin(); it != partidasActivas.end(); it++) {
		if (it->second.pidPartida == childpid) {
			idJugador1 = it->second.idJugador1;
			it->second.idJugador1 = 0;
			idJugador2 = it->second.idJugador2;
			it->second.idJugador2 = 0;
			it->second.pidPartida = 0;
			it->second.libre = true;
			//partidasActivas.erase(it);
			break;
		}
	}
	pthread_mutex_unlock(&mutex_partidasActivas);

	pthread_mutex_lock(&mutex_listJugadores);
	if (listJugadores.count(idJugador1) == 1) {
		listJugadores[idJugador1]->Jugando = false;
	}
	if (listJugadores.count(idJugador2) == 1) {
		listJugadores[idJugador2]->Jugando = false;
	}
	pthread_mutex_unlock(&mutex_listJugadores);

	/*//MUERTE DE LA PARTIDA
	 int childpid = wait(NULL);

	 if (cantPartidasFinalizadas != idPartida || (cantPartidasFinalizadas == 0 && idPartida == 0)) {
	 pthread_mutex_lock(&mutex_murioServidorPartida);
	 murioServidorPartida = true;
	 pthread_mutex_unlock(&mutex_murioServidorPartida);

	 mostrarPantalla("servermatchdown");
	 sleep(5);
	 exit(1);
	 }
	 //exit(1);

	 if (servidorTorneoSIGINT == true) {
	 exit(1);
	 }*/
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

/**
 * Verificar si murio el servidor de partida
 */
/*bool murioServidorDeLaPartida() {
 bool estado;
 pthread_mutex_lock(&mutex_murioServidorPartida);
 estado = murioServidorPartida;
 pthread_mutex_unlock(&mutex_murioServidorPartida);
 return estado;
 }*/

/////////////////////////////// THREADS ////////////////////////////
/**
 * THREAD -> Controla el tiempo que debe durar el torneo
 */
void* temporizadorTorneo(void* data) {
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

	/*
	 * no va porque da error en el cliente y este no sabe cuando muere mal el torneo
	 * if (sSocket != NULL) {
	 cout << "(torneo finalizo tiempo) Torneo -> cierro socket" << endl;
	 delete (sSocket);
	 sSocket = NULL;
	 }*/

	//el tiempo del Torneo llego a su fin, informar a cada cliente
	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		string message(CD_FIN_TORNEO);
		message.append(fillMessage("1"));
		if (it->second != NULL && it->second->SocketAsociado != NULL) {
			it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
		}
	}
	pthread_mutex_unlock(&mutex_listJugadores);

	pthread_mutex_lock(&mutex_seguirAceptandoJugadores);
	seguirAceptandoJugadores = false;
	pthread_mutex_unlock(&mutex_seguirAceptandoJugadores);
	//pthread_cancel(torneo->thAceptarJugadores);

	//SOCKETS
	cout << "Elimino el socket Torneo para que no quede en LISTEN" << endl;
	if (sSocket != NULL) {
		delete (sSocket);
		sSocket = NULL;
	}
	cout << "socket Torneo ELIMINADO" << endl;
	pthread_exit(NULL);
}

/**
 * THREAD -> Lee los resultados de las partidas
 */
void* lecturaDeResultados(void* data) {
	resumenPartida = (struct puntajesPartida *) shmat(idSHM, (char *) 0, 0);

	while (hayPartidasActivas() || !torneoFinalizado() || sem_ServidorTorneoSHM.getValue() != 0) {
		//cout<<"Valor semaforo: "<<sem_ServidorTorneoSHM.getValue()<<endl;
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
				//listJugadores[resumenPartida->idJugador1]->Jugando = false;
			}
			if (listJugadores.count(resumenPartida->idJugador2) == 1) {
				//listJugadores[resumenPartida->idJugador2]->Jugando = false;
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
			//cantPartidasFinalizadas++;
			/*if (cantPartidasFinalizadas == idPartida && torneoFinalizado()) {
			 break;
			 }*/
			sem_ServidorPartidaSHM.V();
		}
		usleep(50000);
	}

	/*//en este punto ya se que todas las partidas finalizaron y el tiempo de torneo tambien
	 if (!murioServidorDeLaPartida()) {
	 kill(pidServidorPartida, SIGINT);
	 }

	 pthread_mutex_lock(&mutex_todasLasPartidasFinalizadas);
	 todasLasPartidasFinalizadas = true;
	 pthread_mutex_unlock(&mutex_todasLasPartidasFinalizadas);*/

	cout << "Termina thread lectura de resultados" << endl;
	pthread_exit(NULL);
}

/**
 * THREAD -> Modo Grafico
 */
SDL_Color colorNegro, colorBlanco;
int minutos;
int segundos;
char txtTiempo[10];
char txtPlayers[10];
char txtInfoJugador[MAX_LENGT_TXT_INFO_JUGADOR];
int cantPlayersConectados;
multimap<float, int> rankings;

void* modoGrafico(void* data) {
	struct thModoGrafico_data *torneo;
	torneo = (struct thModoGrafico_data *) data;

	minutos = torneo->duracion;
	segundos = 0;
	posTiempo.x = 50;
	posTiempo.y = 140;
	//char txtTiempo[10];
	sprintf(txtTiempo, "TIME %02d:%02d", minutos, segundos);
	tiempo = TTF_RenderText_Solid(font, txtTiempo, colorBlanco);
	SDL_BlitSurface(tiempo, NULL, background, &posTiempo);

	posJugadores.x = 530;
	posJugadores.y = 140;
	//char txtPlayers[10];
	sprintf(txtPlayers, "Players: %d", 0);
	jugadores = TTF_RenderText_Solid(font, txtPlayers, colorBlanco);
	SDL_BlitSurface(jugadores, NULL, background, &posJugadores);

	posBackground.x = 0;
	posBackground.y = 0;
	SDL_BlitSurface(background, NULL, screen, &posBackground);
	SDL_Flip(screen);

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
			break;
		}
		usleep(TIEMPO_DE_REDIBUJADO);
	}

	//esperar que todas las partidas finalicen
	//while (!partidasFinalizadas()) {
	//	sleep(5);
	//}

	//esperar que todas las partidas finalicen
	//int cantPartidasActivas;

	while (hayPartidasActivas()) {
		sleep(2);
		//cout<<"Modo grafico: hay partidas activas"<<endl;
	}

	/*bool partidasFinalizadas = false;
	 int cantPartidasFinalizadas = 0;
	 while (!partidasFinalizadas) {
	 pthread_mutex_lock(&mutex_partidasActivas);
	 for(map<unsigned int, datosPartida>::iterator it=partidasActivas.begin();it!=partidasActivas.end();it++)
	 {
	 if(it->second.libre)
	 {
	 cantPartidasFinalizadas++;
	 }
	 }

	 cout<<"Cantidad de partidas finalizadas: "<<cantPartidasFinalizadas<<endl;
	 cout<<"Cantidad de partidas activas: "<<partidasActivas.size()<<endl;

	 if(cantPartidasFinalizadas==partidasActivas.size())
	 {
	 partidasFinalizadas = true;
	 }
	 pthread_mutex_unlock(&mutex_partidasActivas);
	 cantPartidasFinalizadas = 0;
	 sleep(2);
	 }*/

	pthread_mutex_lock(&mutex_todasLasPartidasFinalizadas);
	//cout << "mutex  modoGrafico todasLasPartidasFinalizadas" << endl;
	todasLasPartidasFinalizadas = true;
	pthread_mutex_unlock(&mutex_todasLasPartidasFinalizadas);
	//cout << "unmutex modoGrafico todasLasPartidasFinalizadas" << endl;

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
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	bzero(buffer, sizeof(buffer));
	int readDataCode;

	string message(CD_ACK);
	string content;
	message.append(fillMessage(content));

	//while (!partidasFinalizadas() && !murioServidorDeLaPartida()) {
	while (!partidasFinalizadas()) {
		//Para cada jugador ver si me responden la señal de KEEPALIVE
		pthread_mutex_lock(&mutex_listJugadores);
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			if (it->second->SocketAsociado != NULL) {
				it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
				//readDataCode = it->second->SocketAsociado->ReceiveBloq(buffer, (LONGITUD_CODIGO + LONGITUD_CONTENIDO));
			}
			//if (readDataCode == 0) {
			//El jugador se desconecto
			//quitarJugador(it->first);
			//} else {

			//}
		}
		pthread_mutex_unlock(&mutex_listJugadores);

		usleep(TIEMPO_DE_KEEPALIVEJUGADORES);
	}

	pthread_exit(NULL);
}

void* receiver(void *) {
	while (!partidasFinalizadas()) {
		char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];

		pthread_mutex_lock(&mutex_listJugadores);
		for (map<int, Jugador *>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			if (it->second != NULL && it->second->SocketAsociado != NULL) {
				int readDataCode = it->second->SocketAsociado->ReceiveNoBloq(buffer, (LONGITUD_CODIGO + LONGITUD_CONTENIDO));
				if (readDataCode == 0) {
					quitarJugador(it->second->Id);
					//delete (it->second->SocketAsociado);
					//listJugadores.erase(it);
				}
			}
		}
		pthread_mutex_unlock(&mutex_listJugadores);
		usleep(50000);
	}
}
/**
 * THREAD -> Aceptar las conexiones de nuevos jugadores al torneo
 */
void* aceptarJugadores(void* data) {
	//cout << "Thread aceptarJugadores - PID:" << getpid() << endl;
	int clientId = 0;
	/*//Crear Socket del Servidor
	 try {
	 sSocket = new ServerSocket(puertoServidorTorneo);
	 } catch (...) {
	 cout << "No se pudo conectar a la IP solicitada. Pruebe otra." << endl;
	 exit(1);
	 }*/

	char nombreJugador[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	char aux[LONGITUD_CONTENIDO];

	while (seguirAceptandoNuevosJugadores()) {
		try {
			strcpy(aux, "");
			//cout << "va a bloquearse esperando al JUGADOR" << endl;
			CommunicationSocket * cSocket = sSocket->Accept();
			//cout << "va a bloquearse esperando mensaje" << endl;
			cSocket->ReceiveBloq(nombreJugador, (LONGITUD_CODIGO + LONGITUD_CONTENIDO));
			//cout << "se conecto:" << nombreJugador << endl;
			clientId++;

			//Mandarle el ID al Jugador
			string messageId(CD_ID_JUGADOR);
			sprintf(aux, "%d", clientId);
			messageId.append(fillMessage(aux));
			cSocket->SendNoBloq(messageId.c_str(), messageId.length());

			//Madarle puerto partida
			/*string messagePuertoPartida(CD_PUERTO_PARTIDA);
			 sprintf(aux, "%d", puertoServidorPartida);
			 messagePuertoPartida.append(fillMessage(aux));
			 cSocket->SendNoBloq(messagePuertoPartida.c_str(), messagePuertoPartida.length());*/

			agregarJugador(new Jugador(clientId, nombreJugador, cSocket));
			//cout << "Se agrega el jugador NRO:" << clientId << " NOMBRE: " << nombreJugador << endl;

		} catch (...) {
			//cout<<"Error en accept"<<endl;
			break;
		}
	}

	pthread_exit(NULL);
}

/**
 * THREAD -> Crea los servidores de partidas (un solo serv partida)
 */
/*void* establecerPartidas(void* data) {
 int idJugador;
 int idOponente;

 while (!torneoFinalizado() && !murioServidorDeLaPartida()) {
 //recorro la lista de jugadores viendo a quien le puedo asignar un oponente y que comienze la partida
 pthread_mutex_lock(&mutex_listJugadores);
 for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
 idJugador = it->first;
 idOponente = it->second->obtenerOponente();

 //si no se encuentra jugando actualmente y se encontro un oponente ("crea" una nueva partida)
 if (idOponente >= 1) {
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

 listJugadores[idJugador]->SocketAsociado->SendBloq(message.c_str(), message.length());
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

 listJugadores[idJugador]->SocketAsociado->SendBloq(nombreOponente1.c_str(), nombreOponente1.length());
 listJugadores[idOponente]->SocketAsociado->SendBloq(nombreOponente2.c_str(), nombreOponente2.length());

 } else {
 //No se encontraron oponentes disponibles
 }
 }
 pthread_mutex_unlock(&mutex_listJugadores);
 usleep(INTERVALO_ENTRE_BUSQUEDA_DE_OPONENTES);
 }
 //cout << "Thread EstablecerPartidas va a hacer un Exit" << endl;
 pthread_exit(NULL);
 }*/

/**
 * THREAD -> Crea los servidores de partidas
 */
void* establecerPartidas(void* data) {
	cout << "Thread establecerPartidas - PID:" << getpid() << endl;
	pid_t pid;
	int idJugador;
	int idOponente;
	int nroPartida = 1;
	string nombreSemaforo;
	int i = 1;
	while (!torneoFinalizado()) {
		//cout << "pasada de busqueda nro: " << i++ << " PID:" << getpid() << endl;

		//recorro la lista de jugadores viendo a quien le puedo asignar un oponente y que comienze la partida
		pthread_mutex_lock(&mutex_listJugadores);
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			idJugador = it->first;
			idOponente = it->second->obtenerOponente();

			//si no se encuentra jugando actualmente y se encontro un oponente lanzo el servidor de partida
			if (idOponente > 0) {
				//habilito el temporizador del torneo
				if (nroPartida == 1) {
					//cout << "Se crea la primer partida y doy permiso a iniciar el temporizador" << endl;
					sem_inicializarTemporizador.V();
				}
				nroPartida++;

				int puertoServidorPartida = encontrarPuertoLibreParaPartida();

				//Le mando a los jugadores el nro de Puerto en el que comenzara la partida
				char auxPuertoNuevaPartida[LONGITUD_CONTENIDO];
				sprintf(auxPuertoNuevaPartida, "%d", puertoServidorPartida);
				string message(CD_PUERTO_PARTIDA);
				message.append(fillMessage(auxPuertoNuevaPartida));
				//cout << "le mando a ID: " << idJugador << " - el socket:" << auxPuertoNuevaPartida << endl;
				listJugadores[idJugador]->SocketAsociado->SendBloq(message.c_str(), message.length());
				//cout << "le mando a ID: " << idOponente << " - el socket:" << auxPuertoNuevaPartida << endl;
				listJugadores[idOponente]->SocketAsociado->SendBloq(message.c_str(), message.length());

				cout << "SERVER TORNEO: Utilizando socket " << puertoServidorPartida << endl;

				//Les mando el nombre de su oponente
				char auxnombreOponente1[LONGITUD_CONTENIDO];
				sprintf(auxnombreOponente1, "%s", listJugadores[idOponente]->Nombre.c_str());
				string nombreOponente1(CD_NOMBRE);
				nombreOponente1.append(fillMessage(auxnombreOponente1));
				char auxnombreOponente2[LONGITUD_CONTENIDO];
				sprintf(auxnombreOponente2, "%s", listJugadores[idJugador]->Nombre.c_str());
				string nombreOponente2(CD_NOMBRE);
				nombreOponente2.append(fillMessage(auxnombreOponente2));
				//cout << "le mando a ID: " << idJugador << " - el nombre oponente:" << nombreOponente1 << endl;
				listJugadores[idJugador]->SocketAsociado->SendBloq(nombreOponente1.c_str(), nombreOponente1.length());
				//cout << "le mando a ID: " << idOponente << " - el nombre oponente:" << nombreOponente2 << endl;
				listJugadores[idOponente]->SocketAsociado->SendBloq(nombreOponente2.c_str(), nombreOponente2.length());
				//cout << "Termino de mandar los nombres de oponentes" << endl;

				if ((pid = fork()) == 0) {
					//Proceso hijo
					//cout << "Thread establecerPartidas FORK - PID:" << getpid() << " (" << idJugador << "vs" << idOponente << ") socket:" << auxPuertoNuevaPartida << endl;
					char nombreEjecutable[100];
					strcpy(nombreEjecutable, "ServidorPartida");
					char auxPuertoServidorPartida[LONGITUD_CONTENIDO];
					sprintf(auxPuertoServidorPartida, "%d", puertoServidorPartida);
					char auxCantVidas[2];
					sprintf(auxCantVidas, "%d", cantVidas);

					char *argumentos[] = { nombreEjecutable, auxPuertoServidorPartida, auxCantVidas, NULL };
					execv("../Servidor Partida/ServidorPartida", argumentos);
					cout << "ERROR al ejecutar execv Nueva Partida" << endl;
					exit(1);
				} else if (pid < 0) {
					//Hubo error
					cout << "Error al forkear" << endl;
				} else {
					//Soy el padre.

					pthread_mutex_lock(&mutex_partidasActivas);
					partidasActivas[puertoServidorPartida].pidPartida = pid;
					partidasActivas[puertoServidorPartida].idJugador1 = idJugador;
					partidasActivas[puertoServidorPartida].idJugador2 = idOponente;
					pthread_mutex_unlock(&mutex_partidasActivas);

				}
			} else {
				//cout << "J" << idJugador << " No se le encontraron oponentes" << endl;
			}
		}
		pthread_mutex_unlock(&mutex_listJugadores);
		//cout << "unmutex establecerPartidas" << endl;

		sleep(INTERVALO_ENTRE_BUSQUEDA_DE_OPONENTES);
	}

	//cout << "Thread EstablecerPartidas va a hacer un Exit" << endl;
	pthread_exit(NULL);
}

bool hayPartidasActivas() {
	bool quedanPartidasActivas = false;

	pthread_mutex_lock(&mutex_partidasActivas);
	for (map<unsigned int, datosPartida>::iterator it = partidasActivas.begin(); it != partidasActivas.end(); it++) {
		if (!it->second.libre) {
			quedanPartidasActivas = true;
			break;
		}
	}
	pthread_mutex_unlock(&mutex_partidasActivas);

	return quedanPartidasActivas;
}

void mandarPuntajes() {
	//mandar a cada cliente su puntaje y ranking
	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		string message(CD_RANKING);
		message.append(fillMessage(intToString(it->second->Ranking)));
		it->second->SocketAsociado->SendNoBloq(message.c_str(), message.length());
		delete (it->second);
	}
	listJugadores.clear();
	pthread_mutex_unlock(&mutex_listJugadores);
}

void liberarRecursos() {
	//SOCKETS
	if (sSocket != NULL) {
		delete (sSocket);
		sSocket = NULL;
	}

	pthread_mutex_lock(&mutex_listJugadores);
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		delete (it->second);
	}
	listJugadores.clear();
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
	pthread_mutex_destroy(&mutex_seguirAceptandoJugadores);
	//pthread_mutex_destroy(&mutex_murioServidorPartida);

	//mato las partidas que quedaron activas
	/*pthread_mutex_lock(&mutex_partidasActivas);
	 for (map<unsigned int, datosPartida>::iterator it = partidasActivas.begin(); it != partidasActivas.end(); it++) {
	 kill(pid, SIGINT);
	 }
	 pthread_mutex_unlock(&mutex_partidasActivas);*/

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

bool seguirAceptandoNuevosJugadores() {
	bool estado;
	pthread_mutex_lock(&mutex_seguirAceptandoJugadores);
	estado = seguirAceptandoJugadores;
	pthread_mutex_unlock(&mutex_seguirAceptandoJugadores);
	return estado;
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
