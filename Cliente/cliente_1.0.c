#include <SDL/SDL.h>
//#include <SDL/SDL_image.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <Clases/CommunicationSocket.h>

#define ANCHO_PANTALLA	640
#define ALTO_PANTALLA 480
#define BPP 8

#define PARED_X 90
#define PARED_Y 110

struct ventana{

unsigned short int x;
unsigned short int y;
unsigned short int numero;
char ocupado;

};

void handler(int);
void ConfigRect(SDL_Rect *, int, int, int, int);
void Dibujar(int, int, SDL_Surface *, SDL_Surface *);
void CargarVentanasDelTramo(struct ventana*, unsigned short int, unsigned short int, unsigned short int, char);
void *EscuchaTeclas(void *);
void *EscuchaServidor(void *);
void *DibujarPantalla(void *); //Se usará mas adelante.

SDL_Surface 
	*superficie,
	*pared,
	*ventana,
	*puerta,
	*felix_d,
	*felix_i;

struct ventana ventanas_tramo1[3][5];
unsigned short int felix_ventana = 99;
pthread_t tpid_teclas, tpid_escuchar;
char salir = 'N';
CommunicationSocket socket(5555, "127.0.0.1");
char *buffer = NULL;

int main(int argc, char *argv[]){

	SDL_Rect
		pantalla_juego;

const char 
		pared_bmp[] = "Sprites/pared_grande_tramo1.bmp",
	    ventana_bmp[] = "Sprites/ventana_grande.bmp",
		puerta_bmp[] = "Sprites/puerta_grande.bmp",
		felixd_bmp[] = "Sprites/felix_d.bmp",
		felixi_bmp[] = "Sprites/felix_i.bmp";

		pared = SDL_LoadBMP(pared_bmp);
		ventana = SDL_LoadBMP(ventana_bmp);
		puerta = SDL_LoadBMP(puerta_bmp);
		felix_d = SDL_LoadBMP(felixd_bmp);
		felix_i = SDL_LoadBMP(felixi_bmp);

	socket->ReciveBloq(buffer, sizeof(char));
	pthread_create(&tpid_teclas, NULL, EscuchaServidor, NULL);

	SDL_Init(SDL_INIT_VIDEO);
	signal(SIGINT, handler);
	superficie = SDL_SetVideoMode(ANCHO_PANTALLA, ALTO_PANTALLA, BPP, SDL_HWSURFACE);
	SDL_WM_SetCaption("Rahlp Tournament", NULL);		

//Lanzo el thread que va a estar a la escucha de las teclas que se presionan.
pthread_create(&tpid_teclas, NULL, EscuchaTeclas, NULL);

unsigned short int fila, columna;
unsigned short int ventana_x, ventana_y;

while(salir == 'N'){

	pantalla_juego.x = 0;
	pantalla_juego.y = 0;
	pantalla_juego.w = ANCHO_PANTALLA;
	pantalla_juego.h = ALTO_PANTALLA;

	SDL_FillRect(superficie, &pantalla_juego, SDL_MapRGB(superficie->format, 0, 0, 0));
//Preparo la pantalla.
	Dibujar(PARED_X, PARED_Y, pared, superficie);

// dibujo las ventanas
// Las empiezo a numerar desde arriba.

/* 
Falta hacer una funcion que cargue las ventanas en la matriz antes de 
entrar en este lup
*/
 	ventana_x = PARED_X + 45, ventana_y = PARED_Y + 25;
	for(fila = 0; fila < 2; fila++){
		for(columna = 0; columna<5; columna++){
			Dibujar(ventana_x, ventana_y, ventana, superficie);
			CargarVentanasDelTramo(&ventanas_tramo1[fila][columna], ventana_x, ventana_y, columna + fila*5, 'N');
			ventana_x+=80;	
		}
        ventana_x = PARED_X + 45;
		ventana_y+= 113;
	}

//La primera fila, la que tiene la puerta.
//La trato por separado porque necesito poner la puerta.
	
	ventana_x = PARED_X + 45;
	ventana_y+= 20;

//1
	CargarVentanasDelTramo(&ventanas_tramo1[2][0], ventana_x, ventana_y, 10, 'N');	
	Dibujar(ventana_x, ventana_y, ventana, superficie); ventana_x+=80;
//
	Dibujar(ventana_x + 50, ventana_y, puerta, superficie);

//2
	CargarVentanasDelTramo(&ventanas_tramo1[2][1], ventana_x, ventana_y, 11, 'N');

	Dibujar(ventana_x, ventana_y, ventana, superficie); ventana_x+=160;
//

	CargarVentanasDelTramo(&ventanas_tramo1[2][2], 0, 0, 12, 'N');

//3
	CargarVentanasDelTramo(&ventanas_tramo1[2][3], ventana_x, ventana_y, 13, 'N');
	Dibujar(ventana_x, ventana_y, ventana, superficie); ventana_x+=80;
//

//4
	CargarVentanasDelTramo(&ventanas_tramo1[2][4], ventana_x, ventana_y, 14, 'N');
	Dibujar(ventana_x, ventana_y, ventana, superficie);
//

//Ojo, hay que poner el delay porque sino el proceso no tiene tiempos muertos y 
// el uso del procesador se me va al chori.
	
	if(felix_ventana == 99)
		Dibujar(110, 405, felix_d, superficie);
	else{
		unsigned short int i, j;
		for(i=0; i<3; i++)
			for(j=0; j<5; j++)
				if(ventanas_tramo1[i][j].numero == felix_ventana){
			Dibujar(ventanas_tramo1[i][j].x, ventanas_tramo1[i][j].y, felix_d, superficie);
				}
	}
	SDL_Flip(superficie);
	usleep(250000);
}	

SDL_Quit();
return 0;
}

void CargarVentanasDelTramo(struct ventana *ventana, unsigned short int x, unsigned short int y, unsigned short int nro, char N_S){
			ventana->x = x;
			ventana->y = y;
			ventana->numero = nro;
			ventana->ocupado = N_S;
			
//			printf("X: %d\tY: %d\tNro: %d\tOcupado: %c\n", x, y, nro, N_S);
}

void ConfigRect(SDL_Rect *rect, int x, int y, int w, int h){
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
}

void Dibujar(int x, int y, SDL_Surface *imagen, SDL_Surface *superficie){

SDL_Rect posicion, tamano;
Uint32 colorkey;

	ConfigRect(&posicion, x, y, imagen->w, imagen->h);
	ConfigRect(&tamano, 0, 0, imagen->w, imagen->h);
	colorkey = SDL_MapRGB(imagen->format, 0,0,0);
	SDL_SetColorKey(imagen, SDL_SRCCOLORKEY, colorkey);
	SDL_BlitSurface(imagen, &tamano, superficie, &posicion);
}

void* EscuchaServidor(void *arg){

	while(salir == 'N'){

		socket->ReciveBloq();

		if(felix_ventana == 99)
			felix_ventana = 10;
		else
			if((felix_ventana+1) != 5 && (felix_ventana+1) != 10 && (felix_ventana+1) != 15)
		felix_ventana++;
	}
}

void* EscuchaTeclas(void *arg){

	SDL_Event evento;	//Con esta variable reconozco si se apreto una tecla o se apreto el mouse.
	SDL_keysym keysym; //Con esta variable reconzco el codigo de la tecla que se apreto.

//Lupeo escuchando el teclado.
	while(SDL_WaitEvent(&evento)!=0){
	
		switch(evento.type){

			case SDL_KEYDOWN:
//				keysym = evento.key.keysym;
				if(evento.key.keysym.sym == SDLK_DOWN){
						if((felix_ventana+5) <= 15)
							felix_ventana+=5;
				}
				else if(evento.key.keysym.sym == SDLK_UP){
						if(felix_ventana == 99)
							felix_ventana = 0;
						else
							if((felix_ventana-5) >= 0)
								felix_ventana-=5;
				}
				else if(evento.key.keysym.sym == SDLK_RIGHT){
						if(felix_ventana == 99)
							felix_ventana = 10;
						else
							if((felix_ventana+1) != 5 && (felix_ventana+1) != 10 && (felix_ventana+1) != 15)
felix_ventana++;	
				}
				else if(evento.key.keysym.sym == SDLK_LEFT){
						if((felix_ventana-1) != -1 && (felix_ventana-1) != 4 && (felix_ventana-1) != 9 && (felix_ventana-1) != 98)
felix_ventana--;
				}
			case SDL_QUIT:
				salir = 'S';
				break;
		}
	}
	return NULL;
}

void handler(int senial){

	switch(senial){
	case SIGINT:
		SDL_Quit();
		break;
	}

}

