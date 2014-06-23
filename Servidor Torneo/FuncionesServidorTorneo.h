#ifndef FUNCIONESSERVIDORTORNEO_H_
#define FUNCIONESSERVIDORTORNEO_H_

#include "Support/ConstantesServidorTorneo.h"
#include "Jugador.h"
#include <pthread.h>
#include <list>

extern pthread_mutex_t mutex_listJugadores;
extern map<int, Jugador*> listJugadores;
extern unsigned int puertoPartida;
extern int cantVidas;

struct thTemporizador_data {
	int duracion;
	pthread_t thAceptarJugadores;
	pthread_t thEstablecerPartidas;
};

void getConfiguration(unsigned int* port, string* ip, int* duracionTorneo, int* tiempoInmunidad, int* cantVidas);
void SIGINT_Handler(int inum);
void agregarJugador(Jugador* nuevoJugador);
void quitarJugador(int id);
unsigned int getNewPort();
bool torneoFinalizado();
void asociarSegmento(int* idShm, int* variable);
//THREADS
void* temporizadorTorneo(void* data);
void* aceptarJugadores(void* data);
void* establecerPartidas(void* data);

//AUXILIARES
string fillMessage(string message);
string intToString(int number);

#endif /* FUNCIONESSERVIDORTORNEO_H_ */
