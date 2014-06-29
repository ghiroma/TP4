#ifndef CONSTANTES_H_
#define CONSTANTES_H_

static const int ANCHO_PANTALLA = 640;
static const int ALT0_PANTALLA = 480;
static const int BPP = 8;

static const int PARED_X = 90;
static const int PARED_Y = 110;

static const int EDIFICIO_COLUMNAS = 5;
static const int EDIFICIO_FILAS_1 = 3;
static const int EDIFICIO_FILAS_2 = 4;
static const int EDIFICIO_FILAS_3 = 5;

static const int INTERVALOS_KEEPALIVE = 5;
static const int INTERVALOS_RALPH = 5;
static const int INTERVALOS_PALOMA = 20;
static const int INTERVALOS_TORTA = 60;
static const int INTERVALOS_PERSIANA = 35;

static const int LONGITUD_CODIGO = 2;
static const int LONGITUD_CONTENIDO = 5;

static const int POLLING_DEADTIME = 1000; //Nanosegundos
static const int SEMAPHORE_TIMEOUT = 1; //Segundos
static const int SERVERSOCKET_TIMEOUT = 8; //Segundos
static const int CLIENT_ID_TIMEOUT = 5000; //Milisegundos.
//CD viene de codigo mensaje.
static const char* CD_MOVIMIENTO_RALPH = "00";
static const char* CD_PALOMA = "01";
static const char* CD_TORTA = "02";
static const char* CD_PERSIANA = "03";
static const char* CD_MOVIMIENTO_FELIX = "04";
static const char* CD_PERDIDA_VIDA = "05";
static const char* CD_ID_JUGADOR = "06";
static const char* CD_PUERTO_PARTIDA = "07";
static const char* CD_VENTANA_ARREGLADA = "08";
static const char* CD_ACK = "99";
static const char* CD_PERDIO = "98";
//const char *
//const char *
//const char *
//const char *
//const char *

static const int CD_MOVIMIENTO_FELIX_I = 4;
static const int CD_PERDIDA_VIDA_I = 5;
static const int CD_VENTANA_ARREGLADA_I = 8;

#endif /* CONSTANTES_H_ */

