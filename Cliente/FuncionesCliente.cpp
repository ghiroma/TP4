/*
 * FuncionesCliente.cpp
 *
 *  Created on: Jul 20, 2014
 *      Author: ghiroma
 */

#include "FuncionesCliente.h"
#include "Support/Constantes.h"
#include "Support/Helper.h"
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <fstream>

using namespace std;

queue<string> cola_grafico;
queue<string> cola_ralph;
queue<string> cola_pajaro;
queue<string> cola_torta;
queue<string> cola_mensajes_enviar;
queue<string> cola_felix1;
queue<string> cola_felix2;

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

SDL_Surface *superficie,
*backgroundImg, *pared_tramo1n1, *pared_tramo2n1, *pared_tramo3n1, *pared, *ventana_sana, *ventana_rota1, *ventana_rota2, *ventana_rota3, *ventana_rota4, *ventana, *puerta, *felix_d1, *felix_i1, *felix_r11, *felix_r21, *felix_r31, *felix_r41, *felix_r51, *felix_d2, *felix_i2, *felix_r12, *felix_r22,
		*felix_r32, *felix_r42, *felix_r52, *felix1, *felix2, *ralph_1, *ralph_2, *ralph_3, *ralph_4, *ralph_5, *ralph_6, *ralph, *roca1, *roca2, *roca, *pajaro_1, *pajaro_2, *pajaro, *texto, *puntos, *vidas, *torta;

struct ventana ventanas_tramo1[3][5];

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
bool salir = false;
bool ralph_moverse = false;
bool pajaro_moverse = false;
char ralph_sentido;
bool felix1_reparar = false;
bool felix2_reparar = false;
bool ventanas_cargadas = false;
bool torta_aparece = false;
char felix_cartel_puntos[10] = { 0 };
char felix_cartel_vidas[10] = { 0 };
string felix1_nombre;
string felix2_nombre;
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

unsigned int puertoServidorPartida = 0;
unsigned short int puertoTorneo;
string ip;
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

CommunicationSocket * socketTorneo;
CommunicationSocket * socketPartida;



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
					Helper::encolar(&aux_buffer, &cola_felix2, &mutex_cola_felix2);
				}
				break;
			case CD_OPONENTE_DESCONECTADO_I:
				felix2_vidas = 0;
				felix2_puntos = 0;
				break;
			case CD_PERDIDA_VIDA_I:
				if (buffer[4] == '1') {
					Helper::encolar(&aux_buffer, &cola_felix1, &mutex_cola_felix1);
					if (felix1_vidas > 0) {
						felix1_vidas--;
						felix1_reparar = false;
					}
				} else if (buffer[4] == '2') {
					Helper::encolar(&aux_buffer, &cola_felix2, &mutex_cola_felix2);
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
				felix2_reparar = true;
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
					felix2_reparar = false;
				}
				break;
			case CD_ID_JUGADOR_I:
				break;
			case CD_FIN_PARTIDA_I:
				cout << "Recibi mensaje fin de partida" << endl;
				pthread_mutex_lock(&mutex_partidaFinalizada);
				solicitudDeNuevaParitda = true;
				felix2_nombre = "";
				puertoServidorPartida = 0;
				pthread_mutex_unlock(&mutex_partidaFinalizada);
				break;
			case CD_EMPEZAR_PARTIDA_I:
				pthread_mutex_lock(&mutex_start);
				start = true;
				pthread_mutex_unlock(&mutex_start);
				break;
			case CD_CANTIDAD_VIDAS_I:
				pthread_mutex_lock(&mutex_cantVidas);
				cout << "Recibi mi cantidad de vidas" << endl;
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
				Helper::encolar(&message, &cola_grafico, &mutex_cola_grafico);
				break;
			}
		} else if (readData == 0) {
			//cout<<"Se cayo la partida"<<endl;
			//reseteo el nombre del oponente para que espero que se lo envien
			pthread_mutex_lock(&mutex_nombreOponente);
			nombreOponente = "";
			pthread_mutex_unlock(&mutex_nombreOponente);

			//cout << "Termino la partida" << endl;

			delete (socketPartida);
			socketPartida = NULL;

			//TODO si no termino el torneo
			solicitudDeNuevaParitda = true;
		}

		usleep(2000);
	}
	//cout << "sale el thread de escucharServidor" << endl;
	pthread_exit(NULL);
}

void * EnvioServidor(void * arg) {
	int fd = *(int *) arg;
	CommunicationSocket cSocket(fd);
	while (true) {
		if (!cola_grafico.empty()) {
			string mensaje = Helper::desencolar(&cola_grafico, &mutex_cola_grafico);
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
				cout << "Recibo ranking" << endl;
				ranking = aux_buffer.substr(LONGITUD_CODIGO + LONGITUD_CONTENIDO - 2, 2).c_str();
				salir = true;
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
				cout << "Recibi menasje de fin de torneo" << endl;
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
			cout << "Murio servidor torneo" << endl;
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
			if ((evento.key.keysym.sym == SDLK_DOWN || evento.key.keysym.sym == key_abajo) && felix1_reparar != true) {
				if ((felix1_posicion.fila - 1) >= 0 && felix1_reparar != true && felix1_vidas > 0) {
					char aux[3];
					sprintf(aux, "%d", -1);
					string message(CD_MOVIMIENTO_FELIX);
					message.append(fillMessage(aux));
					Helper::encolar(&message, &cola_grafico, &mutex_cola_grafico);
				}
			} else if ((evento.key.keysym.sym == SDLK_UP || evento.key.keysym.sym == key_arriba) && felix1_reparar != true) {
				if ((felix1_posicion.fila + 1) < 3 && felix1_reparar != true && felix1_vidas > 0) {
					char aux[3];
					sprintf(aux, "%d", 1);
					string message(CD_MOVIMIENTO_FELIX);
					message.append(fillMessage(aux));
					Helper::encolar(&message, &cola_grafico, &mutex_cola_grafico);
				}
			} else if ((evento.key.keysym.sym == SDLK_RIGHT || evento.key.keysym.sym == key_derecha) && felix1_reparar != true) {
				felix1 = felix_d1;
				if ((felix1_posicion.columna + 1) < 5 && felix1_reparar != true && felix1_vidas > 0) {
					string message(CD_MOVIMIENTO_FELIX);
					char aux[5];
					sprintf(aux, "%d", 100);
					message.append(fillMessage(aux));
					Helper::encolar(&message, &cola_grafico, &mutex_cola_grafico);
				}
			} else if ((evento.key.keysym.sym == SDLK_LEFT || evento.key.keysym.sym == key_izquierda) && felix1_reparar != true) {
				felix1 = felix_i1;
				if ((felix1_posicion.columna - 1) >= 0 && felix1_reparar != true && felix1_vidas > 0) {
					char aux[5];
					string message(CD_MOVIMIENTO_FELIX);
					sprintf(aux, "%d", -100);
					message.append(fillMessage(aux));
					Helper::encolar(&message, &cola_grafico, &mutex_cola_grafico);
				}
			} else if ((evento.key.keysym.sym == SDLK_SPACE || evento.key.keysym.sym == key_accion) && felix1_vidas > 0 && felix1_reparar != true) {
				string message(CD_VENTANA_ARREGLANDO);
				message.append(fillMessage("0"));
				Helper::encolar(&message, &cola_grafico, &mutex_cola_grafico);
				felix1_reparar = true;
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
	while (salir == false) {
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

bool CambiaTramo() {
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

	return true;
}

bool CambiaNivel() {
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

	return true;
}

bool IngresaNombre() {
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
		} else if (evento.type == SDL_QUIT) {
			exit(0);
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

	//Espero que el servidor de Torneo me envie  el nombre de mi oponente
	cout << "Esperando nombre de oponente" << endl;
	if (esperarNombreOponente()) {
		cout << "Recibi nombre de mi oponente: " << felix2_nombre << endl;
		//recibio nombre del oponente
	} else {
		salir = true;
		return false;
	}

	esperarPuertoPartida();
	cout << "Recibi el puerto de partida: " << puertoServidorPartida << endl;

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

	//Mando mi ID de Jugador
	string messageIDJugador(CD_ID_JUGADOR);
	messageIDJugador.append(fillMessage(mi_id.c_str()));
	Helper::encolar(&messageIDJugador, &cola_grafico, &mutex_cola_grafico);

	cout << "Mande mi id" << mi_id << endl;

	//Espero mi posicion inicial;
	//cout << "Esperando mi posicion inicial" << endl;
	esperarPosicionInicial();

	cout << "Recibi mi posicion inicial" << endl;

	//Espero la cantidad de vidas
	esperarCantVidas();

	cout << "Recibi la cantidad de vidas" << endl;

	//Mando que estoy listo.
	string messageListo(CD_JUGADOR_LISTO);
	messageListo.append(fillMessage("0"));
	Helper::encolar(&messageListo, &cola_grafico, &mutex_cola_grafico);

	cout << "Envie mensaje de listo" << endl;

	//Espero que el Servidor de Partida me mande el mensaje de START
	//cout << "Esperar signal de START" << endl;
	esperarSTART();

	cout << "Recibo mensaje de empezar" << endl;

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
		Helper::desencolar(&cola_grafico, &mutex_cola_grafico);
	}
	while (!cola_ralph.empty()) {
		Helper::desencolar(&cola_ralph, &mutex_cola_ralph);
	}
	while (!cola_pajaro.empty()) {
		Helper::desencolar(&cola_pajaro, &mutex_cola_pajaro);
	}
	while (!cola_torta.empty()) {
		Helper::desencolar(&cola_torta, &mutex_cola_torta);
	}
	while (!cola_mensajes_enviar.empty()) {
		Helper::desencolar(&cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
	}
	while (!cola_felix1.empty()) {
		Helper::desencolar(&cola_felix1, &mutex_cola_felix1);
	}
	while (!cola_felix2.empty()) {
		Helper::desencolar(&cola_felix2, &mutex_cola_felix2);
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
	salir = false;
	ralph_moverse = false;
	pajaro_moverse = false;
	felix1_reparar = false;
	felix2_reparar = false;
	ventanas_cargadas = false;
	torta_aparece = false;
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


