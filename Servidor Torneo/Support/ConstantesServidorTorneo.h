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
static const char* CD_MOVIMIENTO_RALPH = "00";
static const char* CD_PALOMA = "01";
static const char* CD_TORTA = "02";
static const char* CD_PERSIANA = "03";
static const char* CD_MOVIMIENTO_FELIX = "04";
static const char* CD_PERDIDA_VIDA = "05";
static const char* CD_ID_JUGADOR = "06";
static const char* CD_PUERTO_PARTIDA = "07";
static const char* CD_FIN_TORNEO = "08";
static const char* CD_ACK = "99";


static const int MAX_JUGADORES_SOPORTADOS = 100;

static const int PERMISOS_SHM = 0777;
static const int CLAVE_MEMORIA_COMPARTIDA = 1322;
static const int INTERVALO_ENTRE_BUSQUEDA_DE_OPONENTES = 1000000;

static const int ANCHO_PANTALLA_SERVIDOR = 700;
static const int ALTO_PANTALLA_SERVIDOR = 500;
static const int BPP_SERVIDOR = 16;


#endif /* CONSTANTESSERVIDORTORNEO_H_ */
