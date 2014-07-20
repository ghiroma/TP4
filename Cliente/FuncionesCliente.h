/*
 * FuncionesCliente.h
 *
 *  Created on: Jul 20, 2014
 *      Author: ghiroma
 */

#include <string>
#include <queue>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "Support/Estructuras.h"
#include "Clases/CommunicationSocket.h"

#ifndef FUNCIONESCLIENTE_H_
#define FUNCIONESCLIENTE_H_

using namespace std;

extern queue<string> cola_grafico;
extern queue<string> cola_ralph;
extern queue<string> cola_pajaro;
extern queue<string> cola_torta;
extern queue<string> cola_mensajes_enviar;
extern queue<string> cola_felix1;
extern queue<string> cola_felix2;

extern SDL_Surface *superficie,
*backgroundImg, *pared_tramo1n1, *pared_tramo2n1, *pared_tramo3n1, *pared, *ventana_sana, *ventana_rota1, *ventana_rota2, *ventana_rota3, *ventana_rota4, *ventana, *puerta, *felix_d1, *felix_i1, *felix_r11, *felix_r21, *felix_r31, *felix_r41, *felix_r51, *felix_d2, *felix_i2, *felix_r12, *felix_r22,
		*felix_r32, *felix_r42, *felix_r52, *felix1, *felix2, *ralph_1, *ralph_2, *ralph_3, *ralph_4, *ralph_5, *ralph_6, *ralph, *roca1, *roca2, *roca, *pajaro_1, *pajaro_2, *pajaro, *texto, *puntos, *vidas, *torta;
extern TTF_Font *fuente;
extern SDL_Color color_texto;
extern SDL_Rect pantalla_juego, pantalla_texto, pantalla_puntos, pantalla_vidas, posBackground;
extern SDL_Surface *superficie,
*backgroundImg, *pared_tramo1n1, *pared_tramo2n1, *pared_tramo3n1, *pared, *ventana_sana, *ventana_rota1, *ventana_rota2, *ventana_rota3, *ventana_rota4, *ventana, *puerta, *felix_d1, *felix_i1, *felix_r11, *felix_r21, *felix_r31, *felix_r41, *felix_r51, *felix_d2, *felix_i2, *felix_r12, *felix_r22,
		*felix_r32, *felix_r42, *felix_r52, *felix1, *felix2, *ralph_1, *ralph_2, *ralph_3, *ralph_4, *ralph_5, *ralph_6, *ralph, *roca1, *roca2, *roca, *pajaro_1, *pajaro_2, *pajaro, *texto, *puntos, *vidas, *torta;

extern unsigned short int tramo;
extern char felix_cartel_puntos[10];
extern char felix_cartel_vidas[10];
extern short int felix1_puntos;
extern short int felix2_puntos;
extern short int felix1_vidas;
extern short int felix2_vidas;
extern short int ventanas_reparadas;
extern bool ventanas_cargadas;
extern struct ventana ventanas_tramo1[3][5];
extern int key_arriba;
extern int key_derecha;
extern int key_abajo;
extern int key_izquierda;
extern int key_accion;
extern int key_salir;
extern string mi_id;
extern string ip;
extern unsigned int puertoServidorPartida;
extern unsigned short int puertoTorneo;

extern struct desplazamiento rocas_desplazamiento[20];
extern struct desplazamiento pajaro_desplazamiento;
extern bool pajaro_moverse;

extern unsigned short int nivel;

extern bool felix1_reparar;
extern bool felix2_reparar;
extern bool felix1_inicial;

extern struct posicion felix2_posicion;
extern struct posicion felix1_posicion;

extern const char* ranking;
extern bool torta_aparece;
extern bool torneoFinalizado;
extern bool showWindowRanking;
extern bool murioServidorTorneo;
extern bool solicitudDeNuevaParitda;
extern bool recibioPosicionInicial;
extern int partidasJugadas;
extern bool start;

extern CommunicationSocket * socketTorneo;
extern CommunicationSocket * socketPartida;

extern string felix1_nombre;
extern string felix2_nombre;

extern short int roca_siguiente;

extern pthread_t thEscuchaTorneo, thEscuchaTeclas, thEscuchaServidor, thEnvioServidor;
extern int resultThEscuchaTorneo, resultThEscuchaTeclas, resultThEscuchaServidor, resultThEnvioServidor;

extern bool salir;

extern struct posicion torta_posicion;
extern struct posicion ralph_posicion;

extern char ralph_sentido;

extern bool ralph_moverse;

extern short int ralph_destino;

extern short int cant_rocas;

extern pthread_mutex_t mutex_msjPuertoRecibido ;
extern pthread_mutex_t mutex_nombreOponente ;
extern pthread_mutex_t mutex_torneoFinalizado ;
extern pthread_mutex_t mutex_solicitudDeNuevaParitda ;
extern pthread_mutex_t mutex_partidaFinalizada ;
extern pthread_mutex_t mutex_murioServidorTorneo ;
extern pthread_mutex_t mutex_cantVidas ;

extern pthread_mutex_t mutex_cola_grafico ;
extern pthread_mutex_t mutex_cola_ralph ;
extern pthread_mutex_t mutex_cola_pajaro ;
extern pthread_mutex_t mutex_cola_torta ;
extern pthread_mutex_t mutex_cola_mensajes_enviar ;
extern pthread_mutex_t mutex_cola_felix1 ;
extern pthread_mutex_t mutex_cola_felix2 ;
extern pthread_mutex_t mutex_mostrar_pantalla ;
extern pthread_mutex_t mutex_start ;
extern pthread_mutex_t mutex_recibioPosicionInicial ;

void handler(int);
void ConfigRect(SDL_Rect *, int, int, int, int);
void Dibujar(int, int, SDL_Surface *, SDL_Surface *);
void DibujarVentanas(struct ventana[][5], unsigned short int, SDL_Surface*);
void CargarVentanasDelTramo(struct ventana*, unsigned short int, unsigned short int, unsigned short int, char, unsigned short int, unsigned short int);
void *EscuchaTeclas(void *);
void *EnvioServidor(void *);
void *EscuchaServidor(void *);
void *EscuchaTorneo(void *);
bool IngresaNombre();
bool CambiaTramo();
bool CambiaNivel();
char ventana_reparada(struct posicion *);
string fillMessage(string);
void PantallaIntermedia(char);
bool hayChoque();
void getConfiguration(unsigned short int* port, string* ip, int* arriba, int* derecha, int* abajo, int* izquierda, int* accion, int* salir);
void mostrarRanking(const char*);
void liberarRecursos();
void mostrarPantalla(const char*);
void vaciarColas();
bool inicializarNuevaPartida();
bool murioServidorDelTorneo();
bool cargarImagenes();
bool torneoFinalizo();
bool nuevaPartidaSolicitada();
void esperarPuertoPartida();
bool esperarNombreOponente();
bool esperarCantVidas();
void esperarSTART();
void esperarPosicionInicial();
void inicializarVariablesDeLaPartida();


#endif /* FUNCIONESCLIENTE_H_ */
