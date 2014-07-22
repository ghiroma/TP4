#ifndef CONSTANTESSERVIDORTORNEO_H_
#define CONSTANTESSERVIDORTORNEO_H_

static const int EDIFICIO_COLUMNAS = 5;
static const int EDIFICIO_FILAS_1 = 5;
static const int EDIFICIO_FILAS_2 = 7;

static const int INTERVALOS_KEEPALIVE = 5;
static const int INTERVALOS_RALPH = 5;
static const int INTERVALOS_PALOMA = 20;
static const int INTERVALOS_TORTA = 60;
static const int INTERVALOS_PERSIANA = 35;

static const int LONGITUD_CODIGO = 2;
static const int LONGITUD_CONTENIDO = 5;

//CD viene de codigo mensaje.
/*static const char* CD_MOVIMIENTO_RALPH = "00";
 static const char* CD_PALOMA = "01";
 static const char* CD_TORTA = "02";
 static const char* CD_PERSIANA = "03";
 static const char* CD_MOVIMIENTO_FELIX = "04";
 static const char* CD_PERDIDA_VIDA = "05";
 */

static const char* CD_ID_JUGADOR = "06";
static const char* CD_PUERTO_PARTIDA = "07";
//static const char* CD_FIN_TORNEO = "08";
static const char* CD_NOMBRE = "50";
static const char* CD_ID_PARTIDA = "60";
static const char* CD_RANKING = "80";
static const char* CD_FIN_TORNEO = "81";
static const char* CD_ACK = "99";

static const int CD_ID_JUGADOR_I = 6;
static const int CD_PUERTO_PARTIDA_I = 7;
static const int CD_NOMBRE_I = 50;
static const int CD_ID_PARTIDA_I = 60;
static const int CD_RANKING_I = 80;
static const int CD_FIN_TORNEO_I = 81;
static const int CD_ACK_I = 99;

static const int MAX_JUGADORES_SOPORTADOS = 100;

static const int PERMISOS_SHM = 0666;
static const int CLAVE_MEMORIA_COMPARTIDA = 1322;
static const int INTERVALO_ENTRE_BUSQUEDA_DE_OPONENTES = 8;

static const int MAX_NRO_PUERTOS = 65535;
static const int MIN_PUERTO_REGISTRADO = 1024;
static const int MAX_PUERTO_REGISTRADO = 65000;
static const int MAX_PUERTOS_UTILIZABLES = 10;

//SDL
static const int ANCHO_PANTALLA_SERVIDOR = 700;
static const int ALTO_PANTALLA_SERVIDOR = 500;
static const int BPP_SERVIDOR = 16;
static const int MAX_LENGT_TXT_INFO_JUGADOR = 50;
static const int TIEMPO_DE_REDIBUJADO = 1000000;
static const int TIEMPO_DE_KEEPALIVEJUGADORES = 100000;

#endif /* CONSTANTESSERVIDORTORNEO_H_ */
