#ifndef FUNCIONESSERVIDORTORNEO_H_
#define FUNCIONESSERVIDORTORNEO_H_

#include "Support/ConstantesServidorTorneo.h"
#include "Jugador.h"
#include <pthread.h>
#include <list>

struct thTemporizador_data {
	int duracion;
	pthread_t thAceptarJugadores;
	pthread_t thEstablecerPartidas;
};

struct thModoGrafico_data {
	int duracion;
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
void* actualizarListaJugadores(void*);
void* modoGrafico(void*);
void* aceptarJugadores(void* data);
void* establecerPartidas(void* data);
void* keepAlive(void*);

//AUXILIARES
string fillMessage(string message);
string intToString(int number);

#endif /* FUNCIONESSERVIDORTORNEO_H_ */
