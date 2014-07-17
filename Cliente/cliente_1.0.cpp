#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <queue>
#include <algorithm>
#include "Clases/CommunicationSocket.h"
#include "Support/Constantes.h"

#define ANCHO_PANTALLA	640
#define ALTO_PANTALLA 480
#define BPP 8

#define PARED_X 90
#define PARED_Y 110

using namespace std;

bool msjPuertoRecibido = false;
pthread_mutex_t mutex_msjPuertoRecibido = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_nombreOponente = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_torneoFinalizado = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_solicitudDeNuevaParitda = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_partidaFinalizada = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_murioServidorTorneo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cantVidas = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_cola_grafico = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_ralph = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_pajaro = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_torta = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_mensajes_enviar = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_felix1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_felix2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_mostrar_pantalla = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_start = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_recibioPosicionInicial = PTHREAD_MUTEX_INITIALIZER;

struct ventana {
	short int x;
	short int y;
	short int fila;
	short int columna;
	short int numero;
	short int sana;
	char tipo_ventana;
	char ocupado;
};

struct posicion {
	unsigned short int fila;
	unsigned short int columna;
};

struct desplazamiento {
	short int x;
	short int y;
};

queue<string> cola_grafico;
queue<string> cola_ralph;
queue<string> cola_pajaro;
queue<string> cola_torta;
queue<string> cola_mensajes_enviar;
queue<string> cola_felix1;
queue<string> cola_felix2;

void handler(int);
void ConfigRect(SDL_Rect *, int, int, int, int);
void Dibujar(int, int, SDL_Surface *, SDL_Surface *);
void DibujarVentanas(struct ventana[][5], unsigned short int, SDL_Surface*);
void CargarVentanasDelTramo(struct ventana*, unsigned short int, unsigned short int, unsigned short int, char, unsigned short int, unsigned short int);
void *EscuchaTeclas(void *);
void *EnvioServidor(void *);
void *EscuchaServidor(void *);
void *EscuchaTorneo(void *);
char IngresaNombre();
char CambiaTramo();
char CambiaNivel();
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
//bool esperarIdPartida();
void esperarSTART();
void esperarPosicionInicial();
void inicializarVariablesDeLaPartida();

/* 

 0 - Pantalla intermedia para ingresar el nombre.
 1 - Pantalla intermedia de cambio de tramo.
 2 - Pantalla intermedia a la espera de partidas.

 */

SDL_Surface *superficie,
*backgroundImg, *pared_tramo1n1, *pared_tramo2n1, *pared_tramo3n1, *pared, *ventana_sana, *ventana_rota1, *ventana_rota2, *ventana_rota3, *ventana_rota4, *ventana, *puerta, *felix_d1, *felix_i1, *felix_r11, *felix_r21, *felix_r31, *felix_r41, *felix_r51, *felix_d2, *felix_i2, *felix_r12, *felix_r22,
		*felix_r32, *felix_r42, *felix_r52, *felix1, *felix2, *ralph_1, *ralph_2, *ralph_3, *ralph_4, *ralph_5, *ralph_6, *ralph, *roca1, *roca2, *roca, *pajaro_1, *pajaro_2, *pajaro, *texto, *puntos, *vidas, *torta;

struct ventana ventanas_tramo1[3][5];
/* ALMACENO FILA y COLUMNA -- pienso el edificio como una matriz */

struct posicion felix1_posicion = { 0, 0 };
struct posicion felix2_posicion = { 0, 0 };
struct posicion ralph_posicion = { 3, 2 };
struct posicion torta_posicion;
struct desplazamiento pajaro_desplazamiento = { -1, -1 };
struct desplazamiento rocas_desplazamiento[20];

unsigned short int rahlp_x = PARED_X + 200;
unsigned short int rahlp_y = PARED_Y;
short int roca_siguiente = 0;
short int cant_rocas = 0;
short int ralph_destino = 0;
unsigned short int tramo = 1;
unsigned short int nivel = 1;
pthread_t thEscuchaTorneo, thEscuchaTeclas, thEscuchaServidor, thEnvioServidor;
int resultThEscuchaTorneo, resultThEscuchaTeclas, resultThEscuchaServidor, resultThEnvioServidor;
char salir = 'N';
char ralph_moverse = 'N';
char pajaro_moverse = 'N';
char ralph_sentido;
char felix1_reparar = 'N';
char felix2_reparar = 'N';
char ventanas_cargadas = 'N';
char torta_aparece = 'N';
char felix_cartel_puntos[10] = { 0 };
char felix_cartel_vidas[10] = { 0 };
string felix1_nombre = "";
string felix2_nombre = "";
short int felix1_puntos = 0;
short int felix2_puntos = 0;
short int felix1_vidas = 0;
short int felix2_vidas = 0;
short int ventanas_reparadas = 10;
bool felix2_inicial = true;
bool felix1_inicial = true;
SDL_Event evento;
SDL_keysym keysym;

SDL_Rect pantalla_juego, pantalla_texto, pantalla_puntos, pantalla_vidas, posBackground;

SDL_Color color_texto;
TTF_Font *fuente;

//unsigned int idPartida = 0;
unsigned int puertoServidorPartida = 0;
//configuracion archivo configFile
unsigned short int puertoTorneo;
string ip = "";
int key_arriba = -1;
int key_derecha = -1;
int key_abajo = -1;
int key_izquierda = -1;
int key_accion = -1;
int key_salir = -1;

const char* ranking;
bool torneoFinalizado = false;
bool showWindowRanking = false;
bool murioServidorTorneo = false;
string nombreOponente;
string mi_id;
bool solicitudDeNuevaParitda = false;
bool start = false;
bool recibioPosicionInicial = false;
int partidasJugadas = 0;
//bool recibioIdPartida = false;

CommunicationSocket * socketTorneo;
CommunicationSocket * socketPartida;

int main(int argc, char *argv[]) {
	atexit(liberarRecursos);
	signal(SIGINT, handler);

	int readData = 0;

	if (!cargarImagenes()) {
		cout << "Error al cargar las imagenes" << endl;
		exit(1);
	}

	//Inicio modo video
	SDL_Init(SDL_INIT_VIDEO);
	//Inicio modo texto grafico
	TTF_Init();
	//Defino las propiedades de la pantalla del juego
	superficie = SDL_SetVideoMode(ANCHO_PANTALLA, ALTO_PANTALLA, BPP, SDL_HWSURFACE);
	//Seteo el titulo de la pantalla
	SDL_WM_SetCaption("Rahlp Tournament", NULL);
	//Cargo la fuente
	fuente = TTF_OpenFont("./Fuentes/DejaVuSans.ttf", 24);
	//Color del texto
	color_texto.r = color_texto.g = color_texto.b = 245;

	//Pantalla de inicio.
	posBackground.x = 0;
	posBackground.y = 0;
	mostrarPantalla("start");

	//Empieza a cargar el nombre
	IngresaNombre();

	//Dimensiones rectangulo donde irá el texto
	pantalla_texto.x = 10;
	pantalla_texto.y = 10;

	//Obtener configuracion inicial (ip, puerto, teclas)
	getConfiguration(&puertoTorneo, &ip, &key_arriba, &key_derecha, &key_abajo, &key_izquierda, &key_accion, &key_salir);
	if (puertoTorneo == 0 || ip.compare("") == 0) {
		cout << "Error al obtener configuracion." << endl;
		exit(1);
	}

	//Muestro pantalla de "esperando al servidor por nueva partida"
	mostrarPantalla("waitmatch");

	//Conexion con el servidor de torneo.
	int reintentar = 3;
	do {
		try {
			socketTorneo = new CommunicationSocket(puertoTorneo, (char*) ip.c_str());
			reintentar = 0;
		} catch (...) {
			cout << "No se encuentra un servidor de torneo disponible en el puerto: " << puertoTorneo << " ip: " << ip.c_str() << endl;
			reintentar--;
			if (reintentar == 0) {
				cout << "No se ha podido establecer una conexion con un servidor de torneo." << endl;
				mostrarPantalla("servernotfound");
				sleep(10);
				exit(1);
			}
			sleep(5);
		}
	} while (reintentar != 0);

	//Le mando mi nombre
	socketTorneo->SendNoBloq(felix1_nombre.c_str(), sizeof(felix1_nombre));

	//Recibo el ID que me asigna el Torneo
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	if (!torneoFinalizo()) {
		readData = socketTorneo->ReceiveBloq(buffer, sizeof(buffer));
	} else {
		mostrarPantalla("servernotfound");
		sleep(10);
		exit(1);
	}
	if (readData <= 0) {
		cout << "Se ha cerrado la conexion con el servidor de torneo" << endl;
		mostrarPantalla("servertorneodown");
		sleep(10);
		exit(1);
	}
	string aux_buffer(buffer);
	mi_id = aux_buffer.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO);

	//Recibo el puerto del servidor de partida
	if (!torneoFinalizo()) {
		readData = socketTorneo->ReceiveBloq(buffer, sizeof(buffer));
	} else {
		mostrarPantalla("servernotfound");
		sleep(10);
		exit(1);
	}
	if (readData <= 0) {
		cout << "Se ha cerrado la conexion con el servidor de torneo" << endl;
		mostrarPantalla("servertorneodown");
		sleep(10);
		exit(1);
	}
	string aux_puerto(buffer);
	puertoServidorPartida = atoi(aux_puerto.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str());

	//Thread para escuchar al servidor de Torneo.
	resultThEscuchaTorneo = pthread_create(&thEscuchaTorneo, NULL, EscuchaTorneo, &socketTorneo->ID);
	if (resultThEscuchaTorneo) {
		cout << "Error no se pudo crear el thread de Escuchar Servidor de Torneo" << endl;
		exit(1);
	}
	//Thread que va a estar a la escucha de las teclas que se presionan.
	resultThEscuchaTeclas = pthread_create(&thEscuchaTeclas, NULL, EscuchaTeclas, NULL);
	if (resultThEscuchaTeclas) {
		cout << "Error no se pudo crear el thread de Escucha Teclas" << endl;
		exit(1);
	}

	unsigned short int fila, columna;
	unsigned short int ventana_x, ventana_y;

	inicializarNuevaPartida();

	while (salir == 'N' && !murioServidorDelTorneo()) {
		if (nuevaPartidaSolicitada() && torneoFinalizo()) {
			break;
		}
		if (nuevaPartidaSolicitada() && !torneoFinalizo() && !murioServidorDelTorneo()) {
			//Solicite nueva partida;
			mostrarPantalla("waitmatch");
			sleep(3);
			if (!inicializarNuevaPartida()) {
				//si no pudo inicializar una nueva partida salgo
				break;
			}
			pthread_mutex_lock(&mutex_solicitudDeNuevaParitda);
			solicitudDeNuevaParitda = false;
			pthread_mutex_unlock(&mutex_solicitudDeNuevaParitda);
		}

		pantalla_juego.x = 0;
		pantalla_juego.y = 0;
		pantalla_juego.w = ANCHO_PANTALLA;
		pantalla_juego.h = ALTO_PANTALLA;

		SDL_FillRect(superficie, &pantalla_juego, SDL_MapRGB(superficie->format, 0, 0, 0));
		//Dibujo la pared.
		switch (tramo) {
		case 1:
			pared = pared_tramo1n1;
			break;
		case 2:
			pared = pared_tramo2n1;
			break;
		default:
			pared = pared_tramo3n1;
			break;
		}
		Dibujar(PARED_X, PARED_Y, pared, superficie);

		//Dibujo los puntos de los jugadores
		pantalla_puntos.x = 10;
		pantalla_puntos.y = 30;
		sprintf(felix_cartel_puntos, "Puntos %d", felix1_puntos);
		puntos = TTF_RenderText_Solid(fuente, felix_cartel_puntos, color_texto);
		SDL_BlitSurface(puntos, NULL, superficie, &pantalla_puntos);

		pantalla_puntos.x = 500;
		sprintf(felix_cartel_puntos, "Puntos %d", felix2_puntos);
		puntos = TTF_RenderText_Solid(fuente, felix_cartel_puntos, color_texto);
		SDL_BlitSurface(puntos, NULL, superficie, &pantalla_puntos);

		//Dibujo la cantidad de vidas
		pantalla_vidas.x = 10;
		pantalla_vidas.y = 50;
		sprintf(felix_cartel_vidas, "Vidas %d", felix1_vidas);
		vidas = TTF_RenderText_Solid(fuente, felix_cartel_vidas, color_texto);
		SDL_BlitSurface(vidas, NULL, superficie, &pantalla_vidas);

		pantalla_vidas.x = 500;
		sprintf(felix_cartel_vidas, "Vidas %d", felix2_vidas);
		vidas = TTF_RenderText_Solid(fuente, felix_cartel_vidas, color_texto);
		SDL_BlitSurface(vidas, NULL, superficie, &pantalla_vidas);

		//Dibujo el nombre de los Jugadores Felix
		pantalla_texto.x = 10;
		pantalla_texto.y = 10;
		texto = TTF_RenderText_Solid(fuente, felix1_nombre.c_str(), color_texto);
		SDL_BlitSurface(texto, NULL, superficie, &pantalla_texto);

		pantalla_texto.x = 500;
		texto = TTF_RenderText_Solid(fuente, felix2_nombre.c_str(), color_texto);
		SDL_BlitSurface(texto, NULL, superficie, &pantalla_texto);

		// cargar las ventanas del tramo 1 -- fila 0 es la de mas abajo.
		if (ventanas_cargadas == 'N') {
			ventana_x = PARED_X + 45, ventana_y = PARED_Y + 25;
			for (fila = 3; fila > 0; fila--) {
				for (columna = 0; columna < 5; columna++) {
					ventanas_tramo1[fila - 1][columna].tipo_ventana = 4; //rand() % 5;
					CargarVentanasDelTramo(&ventanas_tramo1[fila - 1][columna], ventana_x, ventana_y, columna + (fila - 1) * 5, 'N', fila - 1, columna);
					ventana_x += 80;
				}
				ventana_x = PARED_X + 45;
				ventana_y += 113;
			}
			ventanas_cargadas = 'S';
		}

		DibujarVentanas(ventanas_tramo1, 3, superficie);

		//Dibujo la puerta
		if (tramo == 1)
			Dibujar(ventanas_tramo1[0][1].x + 65, 383, puerta, superficie);

		//Dibujo la torta
		if (torta_aparece == 'S') {
			Dibujar(ventanas_tramo1[torta_posicion.fila][torta_posicion.columna].x, ventanas_tramo1[torta_posicion.fila][torta_posicion.columna].y, torta, superficie);
		} else {
			if (!cola_torta.empty()) {
				torta_posicion.fila = atoi(cola_torta.front().substr(5, 1).c_str());
				torta_posicion.columna = atoi(cola_torta.front().substr(6, 1).c_str());
				cola_torta.pop();
				torta_aparece = 'S';
			}
		}
		//Dibujo a Ralph
		if (ralph == ralph_1)
			ralph = ralph_2;
		else if (ralph == ralph_2)
			ralph = ralph_3;
		else if (ralph == ralph_3)
			ralph = ralph_4;
		else if (ralph == ralph_4)
			ralph = ralph_5;
		else if (ralph == ralph_5)
			ralph = ralph_6;
		else
			ralph = ralph_1;

		if (ralph_moverse == 'S') {
			if (ralph_posicion.columna != ralph_destino) {
				if (ralph_sentido == 'D') {
					ralph_posicion.columna += 1;
				} else {
					ralph_posicion.columna -= 1;
				}
			} else {
				ralph_moverse = 'N';
				if (cant_rocas < 20) {
					rocas_desplazamiento[roca_siguiente].x = ventanas_tramo1[2][ralph_posicion.columna].x + 10;
					((roca_siguiente + 1) == 20) ? roca_siguiente = 0 : roca_siguiente++;
					cant_rocas++;
				}
			}
		} else {
			if (!cola_ralph.empty()) {
				// Me llegan numeradas del 1 al 5. Le resto 1 porque yo las tengo del 0 al 4.
				ralph_destino = atoi(cola_ralph.front().substr(6, 1).c_str());
				cola_ralph.pop();

				if (ralph_destino > ralph_posicion.columna)
					ralph_sentido = 'D';
				else
					ralph_sentido = 'I';
				ralph_moverse = 'S';
			}
		}
		Dibujar(ventanas_tramo1[2][ralph_posicion.columna].x, PARED_Y - 100, ralph, superficie);

		//Dibujo la pajaro
		if (pajaro == pajaro_1)
			pajaro = pajaro_2;
		else
			pajaro = pajaro_1;
		if (pajaro_moverse == 'S') {
			pajaro_desplazamiento.x += 10;
			Dibujar(pajaro_desplazamiento.x, pajaro_desplazamiento.y, pajaro, superficie);
			if (pajaro_desplazamiento.x > 630) {
				pajaro_moverse = 'N';
				pajaro_desplazamiento.x = -1;
				pajaro_desplazamiento.y = -1;

			}
		} else {
			if (!cola_pajaro.empty()) {
				short int pajaro_fila = atoi(cola_pajaro.front().substr(6, 1).c_str());
				cola_pajaro.pop();
				//cout << "Fila comienzo: " << pajaro_fila << endl;
				pajaro_desplazamiento.x = 10;
				pajaro_desplazamiento.y = ventanas_tramo1[pajaro_fila][0].y;
				pajaro_moverse = 'S';
			}
		}
		//Dibujo a Felix
		if (felix1_vidas > 0) {
			if (felix1_reparar == 'N') {
				if (felix1 == NULL)
					felix1 = felix_d1;
				if (!cola_felix1.empty()) {
					//cout << "Entro a la cola de felix movimiento" << endl;
					felix1_inicial = false;
					string msj = cola_felix1.front();
					cola_felix1.pop();
					felix1_posicion.columna = atoi(msj.substr(5, 1).c_str());
					//cout << "Nueva posicion columna de felix1" << felix1_posicion.columna << endl;
					felix1_posicion.fila = atoi(msj.substr(6, 1).c_str());
					//cout << "Nueva posicion fila de felix1: " << felix1_posicion.fila << endl;
				}
				Dibujar(ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].x, ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].y, felix1, superficie);
			} else {
				//cout << "inicia repara ventana" << endl;
				if (felix1 == felix_d1 || felix1 == felix_i1)
					felix1 = felix_r11;
				else if (felix1 == felix_r11)
					felix1 = felix_r21;
				else if (felix1 == felix_r21)
					felix1 = felix_r31;
				else if (felix1 == felix_r31)
					felix1 = felix_r41;
				else if (felix1 == felix_r41)
					felix1 = felix_r51;
				else if (felix1 == felix_r51) {
					felix1 = felix_d1;

					string message(CD_VENTANA_ARREGLADA);
					message.append(fillMessage("0"));
					cola_grafico.push(message);
					ventana_reparada(&felix1_posicion);

					felix1_reparar = 'N';
				}
				Dibujar(ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].x, ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].y, felix1, superficie);
			}
		}

		if (felix2 == NULL)
			felix2 = felix_d2;

		//Mueveo a felix2, salvo que este en la posicion inicial
		if (felix2_vidas > 0) {
			if (felix2_reparar == 'N') {
				if (!cola_felix2.empty()) {
					string msj = cola_felix2.front();
					cola_felix2.pop();
					felix2_posicion.columna = atoi(msj.substr(5, 1).c_str());
					felix2_posicion.fila = atoi(msj.substr(6, 1).c_str());
				}
				Dibujar(ventanas_tramo1[felix2_posicion.fila][felix2_posicion.columna].x, ventanas_tramo1[felix2_posicion.fila][felix2_posicion.columna].y, felix2, superficie);

			} else {
				if (felix2 == felix_d2 || felix2 == felix_i2)
					felix2 = felix_r12;
				else if (felix2 == felix_r12)
					felix2 = felix_r22;
				else if (felix2 == felix_r22)
					felix2 = felix_r32;
				else if (felix2 == felix_r32)
					felix2 = felix_r42;
				else if (felix2 == felix_r42)
					felix2 = felix_r52;
				else if (felix2 == felix_r52) {
					felix2 = felix_d2;

					//string message(CD_VENTANA_ARREGLADA);
					//message.append(fillMessage("0"));
					//cola_grafico.push(message);
					//ventana_reparada(&felix2_posicion);
					felix2_reparar = 'N';
				}

				Dibujar(ventanas_tramo1[felix2_posicion.fila][felix2_posicion.columna].x, ventanas_tramo1[felix2_posicion.fila][felix2_posicion.columna].y, felix2, superficie);

			}
		}
		//Dibujo las rocas
		roca = roca1;
		for (int i = 0; i < cant_rocas; i++) {
			if (rocas_desplazamiento[i].x != 0) {
				if (rocas_desplazamiento[i].y < 405) {
					Dibujar(rocas_desplazamiento[i].x, rocas_desplazamiento[i].y, roca, superficie);
					rocas_desplazamiento[i].y += 5;
				} else {
					rocas_desplazamiento[i].x = 0;
					rocas_desplazamiento[i].y = 0;
					cant_rocas--;
				}
			}
		}
		if (hayChoque()) {
			//felix1_posicion.fila = 0;
			//felix1_posicion.columna = 0;

			string message(CD_PERDIDA_VIDA);
			message.append(fillMessage("0"));
			cola_grafico.push(message);

			/*if (felix1_vidas > 0) {
			 felix1_vidas--;
			 //felix1_inicial = true;
			 }*/
		}

		if (ventanas_reparadas == 11) {
			ventanas_reparadas = 10;
			ventanas_cargadas = 'N';
			if (tramo == 3) {
				PantallaIntermedia('2');
				tramo = 1;
				nivel++;
			} else {
				PantallaIntermedia('1');
				tramo++;
			}
			salir = 'N';
		} else {
			SDL_Flip(superficie);
			SDL_Delay(300);
		}
	}

	if (murioServidorDelTorneo()) {
		mostrarPantalla("servertorneodown");
		sleep(10);
		exit(1);
	}

	//esperar mientras las demas partidas no han finalizado. (mostrar msj "GameOver. waiting for rankings.. ")
	mostrarPantalla("gameover");
	sleep(3);

	while (!showWindowRanking) {
		sleep(1);
	}
	mostrarRanking(ranking);

	cout << "Ingrese un tecla para terminar: ";
	getchar();

	exit(1);
}

bool hayChoque() {
	if (felix1_posicion.fila != 99 && felix1_posicion.columna != 99 && ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].x <= (pajaro_desplazamiento.x + 20) && (ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].x + 20) >= pajaro_desplazamiento.x
			&& ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].y == pajaro_desplazamiento.y)
		return true;
	for (int i = 0; i < cant_rocas; i++)
		if (felix1_posicion.fila != 99 && felix1_posicion.columna != 99 && ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].y <= rocas_desplazamiento[i].y && (ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].y + 50) >= rocas_desplazamiento[i].y
				&& ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].x == (rocas_desplazamiento[i].x - 10))
			return true;
	return false;
}

void CargarVentanasDelTramo(struct ventana *ventana, unsigned short int x, unsigned short int y, unsigned short int nro, char N_S, unsigned short int f, unsigned short int c) {
	ventana->x = x;
	ventana->y = y;
	ventana->fila = f;
	ventana->columna = c;
	ventana->numero = nro;
	ventana->ocupado = N_S;
	ventana->sana = 0;
}

void ConfigRect(SDL_Rect *rect, int x, int y, int w, int h) {
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
}

void Dibujar(int x, int y, SDL_Surface *imagen, SDL_Surface *superficie) {

	SDL_Rect posicion, tamano;
	Uint32 colorkey;

	ConfigRect(&posicion, x, y, imagen->w, imagen->h);
	ConfigRect(&tamano, 0, 0, imagen->w, imagen->h);
	colorkey = SDL_MapRGB(imagen->format, 0, 0, 0);
	SDL_SetColorKey(imagen, SDL_SRCCOLORKEY, colorkey);
	SDL_BlitSurface(imagen, &tamano, superficie, &posicion);
}

void DibujarVentanas(struct ventana ventanas[][5], unsigned short int cant_filas, SDL_Surface *superficie) {

	short int fila = 0;
	short int columna = 0;

	for (fila = cant_filas; fila > 0; fila--) {
		for (columna = 0; columna < 5; columna++) {
			if ((fila - 1) != 0 || columna != 2) {
				switch (ventanas[fila - 1][columna].tipo_ventana) {
				case 0:
					ventana = ventana_sana;
					ventanas[fila - 1][columna].sana = 3;
					break;
				case 1:
					ventana = ventana_rota1;
					break;
				case 2:
					ventana = ventana_rota2;
					break;
				case 3:
					ventana = ventana_rota3;
					break;
				case 4:
					ventana = ventana_rota4;
					break;
				}
				Dibujar(ventanas[fila - 1][columna].x, ventanas[fila - 1][columna].y, ventana, superficie);
			}
		}
	}
}

/**
 * THREAD -> Escucha mensajes del servidor de Partida
 */
void* EscuchaServidor(void *arg) {
	//cout << "inicia th EscuchaServidor" << endl;
	int fd = *(int *) arg;
	int readData = 0;
	int codigo;
	CommunicationSocket cSocket(fd);
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	bzero(buffer, sizeof(buffer));
	while (true) {
		//cout << "Espero msj del servidor de partida ... " << endl;
		readData = cSocket.ReceiveBloq(buffer, sizeof(buffer));
		if (strlen(buffer) > 0) {
			string aux_buffer(buffer);
			codigo = atoi(aux_buffer.substr(0, LONGITUD_CODIGO).c_str());

			switch (codigo) {
			case CD_MOVIMIENTO_RALPH_I:
				cola_ralph.push(aux_buffer);
				break;
			case CD_PALOMA_I:
				cola_pajaro.push(aux_buffer);
				break;
			case CD_TORTA_I:
				//cola_torta.push(aux_buffer);
				break;
			case CD_PERSIANA_I:
				break;
			case CD_MOVIMIENTO_FELIX_I:
				if (buffer[4] == '1') {
					cola_felix1.push(aux_buffer);
				} else if (buffer[4] == '2') {
					cola_felix2.push(aux_buffer);
				}
				break;
			case CD_OPONENTE_DESCONECTADO_I:
				felix2_vidas = 0;
				felix2_puntos = 0;
				break;
			case CD_PERDIDA_VIDA_I:
				if (buffer[4] == '1') {
					cola_felix1.push(aux_buffer);
					if (felix1_vidas > 0) {
						felix1_vidas--;
						felix1_reparar = 'N';
					}
				} else if (buffer[4] == '2') {
					cola_felix2.push(aux_buffer);
					if (felix2_vidas > 0) {
						felix2_vidas--;
					}
				}
				break;
			case CD_PERDIO_I:
				if (buffer[6] == '1') {
					felix1_vidas = 0;
				} else if (buffer[6] == '2') {
					felix2_vidas = 0;
				}
				break;
			case CD_VENTANA_ARREGLANDO_I:
				//TODO grafico la otra ventana arreglada.
				felix2_reparar = 'S';
				//TODO grafico al otro jugador arreglando la ventana.
				break;
			case CD_VENTANA_ARREGLADA_I:
				if (buffer[6] == '1') {
					felix1_puntos += 10;
				} else if (buffer[6] == '2') {
					felix2_puntos += 10;
					felix2 = felix_d2;
					//cout << "Posicion felix2, fila: " << felix2_posicion.fila << " columna: " << felix2_posicion.columna << endl;
					ventanas_tramo1[felix2_posicion.fila][felix2_posicion.columna].tipo_ventana = 0;
					ventanas_tramo1[felix2_posicion.fila][felix2_posicion.columna].sana = 1;
					felix2_reparar = 'N';
				}
				break;
			case CD_ID_JUGADOR_I:
				break;
			case CD_FIN_PARTIDA_I:
				pthread_mutex_lock(&mutex_partidaFinalizada);
				solicitudDeNuevaParitda = true;
				felix2_nombre = "";
				pthread_mutex_unlock(&mutex_partidaFinalizada);
				break;
			case CD_EMPEZAR_PARTIDA_I:
				pthread_mutex_lock(&mutex_start);
				start = true;
				pthread_mutex_unlock(&mutex_start);
				break;
			case CD_CANTIDAD_VIDAS_I:
				pthread_mutex_lock(&mutex_cantVidas);
				felix1_vidas = atoi(aux_buffer.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str());
				felix2_vidas = felix1_vidas;
				pthread_mutex_unlock(&mutex_cantVidas);
				break;
			case CD_POSICION_INICIAL_I:
				felix1_posicion.columna = atoi(aux_buffer.substr(5, 1).c_str());
				felix1_posicion.fila = atoi(aux_buffer.substr(6, 1).c_str());
				//cout << "Posicion inicial de felix: columna " << felix1_posicion.columna << " fila " << felix1_posicion.fila << endl;
				if (felix1_posicion.columna == 0) {
					felix2_posicion.columna = EDIFICIO_COLUMNAS - 1;
				} else {
					felix2_posicion.columna = 0;
				}
				felix2_posicion.fila = 0;
				pthread_mutex_lock(&mutex_recibioPosicionInicial);
				recibioPosicionInicial = true;
				pthread_mutex_unlock(&mutex_recibioPosicionInicial);
				break;
			case CD_ACK_I:
				string message(CD_ACK);
				message.append(fillMessage("0"));
				//TODO Agregar mutex.
				cola_grafico.push(message);
				break;
			}
		} else if (readData == 0) {
			//reseteo el nombre del oponente para que espero que se lo envien
			pthread_mutex_lock(&mutex_nombreOponente);
			nombreOponente = "";
			pthread_mutex_unlock(&mutex_nombreOponente);

			//cout << "Termino la partida" << endl;

			delete (socketPartida);
			socketPartida = NULL;
		}

		usleep(2000);
	}
	//cout << "sale el thread de escucharServidor" << endl;
	pthread_exit(NULL);
}

void * EnvioServidor(void * arg) {
	int fd = *(int *) arg;
	CommunicationSocket cSocket(fd);
	//cout << "inicia th EnviaServidor" << endl;
	while (true) {
		if (!cola_grafico.empty()) {
			string mensaje = cola_grafico.front();
			cola_grafico.pop();
			//cout << "Mensaje a enviar al servPartida: " << mensaje.c_str() << endl;
			cSocket.SendBloq(mensaje.c_str(), mensaje.length());
		}
		usleep(1000);
	}

	//cout << "sale el thread de EnvioServidor" << endl;
	pthread_exit(NULL);
}

void* EscuchaTorneo(void *arg) {
	int fd = *(int *) arg;
	int readData = 0;
	const char* mensajeNombre;
	char auxNombreOponente[6];
	CommunicationSocket cSocket(fd);
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	bzero(buffer, sizeof(buffer));
	int indice;
	int auxIndice;

	while (true) {
		readData = cSocket.ReceiveBloq(buffer, sizeof(buffer));

		if (strlen(buffer) > 0) {
			string aux_buffer(buffer);

			int codigo = atoi((aux_buffer.substr(0, LONGITUD_CODIGO).c_str()));
			switch (codigo) {
			//case CD_ID_JUGADOR_I:
			//	break;
			case CD_PUERTO_PARTIDA_I:
				pthread_mutex_lock(&mutex_msjPuertoRecibido);
				puertoServidorPartida = atoi(aux_buffer.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str());
				msjPuertoRecibido = true;
				pthread_mutex_unlock(&mutex_msjPuertoRecibido);
				break;
			case CD_RANKING_I:
				ranking = aux_buffer.substr(LONGITUD_CODIGO + LONGITUD_CONTENIDO - 2, 2).c_str();
				salir = 'S';
				showWindowRanking = true;
				//salgo del thread porque este el ultimo mensaje que me interesa
				pthread_exit(NULL);
				break;
			/*case CD_ID_PARTIDA_I:
				idPartida = atoi(aux_buffer.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str());
				recibioIdPartida = true;
				break;*/
			case CD_NOMBRE_I:
				//recibo y limpio el nombre de ceros
				mensajeNombre = aux_buffer.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str();

				auxIndice = 0;
				for (indice = 0; indice < LONGITUD_CONTENIDO; indice++) {
					if (mensajeNombre[indice] != '0') {
						auxNombreOponente[auxIndice] = mensajeNombre[indice];
						auxIndice++;
					}
				}
				auxNombreOponente[auxIndice] = '\0';
				pthread_mutex_lock(&mutex_nombreOponente);
				nombreOponente = auxNombreOponente;
				pthread_mutex_unlock(&mutex_nombreOponente);
				break;
			case CD_FIN_TORNEO_I:
				//que no busque mas establecer partidas
				torneoFinalizado = true;
				break;
			case CD_ACK_I:
				string message(CD_ACK);
				string content = "1";
				message.append(fillMessage(content));
				cSocket.SendNoBloq(message.c_str(), LONGITUD_CODIGO);
				break;
			}
		} else if (readData == 0) {
			pthread_mutex_lock(&mutex_murioServidorTorneo);
			murioServidorTorneo = true;
			pthread_mutex_unlock(&mutex_murioServidorTorneo);

			//salgo del thread sino me llega todo el tiempo el mensaje vacio
			pthread_exit(NULL);
		}
		usleep(1000);
	}

	pthread_exit(NULL);
}

void* EscuchaTeclas(void *arg) {
	SDL_Event evento; //Con esta variable reconozco si se apreto una tecla o se apreto el mouse.
	SDL_keysym keysym; //Con esta variable reconzco el codigo de la tecla que se apreto.
	short int f1_fila;
	short int f1_colu;

	//Lupeo escuchando el teclado.
	while (!murioServidorDelTorneo() && SDL_WaitEvent(&evento) != 0) {

		f1_fila = felix1_posicion.fila;
		f1_colu = felix1_posicion.columna;

		switch (evento.type) {

		case SDL_KEYDOWN:
			if ((evento.key.keysym.sym == SDLK_DOWN || evento.key.keysym.sym == key_abajo) && felix1_reparar != 'S') {
				if ((felix1_posicion.fila - 1) >= 0 && felix1_reparar != 'S' && felix1_vidas > 0) {
					char aux[3];
					sprintf(aux, "%d", -1);
					string message(CD_MOVIMIENTO_FELIX);
					message.append(fillMessage(aux));
					cola_grafico.push(message);
				}
			} else if ((evento.key.keysym.sym == SDLK_UP || evento.key.keysym.sym == key_arriba) && felix1_reparar != 'S') {
				if ((felix1_posicion.fila + 1) < 3 && felix1_reparar != 'S' && felix1_vidas > 0) {
					char aux[3];
					sprintf(aux, "%d", 1);
					string message(CD_MOVIMIENTO_FELIX);
					message.append(fillMessage(aux));
					cola_grafico.push(message);
				}
			} else if ((evento.key.keysym.sym == SDLK_RIGHT || evento.key.keysym.sym == key_derecha) && felix1_reparar != 'S') {
				felix1 = felix_d1;
				if ((felix1_posicion.columna + 1) < 5 && felix1_reparar != 'S' && felix1_vidas > 0) {
					string message(CD_MOVIMIENTO_FELIX);
					char aux[5];
					sprintf(aux, "%d", 100);
					message.append(fillMessage(aux));
					cola_grafico.push(message);
				}
			} else if ((evento.key.keysym.sym == SDLK_LEFT || evento.key.keysym.sym == key_izquierda) && felix1_reparar != 'S') {
				felix1 = felix_i1;
				if ((felix1_posicion.columna - 1) >= 0 && felix1_reparar != 'S' && felix1_vidas > 0) {
					char aux[5];
					string message(CD_MOVIMIENTO_FELIX);
					sprintf(aux, "%d", -100);
					message.append(fillMessage(aux));
					cola_grafico.push(message);
				}
			} else if ((evento.key.keysym.sym == SDLK_SPACE || evento.key.keysym.sym == key_accion) && felix1_vidas > 0 && felix1_reparar != 'S') {
				string message(CD_VENTANA_ARREGLANDO);
				message.append(fillMessage("0"));
				cola_grafico.push(message);
				felix1_reparar = 'S';
			} else if (evento.key.keysym.sym == key_salir) {
				exit(1);
			}
			break;
		case SDL_QUIT:
			exit(1);
		}
		usleep(1000);
	}
	pthread_exit(NULL);
}

char ventana_reparada(struct posicion *felix_posicion) {
	//cout << "Ventana Sana valor " << ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].sana << endl;
	if (ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].tipo_ventana != 0) {
		if (ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].sana < 1) {
			ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].sana++;
			if (ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].sana == 1) {
				ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].tipo_ventana = 0;
				//cout << "Ventana reparada " << felix_posicion->fila << ":" << felix_posicion->columna << endl;
				return 'S';
			} else
				return 'N';
		} else
			return 'N';
	}
	return 'N';
}

void handler(int senial) {
	switch (senial) {
	case SIGINT:
		break;
	}
}

void PantallaIntermedia(char cod) {
	while (salir == 'N') {
		switch (cod) {
		case '0':
			salir = IngresaNombre();
			break;
		case '1':
			salir = CambiaTramo();
			break;
		case '2':
			salir = CambiaNivel();
			break;
		}
		SDL_Delay(30);
	}
}

char CambiaTramo() {
	pantalla_juego.x = 0;
	pantalla_juego.y = 0;
	pantalla_juego.w = ANCHO_PANTALLA;
	pantalla_juego.h = ALTO_PANTALLA;
	SDL_FillRect(superficie, &pantalla_juego, SDL_MapRGB(superficie->format, 0, 0, 0));

	pantalla_texto.x = 10;
	pantalla_texto.y = 10;
	texto = TTF_RenderText_Solid(fuente, "Cambio de tramo", color_texto);
	SDL_BlitSurface(texto, NULL, superficie, &pantalla_texto);
	SDL_Flip(superficie);

	SDL_WaitEvent(&evento);

	return 'S';
}

char CambiaNivel() {
	pantalla_juego.x = 0;
	pantalla_juego.y = 0;
	pantalla_juego.w = ANCHO_PANTALLA;
	pantalla_juego.h = ALTO_PANTALLA;
	SDL_FillRect(superficie, &pantalla_juego, SDL_MapRGB(superficie->format, 0, 0, 0));

	pantalla_texto.x = 10;
	pantalla_texto.y = 10;
	texto = TTF_RenderText_Solid(fuente, "Cambio de nivel", color_texto);
	SDL_BlitSurface(texto, NULL, superficie, &pantalla_texto);
	SDL_Flip(superficie);

	SDL_WaitEvent(&evento);

	return 'S';
}

char IngresaNombre() {
	int teclaIngresada;
	SDL_Rect posTextoNombreIngresado;
	posTextoNombreIngresado.x = 385;
	posTextoNombreIngresado.y = 57;

	while (true) {
		SDL_WaitEvent(&evento);
		if (evento.type == SDL_KEYDOWN) {
			teclaIngresada = evento.key.keysym.sym;
			if (teclaIngresada != SDLK_RETURN && teclaIngresada != SDLK_KP_ENTER) {
				backgroundImg = SDL_LoadBMP("Sprites/Mensajes/start.bmp");
				if (backgroundImg == NULL) {
					printf("Error en SDL_LoadBMP= %s\n", SDL_GetError());
					exit(1);
				}

				if (teclaIngresada >= 97 && teclaIngresada <= 122 && felix1_nombre.length() < LONGITUD_CONTENIDO)
					felix1_nombre += teclaIngresada;
				else if (teclaIngresada == SDLK_BACKSPACE && felix1_nombre.length() > 0)
					felix1_nombre = felix1_nombre.substr(0, felix1_nombre.length() - 1);

				//Texto del texto
				texto = TTF_RenderText_Solid(fuente, felix1_nombre.c_str(), color_texto);
				SDL_BlitSurface(texto, NULL, backgroundImg, &posTextoNombreIngresado);

				SDL_BlitSurface(backgroundImg, NULL, superficie, &posBackground);
				SDL_Flip(superficie);

			} else if (felix1_nombre.length() > 0) {
				break;
			}
		}
	}
	return salir;
}

string fillMessage(string message) {
	string content;
	int cantCeros = LONGITUD_CONTENIDO - message.length();
	content.assign(cantCeros, '0');
	return content.append(message);
}

/**
 * Obtener la configuracion inicial del Cliente
 */
void getConfiguration(unsigned short int* port, string* ip, int* arriba, int* derecha, int* abajo, int* izquierda, int* accion, int* salir) {
	string content;
	string line;
	fstream configFile("configFile", fstream::in | fstream::out);
	while (getline(configFile, line)) {
		if (line.find("IP") == 0) {
			int pos = line.find(":");
			string auxip = line.substr(pos + 1, line.length());
			*ip = auxip.c_str();
		} else if (line.find("Puerto") == 0) {
			int pos = line.find(":");
			string sport = line.substr(pos + 1, line.length());
			*port = atoi(sport.c_str());
		} else if (line.find("Arriba") == 0) {
			int pos = line.find(":");
			*arriba = (int) line.substr(pos + 1, line.length()).at(0);
		} else if (line.find("Derecha") == 0) {
			int pos = line.find(":");
			*derecha = (int) line.substr(pos + 1, line.length()).at(0);
		} else if (line.find("Abajo") == 0) {
			int pos = line.find(":");
			*abajo = (int) line.substr(pos + 1, line.length()).at(0);
		} else if (line.find("Izquierda") == 0) {
			int pos = line.find(":");
			*izquierda = (int) line.substr(pos + 1, line.length()).at(0);
		} else if (line.find("Accion") == 0) {
			int pos = line.find(":");
			*accion = (int) line.substr(pos + 1, line.length()).at(0);
		} else if (line.find("Salir") == 0) {
			int pos = line.find(":");
			*salir = (int) line.substr(pos + 1, line.length()).at(0);
		}
	}
}

void mostrarPantalla(const char* nombrPantalla) {
	pthread_mutex_lock(&mutex_mostrar_pantalla);
	string dir = "Sprites/Mensajes/";
	dir += nombrPantalla;
	dir += ".bmp";
	backgroundImg = SDL_LoadBMP(dir.c_str());
	if (backgroundImg == NULL) {
		cout << "Error en SDL_LoadBMP = " << SDL_GetError() << endl;
		exit(1);
	}
	SDL_BlitSurface(backgroundImg, NULL, superficie, &posBackground);
	if (SDL_Flip(superficie) == -1) {
		cout << "Error: SDL_FLIP" << SDL_GetError() << endl;
		exit(1);
	}
	pthread_mutex_unlock(&mutex_mostrar_pantalla);
}

void mostrarRanking(const char* ranking) {
	pthread_mutex_lock(&mutex_mostrar_pantalla);
	SDL_Rect posTextoRanking;
	posTextoRanking.x = 315;
	posTextoRanking.y = 340;
	backgroundImg = SDL_LoadBMP("Sprites/Mensajes/ranking.bmp");
	if (backgroundImg == NULL) {
		printf("Error en SDL_LoadBMP= %s\n", SDL_GetError());
		exit(1);
	}

	fuente = TTF_OpenFont("./Fuentes/DejaVuSans.ttf", 75);
	texto = TTF_RenderText_Solid(fuente, ranking, color_texto);
	SDL_BlitSurface(texto, NULL, backgroundImg, &posTextoRanking);

	SDL_BlitSurface(backgroundImg, NULL, superficie, &posBackground);
	SDL_Flip(superficie);
	pthread_mutex_unlock(&mutex_mostrar_pantalla);
}

bool inicializarNuevaPartida() {
	vaciarColas();

	if (partidasJugadas >= 1) {
		pthread_cancel(thEscuchaServidor);
		pthread_cancel(thEnvioServidor);
	}
	partidasJugadas++;

	//Espero id de partida (del torneo)
	//cout << "Esperando ID Partida" << endl;
	/*if (esperarIdPartida()) {
		//Le llega el Id de Partida
		recibioIdPartida = false;
	} else {
		salir = 'S';
		return false;
	}*/

	//Espero que el servidor de Torneo me envie  el nombre de mi oponente
	//cout << "Esperando nombre de oponente" << endl;
	if (esperarNombreOponente()) {
		//recibio nombre del oponente
	} else {
		salir = 'S';
		return false;
	}

	//cout << "inicializo un nueva partida. Reconecto" << endl;
	//Me conecto al servidor de partida.
	try {
		socketPartida = new CommunicationSocket(puertoServidorPartida, (char*) ip.c_str());
	} catch (const char * err) {
		cout << "Error al querer conectar al puerto de partida" << endl;
		exit(1);
	}

	//Lanzo Threads de comunicacio con el Servidor de Partida
	resultThEscuchaServidor = pthread_create(&thEscuchaServidor, NULL, EscuchaServidor, &socketPartida->ID);
	if (resultThEscuchaServidor) {
		cout << "Error no se pudo crear el thread de Escucha Servidor" << endl;
		exit(1);
	}
	resultThEnvioServidor = pthread_create(&thEnvioServidor, NULL, EnvioServidor, &socketPartida->ID);
	if (resultThEnvioServidor) {
		cout << "Error no se pudo crear el thread de Envio Servidor" << endl;
		exit(1);
	}

	inicializarVariablesDeLaPartida();

	//Mando mi IdDePartida
	/*string messageIDPartida(CD_ID_PARTIDA);
	char aux[5];
	sprintf(aux, "%d", idPartida);
	//cout << "El nuevo idPartida es: " << aux << endl;
	messageIDPartida.append(fillMessage(aux));
	cola_grafico.push(messageIDPartida);*/

	//Espero mi posicion inicial;
	//cout << "Esperando mi posicion inicial" << endl;
	esperarPosicionInicial();

	//Mando mi ID de Jugador
	string messageIDJugador(CD_ID_JUGADOR);
	messageIDJugador.append(fillMessage(mi_id.c_str()));
	cola_grafico.push(messageIDJugador);

	//Espero que el Servidor de Partida me mande el mensaje de START
	//cout << "Esperar signal de START" << endl;
	esperarSTART();

	return true;
}

void esperarPuertoPartida() {
	bool msjPuerto = false;
	//cout << "comienza while de espera de puerto de partida" << endl;
	while (true) {
		pthread_mutex_lock(&mutex_msjPuertoRecibido);
		msjPuerto = msjPuertoRecibido;
		pthread_mutex_unlock(&mutex_msjPuertoRecibido);

		if (msjPuerto == true) {
			break;
		}
		usleep(1000);
	}
	msjPuertoRecibido = false;
	//cout << "recibio el puerto:" << puertoServidorPartida << endl;
}

bool esperarCantVidas() {
	while (!torneoFinalizo()) {
		if (felix1_vidas != 0) {
			return true;
		}
		usleep(10000);
	}
	return false;
}

/*bool esperarIdPartida() {
	//cout << "esperando id Partida" << endl;
	while (!torneoFinalizo() && !murioServidorDelTorneo()) {
		if (recibioIdPartida) {
			return true;
		}
		usleep(10000);
	}
	return false;
}*/

bool esperarNombreOponente() {
	//cout << "comienza while de espera de nombre de oponente" << endl;
	bool recibioNombreOponente = false;
	while (!recibioNombreOponente && !torneoFinalizo() && !murioServidorDelTorneo()) {
		pthread_mutex_lock(&mutex_nombreOponente);
		if (nombreOponente.length() > 0) {
			felix2_nombre = nombreOponente;
			recibioNombreOponente = true;
		}
		pthread_mutex_unlock(&mutex_nombreOponente);
		usleep(10000);
	}

	//cout << "esperarNombreOponente ->recibo el nombre de mi oponente:" << felix2_nombre << endl;
	if (recibioNombreOponente) {
		return true;
	} else {
		return false;
	}
}

void esperarPosicionInicial() {
	bool auxRecibioPosicionInicial = false;
	while (!auxRecibioPosicionInicial && !murioServidorDelTorneo()) {
		pthread_mutex_lock(&mutex_recibioPosicionInicial);
		auxRecibioPosicionInicial = recibioPosicionInicial;
		pthread_mutex_unlock(&mutex_recibioPosicionInicial);
		usleep(10000);
	}
	pthread_mutex_lock(&mutex_start);
	recibioPosicionInicial = false;
	pthread_mutex_unlock(&mutex_start);
}

void esperarSTART() {
	bool empezar = false;
	while (!empezar && !murioServidorDelTorneo()) {
		pthread_mutex_lock(&mutex_start);
		empezar = start;
		pthread_mutex_unlock(&mutex_start);
		usleep(5000);
	}

	pthread_mutex_lock(&mutex_start);
	start = false;
	pthread_mutex_unlock(&mutex_start);
}

void vaciarColas() {
	while (!cola_grafico.empty()) {
		cola_grafico.pop();
	}
	while (!cola_ralph.empty()) {
		cola_ralph.pop();
	}
	while (!cola_pajaro.empty()) {
		cola_pajaro.pop();
	}
	while (!cola_torta.empty()) {
		cola_torta.pop();
	}
	while (!cola_mensajes_enviar.empty()) {
		cola_mensajes_enviar.pop();
	}
	while (!cola_felix1.empty()) {
		cola_felix1.pop();
	}
	while (!cola_felix2.empty()) {
		cola_felix2.pop();
	}
}

bool cargarImagenes() {
	pared_tramo1n1 = SDL_LoadBMP(pared_tramo1n1_bmp);
	if (pared_tramo1n1 == NULL)
		return false;
	pared_tramo2n1 = SDL_LoadBMP(pared_tramo2n1_bmp);
	if (pared_tramo2n1 == NULL)
		return false;
	pared_tramo3n1 = SDL_LoadBMP(pared_tramo3n1_bmp);
	if (pared_tramo3n1 == NULL)
		return false;
	ventana_sana = SDL_LoadBMP(ventana_sana_bmp);
	if (ventana_sana == NULL)
		return false;
	ventana_rota1 = SDL_LoadBMP(ventana_rota1_bmp);
	if (ventana_rota1 == NULL)
		return false;
	ventana_rota2 = SDL_LoadBMP(ventana_rota2_bmp);
	if (ventana_rota2 == NULL)
		return false;
	ventana_rota3 = SDL_LoadBMP(ventana_rota3_bmp);
	if (ventana_rota3 == NULL)
		return false;
	ventana_rota4 = SDL_LoadBMP(ventana_rota4_bmp);
	if (ventana_rota4 == NULL)
		return false;
	puerta = SDL_LoadBMP(puerta_bmp);
	if (puerta == NULL)
		return false;
	felix_d1 = SDL_LoadBMP(felixd1_bmp);
	if (felix_d1 == NULL)
		return false;
	felix_i1 = SDL_LoadBMP(felixi1_bmp);
	if (felix_i1 == NULL)
		return false;
	felix_r11 = SDL_LoadBMP(felixr11_bmp);
	if (felix_r11 == NULL)
		return false;
	felix_r21 = SDL_LoadBMP(felixr21_bmp);
	if (felix_r21 == NULL)
		return false;
	if (felix_r21 == NULL)
		return false;
	felix_r31 = SDL_LoadBMP(felixr31_bmp);
	if (felix_r31 == NULL)
		return false;
	felix_r41 = SDL_LoadBMP(felixr41_bmp);
	if (felix_r41 == NULL)
		return false;
	felix_r51 = SDL_LoadBMP(felixr51_bmp);
	if (felix_r51 == NULL)
		return false;
	felix_r12 = SDL_LoadBMP(felixr12_bmp);
	if (felix_r12 == NULL)
		return false;
	felix_r22 = SDL_LoadBMP(felixr22_bmp);
	if (felix_r22 == NULL)
		return false;
	felix_r32 = SDL_LoadBMP(felixr32_bmp);
	if (felix_r32 == NULL)
		return false;
	felix_r42 = SDL_LoadBMP(felixr42_bmp);
	if (felix_r42 == NULL)
		return false;
	felix_r52 = SDL_LoadBMP(felixr52_bmp);
	if (felix_r52 == NULL)
		return false;
	felix_d2 = SDL_LoadBMP(felixd2_bmp);
	if (felix_d2 == NULL)
		return false;
	felix_i2 = SDL_LoadBMP(felixi2_bmp);
	if (felix_i2 == NULL)
		return false;
	ralph_1 = SDL_LoadBMP(ralph1_bmp);
	if (ralph_1 == NULL)
		return false;
	ralph_2 = SDL_LoadBMP(ralph2_bmp);
	if (ralph_2 == NULL)
		return false;
	ralph_3 = SDL_LoadBMP(ralph3_bmp);
	if (ralph_3 == NULL)
		return false;
	ralph_4 = SDL_LoadBMP(ralph4_bmp);
	if (ralph_4 == NULL)
		return false;
	ralph_5 = SDL_LoadBMP(ralph5_bmp);
	if (ralph_5 == NULL)
		return false;
	ralph_6 = SDL_LoadBMP(ralph6_bmp);
	if (ralph_6 == NULL)
		return false;
	pajaro_1 = SDL_LoadBMP(pajaro1_bmp);
	if (pajaro_1 == NULL)
		return false;
	pajaro_2 = SDL_LoadBMP(pajaro2_bmp);
	if (pajaro_2 == NULL)
		return false;
	roca1 = SDL_LoadBMP(roca1_bmp);
	if (roca1 == NULL)
		return false;
	roca2 = SDL_LoadBMP(roca2_bmp);
	if (roca2 == NULL)
		return false;
	torta = SDL_LoadBMP(torta_bmp);
	if (torta == NULL)
		return false;

	return true;
}

void liberarRecursos() {
	//cout << "liberar recursos" << endl;
	//SOCKET
	if (socketTorneo != NULL) {
		delete (socketTorneo);
	}

	if (socketPartida != NULL) {
		delete (socketPartida);
	}

	//SEM
	pthread_mutex_destroy(&mutex_msjPuertoRecibido);
	pthread_mutex_destroy(&mutex_cola_grafico);
	pthread_mutex_destroy(&mutex_torneoFinalizado);
	pthread_mutex_destroy(&mutex_nombreOponente);
	pthread_mutex_destroy(&mutex_murioServidorTorneo);

	//SDL
	TTF_Quit();
	SDL_Quit();
}

bool nuevaPartidaSolicitada() {
	bool solicitud;
	pthread_mutex_lock(&mutex_solicitudDeNuevaParitda);
	solicitud = solicitudDeNuevaParitda;
	pthread_mutex_unlock(&mutex_solicitudDeNuevaParitda);
	return solicitud;
}

bool torneoFinalizo() {
	bool estado;
	pthread_mutex_lock(&mutex_torneoFinalizado);
	estado = torneoFinalizado;
	pthread_mutex_unlock(&mutex_torneoFinalizado);
	return estado;
}

bool murioServidorDelTorneo() {
	bool estado;
	pthread_mutex_lock(&mutex_murioServidorTorneo);
	estado = murioServidorTorneo;
	pthread_mutex_unlock(&mutex_murioServidorTorneo);
	return estado;
}

void inicializarVariablesDeLaPartida() {
	//inicializacion de variables
	felix1_posicion.fila = 0;
	felix1_posicion.columna = 0;
	felix2_posicion.fila = 0;
	felix2_posicion.columna = EDIFICIO_COLUMNAS - 1;
	ralph_posicion.fila = 3;
	ralph_posicion.columna = 2;
	pajaro_desplazamiento.x = -1;
	pajaro_desplazamiento.y = -1;

	rahlp_x = PARED_X + 200;
	rahlp_y = PARED_Y;
	roca_siguiente = 0;
	cant_rocas = 0;
	ralph_destino = 0;
	tramo = 1;
	nivel = 1;
	salir = 'N';
	ralph_moverse = 'N';
	pajaro_moverse = 'N';
	felix1_reparar = 'N';
	felix2_reparar = 'N';
	ventanas_cargadas = 'N';
	torta_aparece = 'N';
	strcpy(felix_cartel_puntos, "0");
	strcpy(felix_cartel_vidas, "0");
	//felix1_nombre = ""; NO CAMBIA
	//felix2_nombre = ""; LO RECIBO ARRIBA (me lo manda el servTorneo)
	felix1_puntos = 0;
	felix2_puntos = 0;
	felix1_vidas = 0;
	felix2_vidas = 0;
	ventanas_reparadas = 10;
	felix2_inicial = true;
	felix1_inicial = true;

	for (int i = 0; i < 20; i++) {
		rocas_desplazamiento[i].x = 0;
		rocas_desplazamiento[i].y = 0;
	}
}
