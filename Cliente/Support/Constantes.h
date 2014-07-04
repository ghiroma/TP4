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

static const int CD_ACK_I = 99;
//CD viene de codigo mensaje.
static const char* CD_MOVIMIENTO_RALPH = "00";
static const char* CD_PALOMA = "01";
static const char* CD_TORTA = "02";
static const char* CD_PERSIANA = "03";
static const char* CD_MOVIMIENTO_FELIX = "04";
static const char* CD_PERDIDA_VIDA = "05";
static const char* CD_NOMBRE = "50";
static const char* CD_ACK = "99";

//CODIGOS SERVIDOR TORNEO
static const char* CD_ID_JUGADOR = "06";
static const char* CD_PUERTO_PARTIDA = "07";
static const char* CD_RANKING = "80";
static const char* CD_FIN_TORNEO = "80";

static const int CD_ID_JUGADOR_I = 6;
static const int CD_PUERTO_PARTIDA_I = 7;
static const int CD_RANKING_I = 80;
static const int CD_FIN_TORNEO_I =81;

#endif /* CONSTANTES_H_ */
