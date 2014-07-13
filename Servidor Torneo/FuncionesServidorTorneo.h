#ifndef FUNCIONESSERVIDORTORNEO_H_
#define FUNCIONESSERVIDORTORNEO_H_

#include "Support/ConstantesServidorTorneo.h"
#include "Clases/Jugador.h"
#include <pthread.h>
#include <list>
#include <semaphore.h>

struct thTemporizador_data {
	int duracion;
	pthread_t thAceptarJugadores;
	pthread_t thEstablecerPartidas;
};

struct thModoGrafico_data {
	int duracion;
};


struct puntajesPartida {
	int idJugador1;
	int idJugador2;
	int puntajeJugador1;
	int puntajeJugador2;
	bool partidaFinalizadaOK;
};

struct datosPartida {
	int idShm;
	pid_t pidPartida;
};

void getConfiguration(unsigned int* port, string* ip, int* duracionTorneo, int* tiempoInmunidad, int* cantVidas);
void SIG_Handler(int inum);
void SIG_CHLD(int inum);
void agregarJugador(Jugador* nuevoJugador);
void quitarJugador(int id);
bool torneoFinalizado();
bool partidasFinalizadas();
void mandarPuntajes();
void liberarRecursos();
void mostrarPantalla(const char*);
bool seguirAceptandoNuevosJugadores();
//THREADS
void* temporizadorTorneo(void* data);
void* lecturaDeResultados(void* data);
void* keepAliveJugadores(void*);
void* modoGrafico(void*);
void* aceptarJugadores(void* data);
void* establecerPartidas(void* data);

//AUXILIARES
string fillMessage(string message);
string intToString(int number);

#endif /* FUNCIONESSERVIDORTORNEO_H_ */
