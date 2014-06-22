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

extern unsigned int puertoTorneo;
Semaforo sem_inicializarTemporizador((char*) "/sem_inicializarTemporizador", 0);
pthread_mutex_t mutex_puertoPartida = PTHREAD_MUTEX_INITIALIZER;
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
	pthread_mutex_lock(&mutex_listJugadores);
	//para cada participante le quito de su lista el jugador que se da de baja
	for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
		it->second->quitarJugador(id);
	}
	//lo quito de la lista de jugadores del torneo
	listJugadores.erase(id);
	pthread_mutex_unlock(&mutex_listJugadores);
}

/**
 * Obtener un nuevo puerto para asginarlo a una nueva partida
 */
unsigned int getNewPort() {
	pthread_mutex_lock(&mutex_puertoPartida);
	puertoPartida++;
	int nuevoPuerto = puertoPartida;
	pthread_mutex_unlock(&mutex_puertoPartida);
	return nuevoPuerto;
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
	sleep(torneo->duracion * 60);

	pthread_cancel(torneo->thAceptarJugadores);
	pthread_cancel(torneo->thEstablecerPartidas);
	pthread_exit(NULL);
}

/**
 * THREAD -> Aceptar las conexiones de nuevos jugadores al torneo
 */
void* aceptarJugadores(void* data) {
	string* ip = (string*) data;
	int clientId = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	//FIN CODIGO PARA PROBAR EL ASIGNADOR DE PARTIDAS
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	//Crear Socket del Servidor
	ServerSocket sSocket(puertoTorneo, (char *) (*ip).c_str());
	char * nombreJugador = NULL;
	while (true) {
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
	while (true) {
		cout << "pasada de busqueda nro: " << i++ << endl;

		/*
		 pthread_mutex_lock(&mutex_listJugadores);
		 for (map<int, Jugador*>::iterator it = (listJugadores).begin(); it != (listJugadores).end(); it++) {
		 cout << (*(*it).second).Nombre << " - partidas:" << endl;

		 for (map<int, int>::iterator itmap = (*(*it).second).Partidas.begin(); itmap != (*(*it).second).Partidas.end(); ++itmap) {
		 std::cout << itmap->first << " => " << itmap->second << '\n';
		 }
		 }
		 pthread_mutex_unlock(&mutex_listJugadores);
		 */

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
				//		listJugadores[idJugador]->SocketAsociado->SendNoBloq(message.c_str(), sizeof(message.c_str()));
				//		listJugadores[idOponente]->SocketAsociado->SendNoBloq(message.c_str(), sizeof(message.c_str()));

				if ((pid = fork()) == 0) {
					//Proceso hijo. Hacer exec
					cout << "J" << idJugador << " Crear Partida en el puerto: " << puertoNuevaPartida << " (" << idJugador << "vs" << idOponente << ")" << endl;

					char auxCantVidas[2];
					sprintf(auxCantVidas, "%d", cantVidas);

					char *argumentos[] = { auxPuertoNuevaPartida, auxCantVidas };
					//		execv("/Servidor Partida/Servidor Partida", argumentos);
					exit(1);
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
		usleep(10000000);
	}

	pthread_exit(NULL);
}

void asociarSegmento(int* idShm, int* variable) {
	key_t key = ftok("/bin/ls", CLAVE_MEMORIA_COMPARTIDA);
	if (key == -1) {
		cout << "Error al generar clave de memoria compartida" << endl;
		exit(1);
	}

	*idShm = shmget(key, sizeof(int) * 1, IPC_CREAT | PERMISOS_SHM);
	if (*idShm == -1) {
		cout << "Error al obtener memoria compartida" << endl;
		exit(1);
	}

	variable = (int*) shmat(*idShm, 0, 0);
	if (variable == NULL) {
		cout << "Error al asignar memoria compartida reservada" << endl;
		exit(1);
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
