#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
//#include <SDL/SDL_image.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include "Clases/CommunicationSocket.h"
#include <iostream>
#include <string>
#include <queue>
#include "Support/Constantes.h"

#define ANCHO_PANTALLA	640
#define ALTO_PANTALLA 480
#define BPP 8

#define PARED_X 90
#define PARED_Y 110

using namespace std;

struct ventana {

	unsigned short int x;
	unsigned short int y;
	unsigned short int fila;
	unsigned short int columna;
	unsigned short int numero;
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

void handler(int);
void ConfigRect(SDL_Rect *, int, int, int, int);
void Dibujar(int, int, SDL_Surface *, SDL_Surface *);
void DibujarVentanas(struct ventana[][5], unsigned short int, SDL_Surface *,
		SDL_Surface*);
void CargarVentanasDelTramo(struct ventana*, unsigned short int,
		unsigned short int, unsigned short int, char, unsigned short int,
		unsigned short int);
void *EscuchaTeclas(void *);
void *EnvioServidor(void *);
void *EscuchaServidor(void *);
void *ControlaMovimientos(void *);
string fillMessage(string);
void *DibujarPantalla(void *); //Se usará mas adelante.

SDL_Surface *superficie, *pared, *ventana_sana, *ventana_rota1, *ventana_rota2,
		*ventana_rota3, *ventana_rota4, *puerta, *felix_d1, *felix_i1,
		*felix_d2, *felix_i2, *felix1, *felix2, *ralph_1, *ralph_2, *ralph_3,
		*ralph_4, *ralph_5, *ralph_6, *ralph, *roca1, *roca2, *roca, *pajaro_1,
		*pajaro_2, *pajaro, *texto;

struct ventana ventanas_tramo1[3][5];
/* ALMACENO FILA y COLUMNA -- pienso el edificio como una matriz */

struct posicion felix_posicion = { 99, 99 };
struct posicion ralph_posicion = { 3, 2 };
struct posicion pajaro_posicion = { 1, 0 };
struct desplazamiento rocas_desplazamiento[20];

unsigned short int rahlp_x = PARED_X + 200;
unsigned short int rahlp_y = PARED_Y;
short int roca_siguiente = 0;
short int cant_rocas = 0;
unsigned short int tramo = 1;
unsigned short int nivel = 1;
pthread_t tpid_teclas, tpid_escuchar, tpid_ctrlmov, tpid_envia;
char salir = 'N';
char buffer[256];
SDL_Event evento;
SDL_keysym keysym;

int main(int argc, char *argv[]) {
	CommunicationSocket * socket;

	SDL_Rect pantalla_juego, pantalla_texto;

	SDL_Color color_texto;
	TTF_Font *fuente;

	short int caracter = 0;
	char nombre[10] = { ' ' };

	const char pared_bmp[] = "Sprites/pared_grande_tramo1.bmp",
			ventana_sana_bmp[] = "Sprites/ventana_sana.bmp",
			ventana_rota1_bmp[] = "Sprites/ventana_rota1.bmp",
			ventana_rota2_bmp[] = "Sprites/ventana_rota2.bmp",
			ventana_rota3_bmp[] = "Sprites/ventana_rota3.bmp",
			ventana_rota4_bmp[] = "Sprites/ventana_rota4.bmp", puerta_bmp[] =
					"Sprites/puerta_grande.bmp", felixd1_bmp[] =
					"Sprites/felix_d1.bmp", felixi1_bmp[] =
					"Sprites/felix_i1.bmp", felixd2_bmp[] =
					"Sprites/felix_d2.bmp", felixi2_bmp[] =
					"Sprites/felix_i2.bmp",
			ralph1_bmp[] = "Sprites/rahlp_1.bmp", ralph2_bmp[] =
					"Sprites/rahlp_2.bmp", ralph3_bmp[] = "Sprites/rahlp_3.bmp",
			ralph4_bmp[] = "Sprites/rahlp_4.bmp", ralph5_bmp[] =
					"Sprites/rahlp_5.bmp", ralph6_bmp[] = "Sprites/rahlp_6.bmp",
			pajaro1_bmp[] = "Sprites/pajaro_1.bmp", pajaro2_bmp[] =
					"Sprites/pajaro_2.bmp", roca1_bmp[] = "Sprites/roca1.bmp",
			roca2_bmp[] = "Sprites/roca2.bmp";

	pared = SDL_LoadBMP(pared_bmp);
	ventana_sana = SDL_LoadBMP(ventana_sana_bmp);
	ventana_rota1 = SDL_LoadBMP(ventana_rota1_bmp);
	ventana_rota1 = SDL_LoadBMP(ventana_rota2_bmp);
	ventana_rota1 = SDL_LoadBMP(ventana_rota3_bmp);
	ventana_rota1 = SDL_LoadBMP(ventana_rota4_bmp);
	puerta = SDL_LoadBMP(puerta_bmp);
	felix_d1 = SDL_LoadBMP(felixd1_bmp);
	felix_i1 = SDL_LoadBMP(felixi1_bmp);
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

	for (int i = 0; i < 20; i++) {
		rocas_desplazamiento[i].x = 0;
		rocas_desplazamiento[i].y = 0;
	}

	cout << "Intentando conectarme" << endl;

	socket = new CommunicationSocket(5556, "127.0.0.1");

	//socket->ReceiveBloq(buffer, sizeof(buffer));
	cout << "Conectado" << endl;
	pthread_create(&tpid_teclas, NULL, EscuchaServidor, &socket->ID);
	pthread_create(&tpid_envia, NULL, EnvioServidor, &socket->ID);

//Inicio modo video
	SDL_Init(SDL_INIT_VIDEO);
//Inicio modo texto grafico
	//TTF_Init();
	signal(SIGINT, handler);
//Defino las propiedades de la pantalla del juego
	superficie = SDL_SetVideoMode(ANCHO_PANTALLA, ALTO_PANTALLA, BPP,
	SDL_HWSURFACE);
//Seteo el titulo de la pantalla
	SDL_WM_SetCaption("Rahlp Tournament", NULL);
//Cargo la fuente
	//fuente = TTF_OpenFont("/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf",24);
//Color del texto
	//color_texto.r = color_texto.g = color_texto.b = 245;

//Pantalla de inicio.
//Acá se ingresa el nombre

//Dimensiones rectangulo donde irá el texto
	pantalla_texto.x = 10;
	pantalla_texto.y = 10;

//While que captura teclas para escribir el nombre en la pantalla de inicio.
// ---
//Modularizarlo despues.
// ---
	/*	while (salir == 'N') {
	 SDL_WaitEvent(&evento);
	 if(evento.key.keysym.sym != SDLK_SPACE){
	 pantalla_juego.x = 0;
	 pantalla_juego.y = 0;
	 pantalla_juego.w = ANCHO_PANTALLA;
	 pantalla_juego.h = ALTO_PANTALLA;
	 SDL_FillRect(superficie, &pantalla_juego,
	 SDL_MapRGB(superficie->format, 0, 0, 0));

	 switch(evento.key.keysym.sym){
	 case SDLK_a: if(caracter<9) nombre[caracter++] = 'a'; break;
	 case SDLK_b: if(caracter<9) nombre[caracter++] = 'b'; break;
	 case SDLK_c: if(caracter<9) nombre[caracter++] = 'c'; break;
	 case SDLK_d: if(caracter<9) nombre[caracter++] = 'd'; break;
	 case SDLK_e: if(caracter<9) nombre[caracter++] = 'e'; break;
	 case SDLK_f: if(caracter<9) nombre[caracter++] = 'f'; break;
	 case SDLK_g: if(caracter<9) nombre[caracter++] = 'g'; break;
	 case SDLK_h: if(caracter<9) nombre[caracter++] = 'h'; break;
	 case SDLK_i: if(caracter<9) nombre[caracter++] = 'i'; break;
	 case SDLK_j: if(caracter<9) nombre[caracter++] = 'j'; break;
	 case SDLK_k: if(caracter<9) nombre[caracter++] = 'k'; break;
	 case SDLK_l: if(caracter<9) nombre[caracter++] = 'l'; break;
	 case SDLK_m: if(caracter<9) nombre[caracter++] = 'm'; break;
	 case SDLK_n: if(caracter<9) nombre[caracter++] = 'n'; break;
	 case SDLK_o: if(caracter<9) nombre[caracter++] = 'o'; break;
	 case SDLK_p: if(caracter<9) nombre[caracter++] = 'p'; break;
	 case SDLK_q: if(caracter<9) nombre[caracter++] = 'q'; break;
	 case SDLK_r: if(caracter<9) nombre[caracter++] = 'r'; break;
	 case SDLK_s: if(caracter<9) nombre[caracter++] = 's'; break;
	 case SDLK_t: if(caracter<9) nombre[caracter++] = 't'; break;
	 case SDLK_u: if(caracter<9) nombre[caracter++] = 'u'; break;
	 case SDLK_v: if(caracter<9) nombre[caracter++] = 'v'; break;
	 case SDLK_w: if(caracter<9) nombre[caracter++] = 'w'; break;
	 case SDLK_x: if(caracter<9) nombre[caracter++] = 'x'; break;
	 case SDLK_y: if(caracter<9) nombre[caracter++] = 'y'; break;
	 case SDLK_z: if(caracter<9) nombre[caracter++] = 'z'; break;
	 case SDLK_BACKSPACE: nombre[caracter--] = ' '; break;
	 }
	 fflush(stdin);
	 evento.key.keysym.sym = SDLK_CLEAR;

	 //Texto del texto
	 texto = TTF_RenderText_Solid(fuente, nombre, color_texto);
	 SDL_BlitSurface(texto,NULL, superficie, &pantalla_texto);
	 SDL_Flip(superficie);
	 }
	 else
	 salir = 'S';
	 usleep(100000);
	 }*/
	salir = 'N';

//Empiezo a tirar Thread.

// 1. Para escuchar teclas. - echo
// 2. Para calcular movimientos. - echo
// 3. Para escuchar al servidor. - comentado.
// 4. Para enviarle cosas al servidor. - no ta.

//Lanzo el thread que va a estar a la escucha de las teclas que se presionan.
	pthread_create(&tpid_teclas, NULL, EscuchaTeclas, NULL);
	pthread_create(&tpid_ctrlmov, NULL, ControlaMovimientos, NULL);

	unsigned short int fila, columna;
	unsigned short int ventana_x, ventana_y;

	while (salir == 'N') {
		pantalla_juego.x = 0;
		pantalla_juego.y = 0;
		pantalla_juego.w = ANCHO_PANTALLA;
		pantalla_juego.h = ALTO_PANTALLA;

		SDL_FillRect(superficie, &pantalla_juego,
				SDL_MapRGB(superficie->format, 0, 0, 0));
//Dibujo la pared.
		Dibujar(PARED_X, PARED_Y, pared, superficie);

//Dibujo el texto.
		SDL_BlitSurface(texto, NULL, superficie, &pantalla_texto);

// Dibujo las ventanas -- las empiezo a numerar desde arriba.
		ventana_x = PARED_X + 45, ventana_y = PARED_Y + 25;
		for (fila = 3; fila > 0; fila--) {
			for (columna = 0; columna < 5; columna++) {
				CargarVentanasDelTramo(&ventanas_tramo1[fila - 1][columna],
						ventana_x, ventana_y, columna + (fila - 1) * 5, 'N',
						fila - 1, columna);
				ventana_x += 80;
			}
			ventana_x = PARED_X + 45;
			ventana_y += 113;
		}

		DibujarVentanas(ventanas_tramo1, 3, ventana_sana, superficie);

//Dibujo la puerta
		Dibujar(ventanas_tramo1[0][1].x + 65, 383, puerta, superficie);

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

		Dibujar(ventanas_tramo1[2][ralph_posicion.columna].x, PARED_Y - 100,
				ralph, superficie);
//Dibujo la paloma
		if (pajaro == pajaro_1)
			pajaro = pajaro_2;
		else
			pajaro = pajaro_1;
		Dibujar(
				ventanas_tramo1[pajaro_posicion.fila][pajaro_posicion.columna].x,
				ventanas_tramo1[pajaro_posicion.fila][pajaro_posicion.columna].y,
				pajaro, superficie);

//Dibujo a Ralph
		if (felix1 == NULL)
			felix1 = felix_d1;
		if (felix_posicion.fila == 99)
			Dibujar(110, 405, felix1, superficie);
		else {
			Dibujar(
					ventanas_tramo1[felix_posicion.fila][felix_posicion.columna].x,
					ventanas_tramo1[felix_posicion.fila][felix_posicion.columna].y,
					felix1, superficie);
		}
		if (felix2 == NULL)
			felix2 = felix_d2;
		Dibujar(120, 405, felix2, superficie);
//Dibujo las rocas
		roca = roca1;
		for (int i = 0; i < cant_rocas; i++) {
			if (rocas_desplazamiento[i].x != 0) {
				if (rocas_desplazamiento[i].y < 405) {
					Dibujar(rocas_desplazamiento[i].x,
							rocas_desplazamiento[i].y, roca, superficie);
					rocas_desplazamiento[i].y += 5;
				} else {
					rocas_desplazamiento[i].x = 0;
					rocas_desplazamiento[i].y = 0;
					cant_rocas--;
				}
			}
		}
		SDL_Flip(superficie);
//Ojo, hay que poner el delay porque sino el proceso no tiene tiempos muertos y 
// el uso del procesador se me va al chori.
		usleep(100000);
	}

	delete (socket);
	SDL_Quit();
	TTF_CloseFont(fuente);
	TTF_Quit();
	return 0;
}

void CargarVentanasDelTramo(struct ventana *ventana, unsigned short int x,
		unsigned short int y, unsigned short int nro, char N_S,
		unsigned short int f, unsigned short int c) {
	ventana->x = x;
	ventana->y = y;
	ventana->fila = f;
	ventana->columna = c;
	ventana->numero = nro;
	ventana->ocupado = N_S;

//			printf("X: %d\tY: %d\tNro: %d\tOcupado: %c\n", x, y, nro, N_S);
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

void DibujarVentanas(struct ventana ventanas[][5],
		unsigned short int cant_filas, SDL_Surface *ventana_sana,
		SDL_Surface *superficie) {

	unsigned short int fila = 0;
	unsigned short int columna = 0;

	for (fila = cant_filas; fila > 0; fila--) {
		for (columna = 0; columna < 5; columna++) {
			if ((fila - 1) != 0 || columna != 2)
				Dibujar(ventanas[fila - 1][columna].x,
						ventanas[fila - 1][columna].y, ventana_sana,
						superficie);
		}
	}
}

void* ControlaMovimientos(void *arg) {

	time_t time_pajaro = 0;
	time_t time_ladrillo = 0;
	time_t time_ralph = 0;
	time_t time_general = 0;
	char sentido_ralph = '0';
	char sentido_pajaro = '0';
	char ctrl_pajaro = '0';
	char ctrl_ladrillo = '0';
	char ctrl_ralph = '0';

	while (salir == 'N') {
		if (ctrl_pajaro == '0') {
			time_pajaro = time(NULL);
			ctrl_pajaro = '1';
		}
		if (ctrl_ladrillo == '0') {
			time_ladrillo = time(NULL);
			ctrl_ladrillo = '1';
		}
		if (ctrl_ralph == '0') {
			time_ralph = time(NULL);
			ctrl_ralph = '1';
		}
		time_general = time(NULL);
		if (difftime(time_general, time_ralph) > 0.8) {
			ctrl_ralph = '0';
			if (sentido_ralph == '0') {
				if ((ralph_posicion.columna + 1) < 5)
					ralph_posicion.columna += 1;
				else {
					sentido_ralph = '1';
				}
			} else {
				if ((ralph_posicion.columna - 1) >= 0)
					ralph_posicion.columna -= 1;
				else {
					sentido_ralph = '0';
				}
			}
			if (cant_rocas < 20) {
				rocas_desplazamiento[roca_siguiente].x =
						ventanas_tramo1[2][ralph_posicion.columna].x + 10;
				((roca_siguiente + 1) == 20) ?
						roca_siguiente = 0 : roca_siguiente++;
				cant_rocas++;
			}
		}
		if (difftime(time_general, time_pajaro) > 0.3) {
			ctrl_pajaro = '0';
			if (sentido_pajaro == '0')
				if ((pajaro_posicion.columna + 1) < 5)
					pajaro_posicion.columna += 1;
				else {
					sentido_pajaro = '1';
				}
			else {
				if ((pajaro_posicion.columna - 1) >= 0)
					pajaro_posicion.columna -= 1;
				else {
					sentido_pajaro = '0';
				}
			}
		}
		usleep(100000);
	}
	return NULL;
}

void* EscuchaServidor(void *arg) {
	int fd = *(int *) arg;
	//int fd = (int) arg;
	CommunicationSocket cSocket(fd);

	while (salir == 'N') {
		cout << "espero msj del servidor" << endl;
		cSocket.ReceiveBloq(buffer,
				sizeof(LONGITUD_CODIGO + LONGITUD_CONTENIDO));
		if (strlen(buffer) > 0) {
			cout << "Recibi: " << buffer << endl;
		}
		sleep(1);
		//usleep(10000);
	}
}

void * EnvioServidor(void * arg) {
	int fd = *(int *) arg;
	CommunicationSocket cSocket(fd);
	while (salir == 'N') {
		if (!cola_grafico.empty()) {
			string mensaje = cola_grafico.front();
			cola_grafico.pop();
			cout << "Mensaje a enviar: " << mensaje.c_str()<< " tamanio: "<<sizeof(mensaje) << endl;
			cSocket.SendBloq(mensaje.c_str(), mensaje.length());
		}
		sleep(1);
	}
}

void* EscuchaTeclas(void *arg) {

	SDL_Event evento;//Con esta variable reconozco si se apreto una tecla o se apreto el mouse.
	SDL_keysym keysym; //Con esta variable reconzco el codigo de la tecla que se apreto.

//Lupeo escuchando el teclado.
	while (SDL_WaitEvent(&evento) != 0) {

		switch (evento.type) {

		case SDL_KEYDOWN:
//				keysym = evento.key.keysym;
			if (evento.key.keysym.sym == SDLK_DOWN) {
				if ((felix_posicion.fila - 1) >= 0)
					if ((felix_posicion.fila - 1) != 0)
						felix_posicion.fila--;
					else if ((felix_posicion.columna) != 2)
						felix_posicion.fila--;

				ostringstream ss1;
				ostringstream ss2;
				ss1 << felix_posicion.fila;
				ss2 << felix_posicion.columna;
				string aux(ss1.str() + ss2.str());
				string message(CD_MOVIMIENTO_FELIX);
				message.append(fillMessage(aux));
				cout<<"MENSAJE: "<<message<<endl;
				cola_grafico.push(message);

			} else if (evento.key.keysym.sym == SDLK_UP) {
				if (felix_posicion.columna == 99) {
					felix_posicion.fila = 0;
					felix_posicion.columna = 0;
				} else if ((felix_posicion.fila + 1) < 3)
					felix_posicion.fila += 1;

				ostringstream ss1;
				ostringstream ss2;
				ss1 << felix_posicion.fila;
				ss2 << felix_posicion.columna;
				string aux(ss1.str() + ss2.str());
				string message(CD_MOVIMIENTO_FELIX);
				message.append(fillMessage(aux));
				cout<<"MENSAJE: "<<message<<endl;
				cola_grafico.push(message);

			} else if (evento.key.keysym.sym == SDLK_RIGHT) {
				felix1 = felix_d1;
				if (felix_posicion.fila == 99) {
					felix_posicion.fila = 0;
					felix_posicion.columna = 0;
				} else if ((felix_posicion.columna + 1) < 5)
					if ((felix_posicion.columna + 1) != 2)
						felix_posicion.columna++;
					else if ((felix_posicion.fila) != 0)
						felix_posicion.columna++;

				ostringstream ss1;
				ostringstream ss2;
				ss1 << felix_posicion.fila;
				ss2 << felix_posicion.columna;
				string aux(ss1.str() + ss2.str());
				string message(CD_MOVIMIENTO_FELIX);
				message.append(fillMessage(aux));
				cout<<"MENSAJE: "<<message<<endl;
				cola_grafico.push(message);

			} else if (evento.key.keysym.sym == SDLK_LEFT) {
				felix1 = felix_i1;
				if ((felix_posicion.columna - 1) >= 0)
					if ((felix_posicion.columna - 1) != 2)
						felix_posicion.columna--;
					else if ((felix_posicion.fila) != 0)
						felix_posicion.columna--;

				ostringstream ss1;
				ostringstream ss2;
				ss1 << felix_posicion.fila;
				ss2 << felix_posicion.columna;
				string aux(ss1.str() + ss2.str());
				string message(CD_MOVIMIENTO_FELIX);
				message.append(fillMessage(aux));
				cout<<"MENSAJE: "<<message<<endl;
				cola_grafico.push(message);

			}

			printf("%d %d\n", felix_posicion.fila, felix_posicion.columna);
			break;
		case SDLK_ESCAPE:
		case SDL_QUIT:
			salir = 'S';
			break;
		}
		usleep(100000);
	}
	return NULL;
}

void handler(int senial) {

	switch (senial) {
	case SIGINT:
		SDL_Quit();
		break;
	}

}

string fillMessage(string message) {
	string content;
	int cantCeros = LONGITUD_CONTENIDO - message.length();
	content.assign(cantCeros, '0');
	return content.append(message);
}
