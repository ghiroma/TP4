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

const int ANCHO_PANTALLA = 640;
const int ALTO_PANTALLA = 480;
const int BPP = 8;

const int PARED_X = 90;
const int PARED_Y = 110;

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
static const char* CD_ID_PARTIDA = "60";
static const char* CD_JUGADOR_LISTO = "61";
static const char* CD_EMPEZAR_PARTIDA = "62";
static const char* CD_FIN_PARTIDA = "33";

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
static const int CD_PERDIO_I = 98;
static const int CD_ACK_I = 99;
static const int CD_ID_PARTIDA_I = 60;
static const int CD_EMPEZAR_PARTIDA_I=62;
static const int CD_FIN_PARTIDA_I = 33;
static const int CD_SUBIR_TRAMO_I = 34;
static const int CD_CANTIDAD_VIDAS_I = 24;

// Imagenes.

const char pared_tramo1n1_bmp[] = "Sprites/pared_tramo1n1.bmp";
const char pared_tramo2n1_bmp[] = "Sprites/pared_tramo2n1.bmp";
const char pared_tramo3n1_bmp[] = "Sprites/pared_tramo3n1.bmp";
const char ventana_sana_bmp[] = "Sprites/ventana_sana.bmp";
const char ventana_rota1_bmp[] = "Sprites/ventana_rota1.bmp";
const char ventana_rota2_bmp[] = "Sprites/ventana_rota2.bmp";
const char ventana_rota3_bmp[] = "Sprites/ventana_rota3.bmp";
const char ventana_rota4_bmp[] = "Sprites/ventana_rota4.bmp";
const char puerta_bmp[] = "Sprites/puerta_grande.bmp";
const char felixd1_bmp[] = "Sprites/felix_d1.bmp";
const char felixi1_bmp[] = "Sprites/felix_i1.bmp";
const char felixr11_bmp[] = "Sprites/felix_r11.bmp";
const char felixr21_bmp[] = "Sprites/felix_r21.bmp";
const char felixr31_bmp[] = "Sprites/felix_r31.bmp";
const char felixr41_bmp[] = "Sprites/felix_r41.bmp";
const char felixr51_bmp[] = "Sprites/felix_r51.bmp";
const char felixr12_bmp[] = "Sprites/felix_r12.bmp";
const char felixr22_bmp[] = "Sprites/felix_r22.bmp";
const char felixr32_bmp[] = "Sprites/felix_r32.bmp";
const char felixr42_bmp[] = "Sprites/felix_r42.bmp";
const char felixr52_bmp[] = "Sprites/felix_r52.bmp";
const char felixd2_bmp[] = "Sprites/felix_d2.bmp";
const char felixi2_bmp[] = "Sprites/felix_i2.bmp";
const char ralph1_bmp[] = "Sprites/rahlp_1.bmp";
const char ralph2_bmp[] = "Sprites/rahlp_2.bmp";
const char ralph3_bmp[] = "Sprites/rahlp_3.bmp";
const char ralph4_bmp[] = "Sprites/rahlp_4.bmp";
const char ralph5_bmp[] = "Sprites/rahlp_5.bmp";
const char ralph6_bmp[] = "Sprites/rahlp_6.bmp";
const char pajaro1_bmp[] = "Sprites/pajaro_1.bmp";
const char pajaro2_bmp[] = "Sprites/pajaro_2.bmp";
const char roca1_bmp[] = "Sprites/roca1.bmp";
const char roca2_bmp[] = "Sprites/roca2.bmp";
const char torta_bmp[] = "Sprites/torta.bmp";
const char startImage_bmp[] = "Sprites/Mensajes/start.bmp";
const char rankingImage_bmp[] = "Sprites/Mensajes/ranking.bmp";

#endif /* CONSTANTES_H_ */
