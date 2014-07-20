#ifndef FUNCIONESSERVIDORTORNEO_H_
#define FUNCIONESSERVIDORTORNEO_H_

#include "Clases/ServerSocket.h"
#include "Support/ConstantesServidorTorneo.h"
#include "Support/Estructuras.h"
#include "Clases/Jugador.h"
#include <pthread.h>
#include <list>
#include <signal.h>
#include <semaphore.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

void getConfiguration(unsigned int* port, string* ip, int* duracionTorneo, int* tiempoInmunidad, int* cantVidas);
void SIG_INT(int inum);
void SIG_CHLD(int inum);
void SIG_PIPE(int inum);
void agregarJugador(Jugador* nuevoJugador);
void quitarJugador(int id);
bool torneoFinalizado();
bool partidasFinalizadas();
void mandarPuntajes();
void liberarRecursos();
void mostrarPantalla(const char*);
bool seguirAceptandoNuevosJugadores();
bool murioServidorDeLaPartida();
int quienJugoMenos();
bool hayPartidasActivas();

//THREADS
void* temporizadorTorneo(void* data);
void* lecturaDeResultados(void* data);
void* keepAliveJugadores(void*);
void* modoGrafico(void*);
void* aceptarJugadores(void* data);
void* establecerPartidas(void* data);
void* receiver(void * data);

//AUXILIARES
string fillMessage(string message);
string intToString(int number);

extern map<int,Jugador*> listJugadores;
extern unsigned int puertoServidorTorneo;
extern int cantVidas;
extern int idSHM;
extern ServerSocket* sSocket;
extern SDL_Surface *screen, *background;
extern SDL_Rect posBackground;
extern TTF_Font *font;
extern SDL_Color colorNegro, colorBlanco;
extern sigset_t blockmask;

#endif /* FUNCIONESSERVIDORTORNEO_H_ */
