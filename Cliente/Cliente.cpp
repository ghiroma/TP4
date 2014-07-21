#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <iostream>
#include "FuncionesCliente.h"
#include "Support/Constantes.h"
#include "Support/Helper.h"

using namespace std;

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

	mi_id = buffer;
	mi_id = mi_id.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str();
	cout << "Mi id es: " << buffer << endl;

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

	while (salir == false && !murioServidorDelTorneo()) {
		//while (salir == 'N') {
		if (nuevaPartidaSolicitada() && torneoFinalizo()) {
			cout << "Entro por nuevapartidasolicitada && torneofinalizo" << endl;
			break;
		}
		if (nuevaPartidaSolicitada() && !torneoFinalizo() && !murioServidorDelTorneo()) {
			//Solicite nueva partida;
			mostrarPantalla("waitmatch");
			sleep(3);
			if (!inicializarNuevaPartida()) {
				cout << "No pudo inicializar nueva partida" << endl;
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
		if (ventanas_cargadas == false) {
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
			ventanas_cargadas = true;
		}

		DibujarVentanas(ventanas_tramo1, 3, superficie);

		//Dibujo la puerta
		if (tramo == 1)
			Dibujar(ventanas_tramo1[0][1].x + 65, 383, puerta, superficie);

		//Dibujo la torta
		if (torta_aparece == true) {
			Dibujar(ventanas_tramo1[torta_posicion.fila][torta_posicion.columna].x, ventanas_tramo1[torta_posicion.fila][torta_posicion.columna].y, torta, superficie);
		} else {
			if (!cola_torta.empty()) {
				torta_posicion.fila = atoi(cola_torta.front().substr(5, 1).c_str());
				torta_posicion.columna = atoi(cola_torta.front().substr(6, 1).c_str());
				cola_torta.pop();
				torta_aparece = true;
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

		if (ralph_moverse == true) {
			if (ralph_posicion.columna != ralph_destino) {
				if (ralph_sentido == 'D') {
					ralph_posicion.columna += 1;
				} else {
					ralph_posicion.columna -= 1;
				}
			} else {
				ralph_moverse = false;
				if (cant_rocas < 20) {
					rocas_desplazamiento[roca_siguiente].x = ventanas_tramo1[2][ralph_posicion.columna].x + 10;
					((roca_siguiente + 1) == 20) ? roca_siguiente = 0 : roca_siguiente++;
					cant_rocas++;
				}
			}
		} else {
			if (!cola_ralph.empty()) {
				// Me llegan numeradas del 1 al 5. Le resto 1 porque yo las tengo del 0 al 4.
				string ralph_message = Helper::desencolar(&cola_ralph, &mutex_cola_ralph);
				ralph_destino = atoi(ralph_message.substr(6, 1).c_str());

				if (ralph_destino > ralph_posicion.columna)
					ralph_sentido = 'D';
				else
					ralph_sentido = 'I';
				ralph_moverse = true;
			}
		}
		Dibujar(ventanas_tramo1[2][ralph_posicion.columna].x, PARED_Y - 100, ralph, superficie);

		//Dibujo la pajaro
		if (pajaro == pajaro_1)
			pajaro = pajaro_2;
		else
			pajaro = pajaro_1;
		if (pajaro_moverse == true) {
			pajaro_desplazamiento.x += 10;
			Dibujar(pajaro_desplazamiento.x, pajaro_desplazamiento.y, pajaro, superficie);
			if (pajaro_desplazamiento.x > 630) {
				pajaro_moverse = false;
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
				pajaro_moverse = true;
			}
		}
		//Dibujo a Felix
		if (felix1_vidas > 0) {
			if (felix1_reparar == false) {
				if (felix1 == NULL)
					felix1 = felix_d1;
				if (!cola_felix1.empty()) {
					//cout << "Entro a la cola de felix movimiento" << endl;
					felix1_inicial = false;
					string msj = Helper::desencolar(&cola_felix1, &mutex_cola_felix1);
					felix1_posicion.columna = atoi(msj.substr(5, 1).c_str());
					felix1_posicion.fila = atoi(msj.substr(6, 1).c_str());
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
					Helper::encolar(&message, &cola_grafico, &mutex_cola_grafico);
					ventana_reparada(&felix1_posicion);

					felix1_reparar = false;
				}
				Dibujar(ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].x, ventanas_tramo1[felix1_posicion.fila][felix1_posicion.columna].y, felix1, superficie);
			}
		}

		if (felix2 == NULL)
			felix2 = felix_d2;

		//Mueveo a felix2, salvo que este en la posicion inicial
		if (felix2_vidas > 0) {
			if (felix2_reparar == false) {
				if (!cola_felix2.empty()) {
					string msj = Helper::desencolar(&cola_felix2, &mutex_cola_felix2);
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

					felix2_reparar = false;
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

			string message(CD_PERDIDA_VIDA);
			message.append(fillMessage("0"));
			Helper::encolar(&message, &cola_grafico, &mutex_cola_grafico);

		}

		if (ventanas_reparadas == 11) {
			ventanas_reparadas = 10;
			ventanas_cargadas = false;
			if (tramo == 3) {
				PantallaIntermedia('2');
				tramo = 1;
				nivel++;
			} else {
				PantallaIntermedia('1');
				tramo++;
			}
			salir =false;
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

	while (!mostrarRankings()) {
		if (murioServidorDelTorneo()) {
			mostrarPantalla("servertorneodown");
			sleep(6);
			exit(1);
		} else {
			sleep(1);
		}
	}
	mostrarRanking(ranking);

	cout << "Ingrese un tecla para terminar: ";
	getchar();

	exit(0);
}
