/*
 * Funciones.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "FuncionesServidorPartida.h"
#include "Support/Constantes.h"
#include "Support/Helper.h"
#include "Support/Estructuras.h"
#include "Clases/Semaforo.h"
#include "Clases/Edificio.h"
#include "Clases/Timer.h"
#include "Clases/Partida.h"
#include "Clases/Mensaje.h"
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <map>

using namespace std;

int cantVidas;
key_t shmId;
pid_t ppid;

bool stop = false;

queue<Mensaje> cola_mensajes_recibir;
queue<Mensaje> cola_mensajes_enviar;

pthread_mutex_t mutex_edificio = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_partidas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_mensajes_recibir = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_mensajes_enviar = PTHREAD_MUTEX_INITIALIZER;

struct puntajes * puntaje;
struct idsSharedResources shmIds;

map<int,
Partida*> partidas;

ServerSocket * sSocket;

Semaforo *semTorneo;
Semaforo *semPartida;

/*
 * Thread encargado de la recepcion de jugadores. Crea las partidas.
 */
void* receptorConexiones(void * args) {
	CommunicationSocket * cSocket;
	Partida * partida;
	Felix * felix;
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	int idPartida = 0;
	string message;

	if (sSocket == 0) {
		cout << "No se ha establecido un socket de servidor" << endl;
		exit(1);
	}

	while (!stop) {
		cout << "Esperando cliente" << endl;
		cSocket = sSocket->Accept();
		//Recibo el id de partida, para sber contra quien va a jugar.
		cSocket->ReceiveBloq(buffer, sizeof(buffer));
		cout << "Recibo menasje de nuevo cliente: " << buffer << endl;
		message = buffer;
		idPartida = atoi(message.substr(LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str());
		felix = new Felix(cantVidas);
		if (partidaPendiente(idPartida)) {
			pthread_mutex_lock(&mutex_partidas);
			map<int, Partida*>::iterator iterator = partidas.find(idPartida);
			iterator->second->cSocket2 = cSocket;
			felix->fila = 0;
			felix->columna = EDIFICIO_COLUMNAS - 1;
			iterator->second->felix2 = felix;
			pthread_mutex_unlock(&mutex_partidas);
			Mensaje mensaje(JUGADOR_2, posicionInicial2(felix), iterator->second);
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			//Mando posicion inicial.
			cout << "Partida nro: " << idPartida << " completada" << endl;
		} else {
			cout << "Creando la partida y primer jugador, id partida: " << idPartida << endl;
			partida = new Partida(idPartida);
			partida->cSocket1 = cSocket;
			felix->fila = 0;
			felix->columna = 0;
			partida->felix1 = felix;
			pthread_mutex_lock(&mutex_partidas);
			partidas[partida->id] = partida;
			pthread_mutex_unlock(&mutex_partidas);
			//Mando posicion inicial
			Mensaje mensaje(JUGADOR_1, posicionInicial1(felix), partida);
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
		}
		usleep(POOLING_DEADTIME);
	}

	pthread_exit(0);
}

/*
 * Thread encargado de la escucha de los mensajes provenientes desde el cliente.
 */
void * escuchaClientes(void * args) {
	char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
	string message;
	int readData = 0;

	while (!stop) {
		pthread_mutex_lock(&mutex_partidas);
		if (!partidas.empty()) {
			for (map<int, Partida *>::iterator it = partidas.begin(); it != partidas.end(); it++) {
				//escucho los clientes.
				if (it->second->cSocket1 != NULL) {
					readData = it->second->cSocket1->ReceiveNoBloq(buffer, sizeof(buffer));
					if (readData > 0) {
						message = buffer;
						Mensaje mensaje(JUGADOR_1, message, it->second);
						cola_mensajes_recibir.push(mensaje);
					}
				} else if (readData == 0) {
					delete (it->second->cSocket1);
					message = CD_OPONENTE_DESCONECTADO;
					message.append(Helper::fillMessage("0"));
					Mensaje mensaje(JUGADOR_2, message, it->second);
					Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
					
				}

				if (it->second->cSocket2 != NULL) {
					readData = it->second->cSocket2->ReceiveNoBloq(buffer, sizeof(buffer));
					if (readData > 0) {
						message = buffer;
						Mensaje mensaje(JUGADOR_2, message, it->second);
						cola_mensajes_recibir.push(mensaje);
					} else if (readData == 0) {
						delete (it->second->cSocket2);
						message = CD_OPONENTE_DESCONECTADO;
						message.append(Helper::fillMessage("0"));
						Mensaje mensaje(JUGADOR_1, message, it->second);
						Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
						
					}
				}
			}
		}
		pthread_mutex_unlock(&mutex_partidas);
		usleep(POOLING_DEADTIME);
	}

	cout<<"escuchar clientes salio del stop"<<endl;
	pthread_exit(0);
}

void* procesarMensajesCliente(void * args) {
	while (!stop) {
		if (!cola_mensajes_recibir.empty()) {
			pthread_mutex_lock(&mutex_cola_mensajes_recibir);
			Mensaje mensaje = cola_mensajes_recibir.front();
			cola_mensajes_recibir.pop();
			int codigo = atoi(mensaje.codigo.c_str());
			switch (codigo) {
			case CD_MOVIMIENTO_FELIX_I:
				caseMovimientoFelix(mensaje);
				break;
			case CD_PERDIDA_VIDA_I:
				casePerdidaVida(mensaje);
				break;
			case CD_VENTANA_ARREGLADA_I:
				caseVentanaArreglada(mensaje);
				break;
			case CD_VENTANA_ARREGLANDO_I:
				caseVentanaArreglando(mensaje);
				break;
			case CD_ID_JUGADOR_I:
				caseIdJugador(mensaje);
				break;
			case CD_JUGADOR_LISTO_I:
				caseJugadorListo(mensaje);
				break;
			}
			pthread_mutex_unlock(&mutex_cola_mensajes_recibir);
		}
		usleep(POOLING_DEADTIME);
	}
	pthread_exit(0);
}

void* timer_thread(void * args) {
	Timer timer;
	string message;

	while (!stop) {
		pthread_mutex_lock(&mutex_partidas);
		for (map<int, Partida*>::iterator it = partidas.begin(); it != partidas.end(); it++) {
			message = timer.keepAlive();
			if (message.length() > 0) {
				Mensaje mensaje(BROADCAST, message, it->second);
				Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			}

			if (it->second->estado == ESTADO_JUGANDO) {

				message = timer.ralph(it->second->edificio->nivel);
				if (message.length() > 0) {
					Mensaje mensaje(BROADCAST, message, it->second);
					Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
				}

				message = timer.paloma(it->second->edificio->nivel);
				if (message.length() > 0) {
					Mensaje mensaje(BROADCAST, message, it->second);
					Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
				}
			}
		}
		pthread_mutex_unlock(&mutex_partidas);
		usleep(POOLING_DEADTIME);
	}
	pthread_exit(0);
}

void * sharedMemory(void * args) {
	try {
		int id = shmget(shmId, sizeof(struct puntajes), PERMISOS_SHM);
		if (id < 0) {
			throw "Error al obtener memoria compartida";
		}
		puntaje = (struct puntajes *) shmat(id, (void *) 0, 0);
		if (puntaje == (void *) -1) {
			throw "Error al mapear la memoria compartida";
		}

		semTorneo = new Semaforo(SEMAFORO_TORNEO);
		semPartida = new Semaforo(SEMAFORO_PARTIDA);
		//TODO Crear semaforos.

		while (stop == false) {

			//Verifico si el padre esta vivo
			if (kill(ppid, 0) == -1) {
				cout << "Servidor partida: Padre esta muerto" << endl;
				//stop = true;
				exit(1);
			}

			pthread_mutex_lock(&mutex_partidas);
			for (map<int, Partida*>::iterator it = partidas.begin(); it != partidas.end(); it++) {
				if (it->second->estado == ESTADO_FINALIZADO && !(it->second->cSocket1 == NULL && it->second->cSocket2 == NULL)) {
					if (sem_trywait(semPartida->getSem_t()) != -1) {
						puntaje->idJugador1 = it->second->felix1->id;
						puntaje->idJugador2 = it->second->felix2->id;
						puntaje->puntajeJugador1 = it->second->felix1->puntaje_parcial;
						puntaje->puntajeJugador2 = it->second->felix2->puntaje_parcial;
						puntaje->partidaFinalizadaOk = true;
						string mensajeFinPartida(CD_FIN_PARTIDA);
						mensajeFinPartida.append(Helper::fillMessage("0"));
						cout << "Envio mensaje de fin de partida" << endl;
						if (it->second->cSocket1 != NULL)
							it->second->cSocket1->SendNoBloq(mensajeFinPartida.c_str(), mensajeFinPartida.length());
						if (it->second->cSocket2 != NULL)
							it->second->cSocket2->SendNoBloq(mensajeFinPartida.c_str(), mensajeFinPartida.length());
						//map<int, Partida*>::iterator auxIt = it;
						//++it;
						//partidas.erase(auxIt);
						partidas.erase(it);
						cout << "Borre la partida" << endl;
						semTorneo->V();
					}
					//End usar semaforos para sincronizar.
					//Borrar partida.
				} else if (it->second->cSocket1 == NULL && it->second->cSocket2 == NULL) {
					//Borrar Partida porque se desconectaron ambos jugadores.
					if (sem_trywait(semPartida->getSem_t()) != -1) {
						puntaje->idJugador1 = it->second->felix1->id;
						puntaje->idJugador2 = it->second->felix2->id;
						puntaje->puntajeJugador1 = 0;
						puntaje->puntajeJugador2 = 0;
						puntaje->partidaFinalizadaOk = false;
						map<int, Partida*>::iterator auxIt = it;
						++it;
						partidas.erase(auxIt);
						semTorneo->V();
					}
				}
			}
			pthread_mutex_unlock(&mutex_partidas);
			usleep(POOLING_DEADTIME);
		}
	} catch (const char * err) {
		cout << "Error inesperado al mapear la memoria compartida." << endl;
		exit(1);
	}
	pthread_exit(0);
}

void* enviarMensajesCliente(void * args) {
	char aux[LONGITUD_CODIGO + LONGITUD_CONTENIDO];

	while (!stop) {
		if (!cola_mensajes_enviar.empty()) {
			//do {
			pthread_mutex_lock(&mutex_cola_mensajes_enviar);
			Mensaje mensaje = cola_mensajes_enviar.front();
			cola_mensajes_enviar.pop();
			pthread_mutex_unlock(&mutex_cola_mensajes_enviar);
			strcpy(aux, mensaje.codigo.c_str());
			strcat(aux, mensaje.mensaje.c_str());
			cout << "Estoy por enviar mensaje: " << mensaje.codigo + mensaje.mensaje << "de tipo: " << mensaje.jugador << endl;
			if (mensaje.jugador == BROADCAST) //BroadCast
					{
				if (mensaje.partida->cSocket1 != NULL) {
					mensaje.partida->cSocket1->SendNoBloq(aux, sizeof(aux));
				}
				if (mensaje.partida->cSocket2 != NULL) {
					mensaje.partida->cSocket2->SendNoBloq(aux, sizeof(aux));
				}
			} else if (mensaje.jugador == JUGADOR_2 && mensaje.partida->cSocket2 != NULL) //Jugador 2
			{
				mensaje.partida->cSocket2->SendNoBloq(aux, sizeof(aux));
			} else if (mensaje.jugador == JUGADOR_1 && mensaje.partida->cSocket1 != NULL) {
				mensaje.partida->cSocket1->SendNoBloq(aux, sizeof(aux));
			}
			//} while (!cola_mensajes_enviar.empty());
		}
		usleep(POOLING_DEADTIME);
	}
	pthread_exit(0);
}

bool partidaPendiente(int idPartida) {
	bool encontrado = false;
	pthread_mutex_lock(&mutex_partidas);
	for (map<int, Partida*>::iterator it = partidas.begin(); it != partidas.end(); it++) {
		if (it->first == idPartida) {
			encontrado = true;
			break;
		}
	}
	pthread_mutex_unlock(&mutex_partidas);
	return encontrado;
}

void caseMovimientoFelix(Mensaje mensaje) {
	int columna = atoi(mensaje.mensaje.substr(3, 1).c_str());
	int fila = atoi(mensaje.mensaje.substr(4, 1).c_str());
	Partida * partidaJugador = mensaje.partida;
	Felix * felixJugador = 0;
	Edificio * edificioJugador = mensaje.partida->edificio;

	if (mensaje.jugador == JUGADOR_1)
		felixJugador = partidaJugador->felix1;
	else if (mensaje.jugador == JUGADOR_2)
		felixJugador = partidaJugador->felix2;

	if (!edificioJugador->ventanas[fila][columna].ocupado && fila >= 0 && columna >= 0 && fila < edificioJugador->filas && columna < edificioJugador->columnas && !(fila == 0 && columna == 2)) {
//Puede moverse ahi.
		edificioJugador->ventanas[felixJugador->fila][felixJugador->columna].ocupado = false;
		edificioJugador->ventanas[fila][columna].ocupado = true;
		felixJugador->fila = fila;
		felixJugador->columna = columna;

//Construyo nuevo mensaje a enviar.
		char auxFila[2];
		char auxColumna[2];
		char aux1[5] = { "1" };
		char aux2[5] = { "2" };

		sprintf(auxFila, "%d", fila);
		sprintf(auxColumna, "%d", columna);

		strcat(aux1, auxColumna);
		strcat(aux1, auxFila);
		strcat(aux2, auxColumna);
		strcat(aux2, auxFila);
//Envio mensaje al jugador1.
		Mensaje mensajeMovimiento = mensaje;
		if (felixJugador == mensaje.partida->felix1) {
			mensajeMovimiento.jugador = JUGADOR_1;
			mensajeMovimiento.mensaje = string(Helper::fillMessage(aux1));
			cout << "mensajeMovimiento.mensaje: " << mensajeMovimiento.mensaje << endl;
			Helper::encolar(mensajeMovimiento, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensajeMovimiento.jugador = JUGADOR_2;
			mensajeMovimiento.mensaje = string(Helper::fillMessage(aux2));
			cout << "mensajeMovimiento.mensaje: " << mensajeMovimiento.mensaje << endl;
			Helper::encolar(mensajeMovimiento, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
		} else if (felixJugador == mensaje.partida->felix2) {
			mensajeMovimiento.jugador = JUGADOR_1;
			mensajeMovimiento.mensaje = string(Helper::fillMessage(aux2));
			Helper::encolar(mensajeMovimiento, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensajeMovimiento.jugador = JUGADOR_2;
			mensajeMovimiento.mensaje = string(Helper::fillMessage(aux1));
			Helper::encolar(mensajeMovimiento, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
		}

	}
}

void caseVentanaArreglada(Mensaje mensaje) {
	Felix * felixJugador;
	if (mensaje.jugador == JUGADOR_1) {
		felixJugador = mensaje.partida->felix1;
	} else if (mensaje.jugador == JUGADOR_2) {
		felixJugador = mensaje.partida->felix2;
	}

	if (felixJugador->reparar(mensaje.partida->edificio)) {
		string mensajeAEnviar(CD_VENTANA_ARREGLADA);

		if (felixJugador == mensaje.partida->felix1) {
			mensaje.jugador = JUGADOR_1;
			mensaje.setMensaje(mensajeAEnviar + string(Helper::fillMessage("1")));
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.jugador = JUGADOR_2;
			mensaje.setMensaje(mensajeAEnviar + string(Helper::fillMessage("2")));
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
		} else if (felixJugador == mensaje.partida->felix2) {
			mensaje.jugador = JUGADOR_1;
			mensaje.setMensaje(mensajeAEnviar + string(Helper::fillMessage("2")));
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.jugador = JUGADOR_2;
			mensaje.setMensaje(mensajeAEnviar + string(Helper::fillMessage("1")));
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
		}
	}

	if (tramoFinalizado(mensaje.partida->edificio)) {
		mensaje.jugador = BROADCAST;
		string message(CD_FIN_PARTIDA);
		message.append(Helper::fillMessage("0"));
		mensaje.setMensaje(message);
		Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
	}

}

void caseVentanaArreglando(Mensaje mensaje) {
	if (mensaje.jugador == JUGADOR_1) {
		mensaje.jugador = JUGADOR_2;
		Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
	} else if (mensaje.jugador == JUGADOR_2) {
		mensaje.jugador = JUGADOR_1;
		Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
	}
}

void casePerdidaVida(Mensaje mensaje) {
	Felix * felixJugador;
	if (mensaje.jugador == JUGADOR_1) {
		felixJugador = mensaje.partida->felix1;
	} else if (mensaje.jugador == JUGADOR_2) {
		felixJugador = mensaje.partida->felix2;
	}

	if (!felixJugador->perderVida()) //Perdio vida
	{
		if (felixJugador == mensaje.partida->felix1) {
			mensaje.mensaje = Helper::fillMessage("100");
			mensaje.jugador = JUGADOR_1;
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.mensaje = Helper::fillMessage("200");
			mensaje.jugador = JUGADOR_2;
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			felixJugador->mover(0, 0, mensaje.partida->edificio);
		} else if (felixJugador == mensaje.partida->felix2) {
			mensaje.mensaje = Helper::fillMessage("240");
			mensaje.jugador = JUGADOR_1;
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.mensaje = Helper::fillMessage("140");
			mensaje.jugador = JUGADOR_2;
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			felixJugador->mover(0, 4, mensaje.partida->edificio);
		}
	} else //Murio
	{
		if (felixJugador == mensaje.partida->felix1) {
			mensaje.setMensaje(string(CD_PERDIO) + Helper::fillMessage("1"));
			mensaje.jugador = JUGADOR_1;
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.setMensaje(string(CD_PERDIO) + Helper::fillMessage("2"));
			mensaje.jugador = JUGADOR_2;
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.partida->edificio->ventanas[felixJugador->fila][felixJugador->columna].ocupado = false;
			mensaje.partida->edificio->ventanas[0][0].ocupado = false;
		} else if (felixJugador == mensaje.partida->felix2) {
			mensaje.setMensaje(string(CD_PERDIO) + Helper::fillMessage("2"));
			mensaje.jugador = JUGADOR_1;
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.setMensaje(string(CD_PERDIO) + Helper::fillMessage("1"));
			mensaje.jugador = JUGADOR_2;
			Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
			mensaje.partida->edificio->ventanas[felixJugador->fila][felixJugador->columna].ocupado = false;
			mensaje.partida->edificio->ventanas[0][4].ocupado = false;
		}
	}
	if ((mensaje.partida->felix1->cantidad_vidas <= 0 && mensaje.partida->felix2->cantidad_vidas <= 0) || (mensaje.partida->felix1->cantidad_vidas <= 0 && mensaje.partida->cSocket2 == NULL) || (mensaje.partida->felix2->cantidad_vidas <= 0 && mensaje.partida->cSocket1 == NULL)) {
		mensaje.partida->estado = ESTADO_FINALIZADO;
	}
}

void caseIdJugador(Mensaje mensaje) {
	pthread_mutex_lock(&mutex_partidas);
	if (mensaje.jugador == JUGADOR_1) {
		mensaje.partida->felix1->id = atoi(mensaje.mensaje.c_str());
	} else if (mensaje.jugador == JUGADOR_2) {
		mensaje.partida->felix2->id = atoi(mensaje.mensaje.c_str());
	}
	pthread_mutex_unlock(&mutex_partidas);
}

void caseJugadorListo(Mensaje mensaje) {
	if (mensaje.jugador == JUGADOR_1) {
		mensaje.partida->jugador1Listo = true;
	} else if (mensaje.jugador == JUGADOR_2) {
		mensaje.partida->jugador2Listo = true;
	}

	if (mensaje.partida->jugador1Listo && mensaje.partida->jugador2Listo) {
		string message(CD_EMPEZAR_PARTIDA);
		message.append(Helper::fillMessage("0"));
		mensaje.jugador = BROADCAST;
		try {
			mensaje.setMensaje(message);
		} catch (const char *err) {
			cout << "Error: " << err << endl;
		}
		Helper::encolar(mensaje, &cola_mensajes_enviar, &mutex_cola_mensajes_enviar);
		mensaje.partida->estado = ESTADO_JUGANDO;
	}
}

void SIGINT_Handler(int inum) {
	cout << "Serv Partida  Signal received PID:"<<getpid()<< endl;
	stop = true;
	exit(1);
}

/*
 * Libera recursos del servidor.
 */
void liberarRecursos() {
	cout<<"libero recursos de Servidor Partida PID:"<<getpid()<<endl;
	if (puntaje != NULL)
		shmdt(puntaje);
	shmctl(shmIds.shmId, IPC_RMID, 0);
	delete (sSocket);
	delete (semPartida);
	delete (semTorneo);
	pthread_mutex_destroy(&mutex_edificio);
	pthread_mutex_destroy(&mutex_cola_mensajes_enviar);
	pthread_mutex_destroy(&mutex_cola_mensajes_recibir);
	pthread_mutex_destroy(&mutex_partidas);
}

string posicionInicial1(Felix * felix) {
	string message(CD_POSICION_INICIAL);
	int fila = 0;
	int columna = 0;
	char cFila[2];
	char cColumna[2];
	char cPos[3];
	sprintf(cFila, "%d", fila);
	sprintf(cColumna, "%d", columna);
	strcpy(cPos, cColumna);
	strcat(cPos, cFila);
	message.append(Helper::fillMessage(cPos));
	felix->fila = fila;
	felix->columna = columna;
	return message;
}

string posicionInicial2(Felix * felix) {
	string message(CD_POSICION_INICIAL);
	int fila = 0;
	int columna = EDIFICIO_COLUMNAS - 1;
	char cFila[2];
	char cColumna[2];
	char cPos[3];
	sprintf(cFila, "%d", fila);
	sprintf(cColumna, "%d", columna);
	strcpy(cPos, cColumna);
	strcat(cPos, cFila);
	message.append(Helper::fillMessage(cPos));
	felix->fila = fila;
	felix->columna = columna;
	return message;
}

bool tramoFinalizado(Edificio * edificio) {
	int result = 0;
	for (int fila = 0; fila < edificio->filas; fila++) {
		for (int columna = 0; columna < edificio->columnas; columna++) {
			if (!(fila == 0 && columna == 2))
				result += edificio->ventanas[fila][columna].ventanaRota;
		}
	}
	return result == 0;
}

