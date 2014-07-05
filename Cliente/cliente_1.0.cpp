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
#include "Clases/CommunicationSocket.h"
#include <iostream>
#include <string>
#include <queue>
#include "Support/Constantes.h"
#include <algorithm>

#define ANCHO_PANTALLA	640
#define ALTO_PANTALLA 480
#define BPP 8

#define PARED_X 90
#define PARED_Y 110

using namespace std;

bool msjPuertoRecibido = false;
pthread_mutex_t mutex_msjPuertoRecibido = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_grafico = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_nombreOponente = PTHREAD_MUTEX_INITIALIZER;

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

/* 

 0 - Pantalla intermedia para ingresar el nombre.
 1 - Pantalla intermedia de cambio de tramo.
 2 - Pantalla intermedia a la espera de partidas.

 */

SDL_Surface *superficie,
*backgroundImg, *pared_tramo1n1, *pared_tramo2n1, *pared_tramo3n1, *pared, *ventana_sana, *ventana_rota1, *ventana_rota2, *ventana_rota3, *ventana_rota4, *ventana, *puerta, *felix_d1, *felix_i1, *felix_r11, *felix_r21, *felix_r31, *felix_r41, *felix_r51, *felix_d2, *felix_i2, *felix1, *felix2,
		*ralph_1, *ralph_2, *ralph_3, *ralph_4, *ralph_5, *ralph_6, *ralph, *roca1, *roca2, *roca, *pajaro_1, *pajaro_2, *pajaro, *texto, *puntos, *vidas, *torta;

struct ventana ventanas_tramo1[3][5];
/* ALMACENO FILA y COLUMNA -- pienso el edificio como una matriz */

struct posicion felix1_posicion = { 0, 0 };
struct posicion ralph_posicion = { 3, 2 };
struct posicion torta_posicion;
struct desplazamiento pajaro_desplazamiento = { -1, -1 };
struct desplazamiento rocas_desplazamiento[20];

unsigned short int rahlp_x = PARED_X + 200;
unsigned short int rahlp_y = PARED_Y;
short int roca_siguiente = 0;
short int cant_rocas = 0;
short int ralph_destino = 0;
int puertoServidorPartida = 0;
unsigned short int tramo = 1;
unsigned short int nivel = 1;
pthread_t tpid_teclas, tpid_escuchar, tpid_envia, tpid_escuchar_torneo;
char salir = 'N';
char ralph_moverse = 'N';
char pajaro_moverse = 'N';
char ralph_sentido;
char felix1_reparar = 'N';
char ventanas_cargadas = 'N';
char torta_aparece = 'N';
char felix_cartel_puntos[10] = { 0 };
char felix_cartel_vidas[10] = { 0 };
string felix1_nombre = "";
string felix2_nombre = "";
short int felix1_puntos = 0;
short int felix2_puntos = 0;
short int felix1_vidas = 3;
short int felix2_vidas = 3;
short int ventanas_reparadas = 10;
bool felix2_inicial = true;
bool felix1_inicial = true;
SDL_Event evento;
SDL_keysym keysym;

SDL_Rect pantalla_juego, pantalla_texto, pantalla_puntos, pantalla_vidas, posBackground;

SDL_Color color_texto;
TTF_Font *fuente;

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

int main(int argc, char *argv[]) {
	CommunicationSocket * socketTorneo;
	CommunicationSocket * socketPartida;

	const char pared_tramo1n1_bmp[] = "Sprites/pared_tramo1n1.bmp", pared_tramo2n1_bmp[] = "Sprites/pared_tramo2n1.bmp", pared_tramo3n1_bmp[] = "Sprites/pared_tramo3n1.bmp", ventana_sana_bmp[] = "Sprites/ventana_sana.bmp", ventana_rota1_bmp[] = "Sprites/ventana_rota1.bmp", ventana_rota2_bmp[] =
			"Sprites/ventana_rota2.bmp", ventana_rota3_bmp[] = "Sprites/ventana_rota3.bmp", ventana_rota4_bmp[] = "Sprites/ventana_rota4.bmp", puerta_bmp[] = "Sprites/puerta_grande.bmp", felixd1_bmp[] = "Sprites/felix_d1.bmp", felixi1_bmp[] = "Sprites/felix_i1.bmp", felixr11_bmp[] =
			"Sprites/felix_r11.bmp", felixr21_bmp[] = "Sprites/felix_r21.bmp", felixr31_bmp[] = "Sprites/felix_r31.bmp", felixr41_bmp[] = "Sprites/felix_r41.bmp", felixr51_bmp[] = "Sprites/felix_r51.bmp", felixd2_bmp[] = "Sprites/felix_d2.bmp", felixi2_bmp[] = "Sprites/felix_i2.bmp", ralph1_bmp[] =
			"Sprites/rahlp_1.bmp", ralph2_bmp[] = "Sprites/rahlp_2.bmp", ralph3_bmp[] = "Sprites/rahlp_3.bmp", ralph4_bmp[] = "Sprites/rahlp_4.bmp", ralph5_bmp[] = "Sprites/rahlp_5.bmp", ralph6_bmp[] = "Sprites/rahlp_6.bmp", pajaro1_bmp[] = "Sprites/pajaro_1.bmp", pajaro2_bmp[] =
			"Sprites/pajaro_2.bmp", roca1_bmp[] = "Sprites/roca1.bmp", roca2_bmp[] = "Sprites/roca2.bmp", torta_bmp[] = "Sprites/torta.bmp";

	pared_tramo1n1 = SDL_LoadBMP(pared_tramo1n1_bmp);
	pared_tramo2n1 = SDL_LoadBMP(pared_tramo2n1_bmp);
	pared_tramo3n1 = SDL_LoadBMP(pared_tramo3n1_bmp);
	ventana_sana = SDL_LoadBMP(ventana_sana_bmp);
	ventana_rota1 = SDL_LoadBMP(ventana_rota1_bmp);
	ventana_rota2 = SDL_LoadBMP(ventana_rota2_bmp);
	ventana_rota3 = SDL_LoadBMP(ventana_rota3_bmp);
	ventana_rota4 = SDL_LoadBMP(ventana_rota4_bmp);
	puerta = SDL_LoadBMP(puerta_bmp);
	felix_d1 = SDL_LoadBMP(felixd1_bmp);
	felix_i1 = SDL_LoadBMP(felixi1_bmp);
	felix_r11 = SDL_LoadBMP(felixr11_bmp);
	felix_r21 = SDL_LoadBMP(felixr21_bmp);
	felix_r31 = SDL_LoadBMP(felixr31_bmp);
	felix_r41 = SDL_LoadBMP(felixr41_bmp);
	felix_r51 = SDL_LoadBMP(felixr51_bmp);
	felix_d2 = SDL_LoadBMP(felixd2_bmp);
	felix_i2 = SDL_LoadBMP(felixi2_bmp);
	ralph_1 = SDL_LoadBMP(ralph1_bmp);
	ralph_2 = SDL_LoadBMP(ralph2_bmp);
	ralph_3 = SDL_LoadBMP(ralph3_bmp);
	ralph_4 = SDL_LoadBMP(ralph4_bmp);
	ralph_5 = SDL_LoadBMP(ralph5_bmp);
	ralph_6 = SDL_LoadBMP(ralph6_bmp);
	pajaro_1 = SDL_LoadBMP(pajaro1_bmp);
	pajaro_2 = SDL_LoadBMP(pajaro2_bmp);
	roca1 = SDL_LoadBMP(roca1_bmp);
	roca2 = SDL_LoadBMP(roca2_bmp);
	torta = SDL_LoadBMP(torta_bmp);

	for (int i = 0; i < 20; i++) {
		rocas_desplazamiento[i].x = 0;
		rocas_desplazamiento[i].y = 0;
	}

	//Inicio modo video
	SDL_Init(SDL_INIT_VIDEO);
	//Inicio modo texto grafico
	TTF_Init();
	signal(SIGINT, handler);
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

	//Conexion con el servidor de torneo.
	//Muestro pantalla de "esperando al servidor por nueva partida"
	mostrarPantalla("waitmatch");

	cout << "Intentando conectarme al torneo" << endl;
	bool error = true;
	do {
		cout << "Esperando servidor" << endl;
		try {
			socketTorneo = new CommunicationSocket(puertoTorneo, (char*) ip.c_str());
			error = false;
		} catch (const char *err) {
			cout << "mensaje : " << err << endl;
			cout << "No se puedo conectar al torneo" << endl;
			error = true;
			usleep(1000000);
		}
	} while (error == true);

	cout << "Conectado" << endl;

	socketTorneo->SendNoBloq(felix1_nombre.c_str(), sizeof(felix1_nombre));
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	bzero(buffer, sizeof(buffer));
	socketTorneo->ReceiveBloq(buffer, sizeof(buffer));
	string aux_buffer(buffer);
	string mi_id = aux_buffer.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO);
	cout << "Mi id: " << mi_id << endl;

	//mando mi nombre de jugador
	string messageNombre(CD_NOMBRE);
	messageNombre.append(fillMessage(felix1_nombre));
	socketTorneo->SendBloq(messageNombre.c_str(), messageNombre.length());

	//Thread para escuchar al servidor de Torneo.
	pthread_create(&tpid_escuchar_torneo, NULL, EscuchaTorneo, &socketTorneo->ID);

	//recibo el nombre de mi oponente (me lo manda el servTorneo)
	int recibioNombreOponente = false;
	nombreOponente = "";
	while (!recibioNombreOponente) {
		pthread_mutex_lock(&mutex_nombreOponente);
		if (nombreOponente.length() > 0) {
			felix2_nombre = nombreOponente;
			recibioNombreOponente = true;
		}
		pthread_mutex_unlock(&mutex_nombreOponente);
		sleep(1);
	}
	//felix2_nombre = nombreOponente;
	cout << "recibo el nombre de mi oponente:" << felix2_nombre << endl;
	//felix2_nombre = "Jugador 2";

	//Espero que el servidor de Torneo me envie el puerto para conectarme al servidor de partida.
	bool msjPuerto = false;
	while (true) {
		pthread_mutex_lock(&mutex_msjPuertoRecibido);
		msjPuerto = msjPuertoRecibido;
		pthread_mutex_unlock(&mutex_msjPuertoRecibido);

		if (msjPuerto == true) {
			break;
		}
		usleep(1000000);
	}
	msjPuertoRecibido = false;

	//Me conecto al servidor de partida.
	cout << "Antes de inicializar el socketPartida " << puertoServidorPartida << endl;
	socketPartida = new CommunicationSocket(puertoServidorPartida, (char*) ip.c_str());

	//mando mi ID
	string message(CD_ID_JUGADOR);
	message.append(fillMessage(mi_id));
	socketPartida->SendBloq(message.c_str(), message.length());

	//Empiezo a tirar Thread para comunicarme con el servidor de partida.
	pthread_create(&tpid_teclas, NULL, EscuchaServidor, &socketPartida->ID);
	pthread_create(&tpid_envia, NULL, EnvioServidor, &socketPartida->ID);

	//Lanzo el thread que va a estar a la escucha de las teclas que se presionan.
	pthread_create(&tpid_teclas, NULL, EscuchaTeclas, NULL);

	unsigned short int fila, columna;
	unsigned short int ventana_x, ventana_y;

	while (salir == 'N') {
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

		//Dibujo los puntos.
		pantalla_puntos.x = 10;
		pantalla_puntos.y = 30;
		sprintf(felix_cartel_puntos, "Puntos %d", felix1_puntos);
		puntos = TTF_RenderText_Solid(fuente, felix_cartel_puntos, color_texto);
		SDL_BlitSurface(puntos, NULL, superficie, &pantalla_puntos);

		pantalla_puntos.x = 520;
		sprintf(felix_cartel_puntos, "Puntos %d", felix2_puntos);
		puntos = TTF_RenderText_Solid(fuente, felix_cartel_puntos, color_texto);
		SDL_BlitSurface(puntos, NULL, superficie, &pantalla_puntos);

		//Dibujo las vidas.
		pantalla_vidas.x = 10;
		pantalla_vidas.y = 50;
		sprintf(felix_cartel_vidas, "Vidas %d", felix1_vidas);
		vidas = TTF_RenderText_Solid(fuente, felix_cartel_vidas, color_texto);
		SDL_BlitSurface(vidas, NULL, superficie, &pantalla_vidas);

		pantalla_vidas.x = 520;
		sprintf(felix_cartel_vidas, "Vidas %d", felix2_vidas);
		vidas = TTF_RenderText_Solid(fuente, felix_cartel_vidas, color_texto);
		SDL_BlitSurface(vidas, NULL, superficie, &pantalla_vidas);

		//Dibujo el texto.
		pantalla_texto.x = 10;
		pantalla_texto.y = 10;
		texto = TTF_RenderText_Solid(fuente, felix1_nombre.c_str(), color_texto);
		SDL_BlitSurface(texto, NULL, superficie, &pantalla_texto);

		pantalla_texto.x = 520;
		texto = TTF_RenderText_Solid(fuente, felix2_nombre.c_str(), color_texto);
		SDL_BlitSurface(texto, NULL, superficie, &pantalla_texto);

		// cargar las ventanas del tramo 1 -- fila 0 es la de mas abajo.
		if (ventanas_cargadas == 'N') {
			ventana_x = PARED_X + 45, ventana_y = PARED_Y + 25;
			for (fila = 3; fila > 0; fila--) {
				for (columna = 0; columna < 5; columna++) {
					ventanas_tramo1[fila - 1][columna].tipo_ventana = rand() % 5;
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
				cout << "Columna destino " << ralph_destino << endl;
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
				cout << "Fila comienzo: " << pajaro_fila << endl;
				pajaro_desplazamiento.x = 10;
				pajaro_desplazamiento.y = ventanas_tramo1[pajaro_fila][0].y;
				pajaro_moverse = 'S';
			}
		}
		//Dibujo a Felix
		if (felix1_reparar == 'N') {
			if (felix1 == NULL)
				felix1 = felix_d1;
			if (felix1_posicion.fila == 99) {
				Dibujar(110, 400, felix1, superficie);
			} else {
				if (felix1_inicial == true) {
					Dibujar(120, 350, felix1, superficie);
					felix1_inicial = false;
				} else {
					if (!cola_felix1.empty()) {
						cout<<"Entro a la cola de felix movimiento"<<endl;
						felix1_inicial = false;
						string msj = cola_felix1.front();
						cola_felix1.pop();
						felix1_posicion.columna = atoi(msj.substr(5, 1).c_str());
						cout<<"Nueva posicion columna de felix1"<<felix1_posicion.columna<<endl;
						felix1_posicion.fila = atoi(msj.substr(6, 1).c_str());
						cout<<"Nueva posicion fila de felix1: "<<felix1_posicion.fila<<endl;
					}
					Dibujar(ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].x, ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].y, felix1, superficie);
				}
			}
		} else {
			cout << "inicia repara ventana" << endl;
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
				if (ventana_reparada(&felix1_posicion) == 'S') {
					felix1_puntos++;
					ventanas_reparadas++;
				}
				felix1_reparar = 'N';
			}
			cout << "casi termina repara ventana" << endl;
			Dibujar(ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].x, ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].y, felix1, superficie);
			cout << "termino repara ventana" << endl;
		}

		if (felix2 == NULL)
			felix2 = felix_d2;

		//Mueveo a felix2, salvo que este en la posicion inicial
		if (!cola_felix2.empty()) {
			felix2_inicial = false;
			string msj = cola_felix2.front();
			cola_felix2.pop();
			short int f2_fila = atoi(msj.substr(5, 1).c_str());
			short int f2_colu = atoi(msj.substr(6, 1).c_str());

			Dibujar(ventanas_tramo1[f2_fila][f2_colu].x, ventanas_tramo1[f2_fila][f2_colu].y, felix2, superficie);
		}

		if (felix2_inicial == true)
			Dibujar(120, 405, felix2, superficie);

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
		//Ojo, hay que poner el delay porque sino el proceso no tiene tiempos muertos y
		// el uso del procesador se me va al chori.
		if (hayChoque()) {
			felix1_posicion.fila = 0;
			felix1_posicion.columna = 0;

			string message(CD_PERDIDA_VIDA);
			message.append(fillMessage("0"));
			cola_grafico.push(message);

			if (felix1_vidas > 0) {
				felix1_vidas--;
				felix1_inicial = true;
			}
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

	//esperar mientras las demas partidas no han finalizado. (mostrar msj "GameOver. waiting for rankings.. ")
	mostrarPantalla("gameover");

	while (!showWindowRanking) {
		sleep(1);
	}
	mostrarRanking(ranking);

	cout << "Ingrese un tecla para terminar: ";
	getchar();

	//Liberar recursos
	liberarRecursos();
	return 0;
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

void* EscuchaServidor(void *arg) {
	int fd = *(int *) arg;
	int readData = 0;
	int codigo;
	CommunicationSocket cSocket(fd);
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	bzero(buffer, sizeof(buffer));
	while (salir == 'N') {
		//cout << "Espero msj del servidor de partida ... " << endl;
		readData = cSocket.ReceiveBloq(buffer, sizeof(buffer));
		if (strlen(buffer) > 0) {
			string aux_buffer(buffer);
			codigo = atoi(aux_buffer.substr(0, LONGITUD_CODIGO).c_str());

			if (aux_buffer.compare("9900000") != 0) {
				cout << "Recibi: " << buffer << endl;
			}

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
				} else
					cola_felix2.push(aux_buffer);
				break;
			case CD_PERDIDA_VIDA_I:
				break;
			case CD_ID_JUGADOR_I:
				break;
			case CD_ACK_I:
				string message(CD_ACK);
				message.append(fillMessage("0"));
				//TODO Agregar mutex.
				cola_grafico.push(message);
				break;
			}
		} else if (readData == 0) {
			cout << "Murio el servidor de partida" << endl;

			//si no se llego a fin del torneo
			//busco en una  nueva partida (que me la del torneo)


			//pthread_exit(0);
			//salir='S';
		}
		//sleep(1);
		usleep(10000);
	}
	pthread_exit(NULL);
}

void * EnvioServidor(void * arg) {
	int fd = *(int *) arg;
	CommunicationSocket cSocket(fd);
	while (salir == 'N') {
		if (!cola_grafico.empty()) {
			string mensaje = cola_grafico.front();
			cola_grafico.pop();
			cout << "Mensaje a enviar al servPartida: " << mensaje.c_str() << endl;
			cSocket.SendBloq(mensaje.c_str(), mensaje.length());
		}
		usleep(10000);
	}
	pthread_exit(NULL);
}

void* EscuchaTorneo(void *arg) {
	int fd = *(int *) arg;
	int readData = 0;
	const char* mensajeNombre;
	string auxNombreOponente;
	cout << "FD: " << fd << endl;
	CommunicationSocket cSocket(fd);
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	bzero(buffer, sizeof(buffer));
	while (salir == 'N') {
		//cout << "Espero msj del servidor de Torneo ... " << endl;
		readData = cSocket.ReceiveBloq(buffer, sizeof(buffer));
		if (strlen(buffer) > 0) {
			string aux_buffer(buffer);

			if (aux_buffer.compare("9900000") != 0) {
				cout << "Recibi: " << buffer << endl;
			}

			int codigo = atoi((aux_buffer.substr(0, LONGITUD_CODIGO).c_str()));
			switch (codigo) {
			//case CD_ID_JUGADOR_I:
			//	break;
			case CD_PUERTO_PARTIDA_I:
				pthread_mutex_lock(&mutex_msjPuertoRecibido);
				puertoServidorPartida = atoi(aux_buffer.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str());
				cout << "Puerto: " << puertoServidorPartida << endl;
				msjPuertoRecibido = true;
				pthread_mutex_unlock(&mutex_msjPuertoRecibido);
				break;
			case CD_RANKING_I:
				cout << "RANKING #" << aux_buffer.substr(LONGITUD_CODIGO + LONGITUD_CONTENIDO - 2, 2).c_str() << endl;
				ranking = aux_buffer.substr(LONGITUD_CODIGO + LONGITUD_CONTENIDO - 2, 2).c_str();

				//ESTO NO DEBERIA SER ASI, EL JUGADOR TIENE QUE RECONOCER QUE SE TERMINO LA PARTIDA
				//HARCODEADO PARA PROBAR LA PANTALLA DE RANKING
				salir = 'S';

				showWindowRanking = true;
				//salgo del thread porque este el ultimo mensaje que me interesa
				pthread_exit(NULL);
				break;
			case CD_NOMBRE_I:
				//recibo y limpio el nombre
				mensajeNombre = aux_buffer.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str();
				auxNombreOponente = "";
				int caracterNomb;
				for (caracterNomb = 0; caracterNomb < LONGITUD_CONTENIDO; caracterNomb++) {
					if (mensajeNombre[caracterNomb] != '0') {
						auxNombreOponente += mensajeNombre[caracterNomb];
					}
				}
				pthread_mutex_lock(&mutex_nombreOponente);
				nombreOponente = auxNombreOponente;
				pthread_mutex_unlock(&mutex_nombreOponente);
				break;
			case CD_FIN_TORNEO_I:
				//que no busque mas establecer partidas
				torneoFinalizado = true;
				cout << "Fin de Toreno:" << aux_buffer.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str() << endl;
				break;
			case CD_ACK_I:
				string message(CD_ACK);
				string content = "1";
				message.append(fillMessage(content));
				cSocket.SendNoBloq(message.c_str(), LONGITUD_CODIGO);
				break;
			}
		} else if (readData == 0) {
			cout << "Murio el servidor torneo" << endl;

			/// dar aviso de que murio y cerrar todo

			murioServidorTorneo = true;
		}
		usleep(5000);
	}

	pthread_exit(NULL);
}

void* EscuchaTeclas(void *arg) {

	SDL_Event evento; //Con esta variable reconozco si se apreto una tecla o se apreto el mouse.
	SDL_keysym keysym; //Con esta variable reconzco el codigo de la tecla que se apreto.
	short int f1_fila;
	short int f1_colu;

	//Lupeo escuchando el teclado.
	while (SDL_WaitEvent(&evento) != 0) {

		f1_fila = felix1_posicion.fila;
		f1_colu = felix1_posicion.columna;

		switch (evento.type) {

		case SDL_KEYDOWN:
			if (evento.key.keysym.sym == SDLK_DOWN || evento.key.keysym.sym == key_abajo) {
				/*if ((felix1_posicion.fila - 1) >= 0)
					if ((felix1_posicion.fila - 1) != 0)
						f1_colu = felix1_posicion.fila - 1;
					else if ((felix1_posicion.columna) != 2)
						f1_colu = felix1_posicion.fila - 1;*/
				felix1_reparar = 'N';

				ostringstream ss1;
				ostringstream ss2;
				ss1 << felix1_posicion.columna;
				ss2 << felix1_posicion.fila-1;
				string aux(ss1.str() + ss2.str());
				string message(CD_MOVIMIENTO_FELIX);
				message.append(fillMessage(aux));
				//cout << "MENSAJE: " << message << endl;
				cola_grafico.push(message);

			} else if (evento.key.keysym.sym == SDLK_UP || evento.key.keysym.sym == key_arriba) {
				/*if (felix1_posicion.columna == 99) {
					f1_fila = 0;
					f1_colu = 0;
				} else if ((felix1_posicion.fila + 1) < 3)
					f1_fila = felix1_posicion.fila +1;*/
					//f1_colu = felix1_posicion.fila + 1;
				felix1_reparar = 'N';

				ostringstream ss1;
				ostringstream ss2;
				ss1 << felix1_posicion.columna;
				ss2 << felix1_posicion.fila+1;
				//ss1 << felix1_posicion.fila;
				//ss2 << felix1_posicion.columna;
				string aux(ss1.str() + ss2.str());
				string message(CD_MOVIMIENTO_FELIX);
				message.append(fillMessage(aux));
				cout << "MENSAJE: " << message << endl;
				cola_grafico.push(message);

			} else if (evento.key.keysym.sym == SDLK_RIGHT || evento.key.keysym.sym == key_derecha) {
				felix1 = felix_d1;
				/*if (felix1_posicion.fila == 99) {
					f1_fila = 0;
					f1_colu = 0;
				} else if ((felix1_posicion.columna + 1) < 5)
					if ((felix1_posicion.columna + 1) != 2)
						f1_colu = felix1_posicion.columna + 1;
					else if ((felix1_posicion.fila) != 0)
						f1_colu = felix1_posicion.columna + 1;*/
				felix1_reparar = 'N';

				ostringstream ss1;
				ostringstream ss2;
				ss1 << felix1_posicion.columna+1;
				ss2 << felix1_posicion.fila;
				string aux(ss1.str() + ss2.str());
				string message(CD_MOVIMIENTO_FELIX);
				message.append(fillMessage(aux));
				cout << "MENSAJE: " << message << endl;
				cola_grafico.push(message);

			} else if (evento.key.keysym.sym == SDLK_LEFT || evento.key.keysym.sym == key_izquierda) {
				felix1 = felix_i1;
				/*if ((felix1_posicion.columna - 1) >= 0)
					if ((felix1_posicion.columna - 1) != 2)
						f1_colu = felix1_posicion.columna - 1;
					else if ((felix1_posicion.fila) != 0)
						f1_colu = felix1_posicion.columna - 1;*/
				felix1_reparar = 'N';

				ostringstream ss1;
				ostringstream ss2;
				ss1 << felix1_posicion.columna-1;
				ss2 << felix1_posicion.fila;
				string aux(ss1.str() + ss2.str());
				string message(CD_MOVIMIENTO_FELIX);
				message.append(fillMessage(aux));
				cout << "MENSAJE: " << message << endl;
				cola_grafico.push(message);

			} else if (evento.key.keysym.sym == SDLK_SPACE || evento.key.keysym.sym == key_accion) {
				felix1_reparar = 'S';
			} else if (evento.key.keysym.sym == key_salir) {
				salir = 'S';
			}
			break;
		case SDLK_ESCAPE:
			break;
		case SDL_QUIT:
			salir = 'S';
			break;
		}
		usleep(100000);
	}
	pthread_exit(NULL);
}

char ventana_reparada(struct posicion *felix_posicion) {

	cout << "Ventana Sana valor " << ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].sana << endl;
	if (ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].tipo_ventana != 0) {
		if (ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].sana < 3) {
			ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].sana++;
			if (ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].sana == 3) {
				ventanas_tramo1[felix_posicion->fila][felix_posicion->columna].tipo_ventana = 0;
				cout << "Ventana reparada " << felix_posicion->fila << ":" << felix_posicion->columna << endl;
				return 'S';
			} else
				return 'N';
		} else
			return 'N';
	}
}

void handler(int senial) {

	switch (senial) {
	case SIGINT:
		liberarRecursos();
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
	texto = TTF_RenderText_Solid(fuente, "Cambio de tramo, puto!", color_texto);
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
	texto = TTF_RenderText_Solid(fuente, "Cambio de nivel, gay", color_texto);
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
	string dir = "Sprites/Mensajes/";
	dir += nombrPantalla;
	dir += ".bmp";
	backgroundImg = SDL_LoadBMP(dir.c_str());
	if (backgroundImg == NULL) {
		printf("Error en SDL_LoadBMP= %s\n", SDL_GetError());
		exit(1);
	}
	SDL_BlitSurface(backgroundImg, NULL, superficie, &posBackground);
	SDL_Flip(superficie);
}

void mostrarRanking(const char* ranking) {
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
}

void liberarRecursos() {
	//SEM
	pthread_mutex_destroy(&mutex_msjPuertoRecibido);
	pthread_mutex_destroy(&mutex_cola_grafico);
	pthread_mutex_destroy(&mutex_nombreOponente);

	//SDL
	SDL_FreeSurface(superficie);
	SDL_FreeSurface(backgroundImg);
	SDL_FreeSurface(pared_tramo1n1);
	SDL_FreeSurface(pared_tramo2n1);
	SDL_FreeSurface(pared_tramo3n1);
	SDL_FreeSurface(pared);
	SDL_FreeSurface(ventana_sana);
	SDL_FreeSurface(ventana_rota1);
	SDL_FreeSurface(ventana_rota2);
	SDL_FreeSurface(ventana_rota3);
	SDL_FreeSurface(ventana_rota4);
	SDL_FreeSurface(ventana);
	SDL_FreeSurface(puerta);
	SDL_FreeSurface(felix_d1);
	SDL_FreeSurface(felix_i1);
	SDL_FreeSurface(felix_r11);
	SDL_FreeSurface(felix_r21);
	SDL_FreeSurface(felix_r31);
	SDL_FreeSurface(felix_r41);
	SDL_FreeSurface(felix_r51);
	SDL_FreeSurface(felix_d2);
	SDL_FreeSurface(felix_i2);
	SDL_FreeSurface(felix1);
	SDL_FreeSurface(felix2);
	SDL_FreeSurface(ralph_1);
	SDL_FreeSurface(ralph_2);
	SDL_FreeSurface(ralph_3);
	SDL_FreeSurface(ralph_4);
	SDL_FreeSurface(ralph_5);
	SDL_FreeSurface(ralph_6);
	SDL_FreeSurface(ralph);
	SDL_FreeSurface(roca1);
	SDL_FreeSurface(roca2);
	SDL_FreeSurface(roca);
	SDL_FreeSurface(pajaro_1);
	SDL_FreeSurface(pajaro_2);
	SDL_FreeSurface(pajaro);
	SDL_FreeSurface(texto);
	SDL_FreeSurface(puntos);
	SDL_FreeSurface(vidas);
	SDL_FreeSurface(torta);
	SDL_Quit();
	TTF_CloseFont(fuente);
	TTF_Quit();
}
