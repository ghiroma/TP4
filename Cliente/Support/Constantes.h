/*
 * Constantes.h
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#ifndef CONSTANTES_H_2
#define CONSTANTES_H_2

const int EDIFICIO_COLUMNAS = 5;
const int EDIFICIO_FILAS_1 = 5;
const int EDIFICIO_FILAS_2 = 7;

const int INTERVALOS_KEEPALIVE = 5;
const int INTERVALOS_RALPH = 5;
const int INTERVALOS_PALOMA = 20;
const int INTERVALOS_TORTA = 60;
const int INTERVALOS_PERSIANA = 35;

const int LONGITUD_CODIGO = 2;
const int LONGITUD_CONTENIDO = 5;

//CD viene de codigo mensaje.
static const char* CD_MOVIMIENTO_RALPH = "00";
static const char* CD_PALOMA = "01";
static const char* CD_TORTA = "02";
static const char* CD_PERSIANA = "03";
static const char* CD_MOVIMIENTO_FELIX = "04";
static const char* CD_PERDIDA_VIDA = "05";
static const char* CD_VENTANA_ARREGLADA = "08";
static const char* CD_VENTANA_ARREGLANDO = "12";
static const char* CD_POSICION_INICIAL = "46";
static const char* CD_OPONENTE_DESCONECTADO = "96";
static const char* CD_ACK = "99";

//CODIGOS SERVIDOR TORNEO
static const char* CD_ID_JUGADOR = "06";
static const char* CD_PUERTO_PARTIDA = "07";
static const char* CD_NOMBRE = "50";
static const char* CD_RANKING = "80";
static const char* CD_FIN_TORNEO = "81";

static const int CD_MOVIMIENTO_RALPH_I = 0;
static const int CD_PALOMA_I = 1;
static const int CD_TORTA_I = 2;
static const int CD_PERSIANA_I = 3;
static const int CD_MOVIMIENTO_FELIX_I = 4;
static const int CD_PERDIDA_VIDA_I = 5;
static const int CD_ID_JUGADOR_I = 6;
static const int CD_PUERTO_PARTIDA_I = 7;
static const int CD_VENTANA_ARREGLADA_I = 8;
static const int CD_VENTANA_ARREGLANDO_I = 12;
static const int CD_NOMBRE_I = 50;
static const int CD_RANKING_I = 80;
static const int CD_FIN_TORNEO_I = 81;
static const int CD_POSICION_INICIAL_I = 46;
static const int CD_OPONENTE_DESCONECTADO_I = 96;
static const int CD_ACK_I = 99;

#endif /* CONSTANTES_H_ */
