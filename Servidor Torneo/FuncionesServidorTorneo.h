#ifndef FUNCIONESSERVIDORTORNEO_H_
#define FUNCIONESSERVIDORTORNEO_H_

#include "Jugador.h"
#include <pthread.h>
#include <list>

#define PERMISOS_SHM 0777
#define CLAVE_MEMORIA_COMPARTIDA 1322

//extern pthread_mutex_t mutex_puerto;
extern pthread_mutex_t mutex_listJugadores;
extern map<int, Jugador*> listJugadores;
extern unsigned int puerto;
extern int cantVidas;

struct thTemporizador_data {
	bool timeIsUp;
	int duracion;
	pthread_t thEstablecerPartidas;
};

void getConfiguration(unsigned int* port, string* ip, int* duracionTorneo, int* tiempoInmunidad, int* cantVidas);
void SIGINT_Handler(int inum);
void agregarJugador(Jugador* nuevoJugador);
void quitarJugador(int id);
unsigned int getPort();
unsigned int getNewPort();
void asociarSegmento(int* idShm, int* variable);
//THREADS
void* temporizadorTorneo(void* data);
void* establecerPartidas(void* data);

//AUXILIARES
string intToString(int number);

#endif /* FUNCIONESSERVIDORTORNEO_H_ */
