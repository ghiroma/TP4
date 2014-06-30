/*
 * Funciones.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "FuncionesServidorPartida.h"
#include <iostream>
#include "Support/Constantes.h"
#include "Clases/Semaforo.h"
#include <string.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include "Clases/Edificio.h"
#include "Support/Helper.h"
#include "Support/Estructuras.h"
#include <cstdio>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>

using namespace std;

int filaPreviaPersiana = 0;
int columnaPreviaPersiana = 0;

bool stop = false;
bool cliente1_conectado = true;
bool cliente2_conectado = true;
bool cliente1_jugando = true;
bool cliente2_jugando = true;

queue<string> receiver1_queue;
queue<string> receiver2_queue;
queue<string> sender1_queue;
queue<string> sender2_queue;

CommunicationSocket * cSocket1;
CommunicationSocket * cSocket2;

Felix *felix1;
Felix *felix2;

Edificio *edificio;

pthread_mutex_t mutex_receiver1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_receiver2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sender2 = PTHREAD_MUTEX_INITIALIZER;

sem_t * semPartida, *semTorneo;

struct puntajes * puntaje;

bool
TimeDifference (int timeDifference, time_t startingTime)
{
  if ((time (0) - startingTime) > timeDifference)
    {
      return true;
    }
  else
    {
      return false;
    }
}

void*
timer_thread (void* arg)
{

  time_t startingTimeKeepAlive = time (0);
  time_t startingTimeRalph = time (0);
  time_t startingTimePaloma = time (0);
  time_t startingTimePersiana = time (0);
  time_t startingTimeTorta = time (0);

  //TODO Agregar variacion segun el nivel.
  while (stop == false && (cliente1_conectado || cliente2_conectado))
    {
      if (TimeDifference (INTERVALOS_KEEPALIVE, startingTimeKeepAlive) == true)
	{
	  string message (CD_ACK);
	  string content;
	  message.append (Helper::fillMessage (content));
	  if (cliente1_conectado)
	    Helper::encolar (&message, &sender1_queue, &mutex_sender1);
	  if (cliente2_conectado)
	    Helper::encolar (&message, &sender2_queue, &mutex_sender2);
	  startingTimeKeepAlive = time (0);
	}

      if (TimeDifference (INTERVALOS_RALPH, startingTimeRalph) == true)
	{
	  char aux[5];
	  string message (CD_MOVIMIENTO_RALPH);
	  sprintf (aux, "%d", randomRalphMovement ());
	  message.append (Helper::fillMessage (aux));
	  if (cliente1_conectado)
	    Helper::encolar (&message, &sender1_queue, &mutex_sender1);
	  if (cliente2_conectado)
	    Helper::encolar (&message, &sender2_queue, &mutex_sender2);
	  startingTimeRalph = time (0);
	}

      if (TimeDifference (INTERVALOS_PALOMA, startingTimePaloma) == true)
	{
	  string message (CD_PALOMA);
	  char aux[5];
	  sprintf (aux, "%d", randomPaloma (0));
	  message.append (Helper::fillMessage (aux));
	  if (cliente1_conectado)
	    Helper::encolar (&message, &sender1_queue, &mutex_sender1);
	  if (cliente2_conectado)
	    Helper::encolar (&message, &sender2_queue, &mutex_sender2);
	  startingTimePaloma = time (0);
	}

      if (TimeDifference (INTERVALOS_TORTA, startingTimeTorta) == true)
	{
	  //string message(CD_TORTA);
	  //message.append(randomTorta());
	  //Helper::encolar(&message, &sender1_queue, &mutex_sender1);
	  //Helper::encolar(&message, &sender2_queue, &mutex_sender2);
	  startingTimeTorta = time (0);
	}

      if (TimeDifference (INTERVALOS_PERSIANA, startingTimePersiana) == true)
	{
	  //strcpy(message.codigo_mensaje,CD_PERSIANA);
	  //strcpy(message.contenido,randomPersiana);
	  //Helper::encolar(&message, &sender1_queue, &mutex_sender1);
	  //Helper::encolar(&message, &sender2_queue, &mutex_sender2);
	  startingTimePersiana = time (0);
	}

      usleep (POLLING_DEADTIME);
      //sleep(1);
    }

  pthread_exit (0);
}

void*
sender1_thread (void * arguments)
{
  while (stop == false && cliente1_conectado)
    {
      if (!sender1_queue.empty ())
	{
	  //Lo que venga del timer y validator, se replica a ambos jugadores.
	  string message;
	  Helper::desencolar (&message, &sender1_queue, &mutex_sender1);
	  cout << "Mensaje a enviar: " << message.c_str () << endl;
	  if (cliente1_conectado)
	    {
	      cSocket1->SendBloq (message.c_str (), message.length ());
	    }
	}

      usleep (POLLING_DEADTIME);
      //sleep(1);
    }

  pthread_exit (0);
}

void*
sender2_thread (void * arguments)
{
  while (stop == false && cliente2_conectado)
    {
      if (!sender2_queue.empty ())
	{
	  //Lo que venga del timer y validator, se replica a ambos jugadores.
	  string message;
	  Helper::desencolar (&message, &sender2_queue, &mutex_sender2);
	  cout << "Mensaje a enviar: " << message.c_str () << endl;
	  if (cliente2_conectado)
	    {
	      cSocket2->SendBloq (message.c_str (), message.length ());
	    }
	}
      usleep (POLLING_DEADTIME);
      //sleep(1);
    }

  pthread_exit (0);
}

void*
receiver1_thread (void * fd)
{
  char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
  int readDataCode;
  bzero (buffer, sizeof(buffer));

  while (stop == false && cliente1_conectado)
    {
      readDataCode = cSocket1->ReceiveNoBloq (buffer, sizeof(buffer));
      if (readDataCode > 0)
	{
	  string aux (buffer);
	  cout << "Recibi mensaje: " << buffer << endl;
	  Helper::encolar (&aux, &receiver1_queue, &mutex_receiver1);
	}
      else if (readDataCode == 0)
	{
	  cout << "Se desconecto el cliente nro 1" << endl;
	  //TODO decirle al jugador nro2 que el cliente 1 se desconecto.
	  cliente1_conectado = false;
	}
      usleep (POLLING_DEADTIME);
    }

  pthread_exit (0);
}

void*
receiver2_thread (void * fd)
{
  char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];
  int readDataCode;
  bzero (buffer, sizeof(buffer));

  while (stop == false && cliente2_conectado)
    {
      readDataCode = cSocket2->ReceiveNoBloq (buffer, sizeof(buffer));
      if (readDataCode > 0)
	{
	  string mensaje (buffer);
	  Helper::encolar (&mensaje, &receiver2_queue, &mutex_receiver2);
	}
      else if (readDataCode == 0)
	{
	  cout << "Se desconecto el cliente nro 2" << endl;
	  cliente2_conectado = false;
	}

      usleep (POLLING_DEADTIME);
    }

  pthread_exit (0);
}

//TODO el validator tambien se encarga de cerrar los socket una vez que el cliente se desconecto?
void *
validator_thread (void * argument)
{

  while (stop == false && (cliente1_conectado || cliente2_conectado))
    {
      if (!receiver1_queue.empty ())
	{
	  string message = receiver1_queue.front ();
	  receiver1_queue.pop ();
	  string scodigo = message.substr (0, LONGITUD_CODIGO);
	  int codigo = atoi (scodigo.c_str ());
	  //Dependiendo del codigo, voy a ver que valido.
	  //TODO me van mandar el id, asi que yo tengo que crear los Felix de cada
	  //Jugador.
	  switch (codigo)
	    {
	    //Escribe en el sender_queue.
	    case CD_MOVIMIENTO_FELIX_I:
	      int fila;
	      int columna;
	      cout << "Entro a movimiento felix" << endl;
	      fila = atoi (message.substr (5, 1).c_str ());
	      columna = atoi (message.substr (6, 1).c_str ());

	      if (validateMovement (felix1, fila, columna, edificio))
		{
		  string mensaje_movimiento1 = scodigo
		      + Helper::fillMessage ("1" + message.substr (2, 2));
		  string mensaje_movimiento2 = scodigo
		      + Helper::fillMessage ("2" + message.substr (2, 2));
		  Helper::encolar (&mensaje_movimiento1, &sender1_queue,
				   &mutex_sender1);
		  Helper::encolar (&mensaje_movimiento2, &sender2_queue,
				   &mutex_sender2);
		}
	      break;
	    case CD_PERDIDA_VIDA_I:
	      cout << "Perdieron vida" << endl;
	      if (validateLives (felix1))
		{
		  string message1 (CD_PERDIDA_VIDA);
		  string message2 (CD_PERDIDA_VIDA);
		  message1.append (Helper::fillMessage ("1"));
		  message2.append (Helper::fillMessage ("2"));
		  Helper::encolar (&message1, &sender1_queue, &mutex_sender1);
		  Helper::encolar (&message2, &sender2_queue, &mutex_sender2);
		}
	      else
		{
		  string message1 (CD_PERDIO);
		  string message2 (CD_PERDIO);
		  message1.append (Helper::fillMessage ("1"));
		  message2.append (Helper::fillMessage ("2"));
		  Helper::encolar (&message1, &sender1_queue, &mutex_sender1);
		  Helper::encolar (&message2, &sender2_queue, &mutex_sender2);
		  cliente1_jugando = false;
		}
	      break;
	    case CD_VENTANA_ARREGLADA_I:
	      if (validateWindowFix (felix1, edificio))
		{
		  string message1 (CD_VENTANA_ARREGLADA);
		  string message2 (CD_VENTANA_ARREGLADA);
		  message1.append (Helper::fillMessage ("1"));
		  message2.append (Helper::fillMessage ("2"));
		  Helper::encolar (&message1, &sender1_queue, &mutex_sender1);
		  Helper::encolar (&message2, &sender2_queue, &mutex_sender2);
		}
	      break;
	    }

	}
      if (!receiver2_queue.empty ())
	{
	  string message = receiver2_queue.front ();
	  receiver2_queue.pop ();

	  //switch()
	  //{

	  //}
	}

      usleep (POLLING_DEADTIME);
    }

  pthread_exit (0);
}

void*
sharedMemory_thread (void * arguments)
{
  //TODO usar la clase de semaforo.
  struct idsSharedResources * shmIds = (struct idsSharedResources *) arguments;
  cout<<"key antes de utilizarse: "<<shmIds->shmId<<endl;
  int shmId = shmget (shmIds->shmId, sizeof(struct puntajes), PERMISOS_SHM);
  if (shmId < 0)
    {
      cout << "SRV Partida error en shmget" << endl;
      if(errno==ENOENT)
	cout<<"No existe egmento de memoria para dicho key"<<endl;
      if(errno==EACCES)
	cout<<"No se tienen permisos"<<endl;
      if(errno==EINVAL)
	cout<<"No se puede crear porque ya existe."<<endl;

    }
  puntaje = (struct puntajes *) shmat (shmId, (void *) 0, 0);
  if(puntaje==(void *)-1)
    {
      cout<<"Error en shmat"<<endl;
    }
  semPartida = sem_open (shmIds->semNamePartida, O_CREAT);
  semTorneo = sem_open (shmIds->semNameTorneo, O_CREAT);

  //Semaforo semTorneo(shmIds->semNameTorneo);
  //Semaforo semPartida(shmIds->semNamePartida);
  //cout<<"SERV PARTIDA --> Sem_t * semaforo Torneo: "<<semTorneo.getSem_t()<<endl;
  //cout<<"SERV PARTIDA --> Sem_t * semaforo Partida: "<<semPartida.getSem_t()<<endl;
  int reintentos = 0;

  cout << "Direccion de memoria compartida: " << puntaje << endl;

  struct timespec ts;
  ts.tv_sec = 1;

  while (stop == false && (cliente1_conectado || cliente2_conectado))
    {
      cout << "Espererando en el timedwait" << endl;
      if (sem_timedwait (semTorneo, &ts))
	{
	  //if (semTorneo.timedWait(700000) == 0) {

	  reintentos = 0;
	  cout << "Entro por el timedwait" << endl;

	  if ((cliente1_jugando || cliente2_jugando)
	      && (cliente1_conectado || cliente2_conectado))
	    {

	      struct puntajes aux;
	      aux.idJugador1 = felix1->id;
	      aux.idJugador2 = felix2->id;
	      aux.puntajeJugador1 = felix1->puntaje_parcial;
	      aux.puntajeJugador2 = felix2->puntaje_parcial;
	      aux.keepAlivePartida = true;
	      aux.jugando = true;
	      if (puntaje->keepAliveTorneo == false)
		reintentos++;
	      aux.keepAliveTorneo = false;
	      puntaje = &aux;

	      cout << "Escribi en la memoria compartida" << endl;
	    }
	  else //Murieron los dos jugadores.
	    {
	      cout << "Jugadores desconectados dentro del semaforo" << endl;

	      struct puntajes aux;
	      aux.idJugador1 = felix1->id;
	      aux.idJugador2 = felix2->id;
	      aux.puntajeJugador1 = felix1->puntaje_parcial;
	      aux.puntajeJugador2 = felix2->puntaje_parcial;
	      aux.keepAlivePartida = true;
	      aux.jugando = false;
	      puntaje = &aux;
	      stop = true;
	    }
	  //semPartida.V();
	  sem_post (semPartida);
	  cout << "V de semaforo Torneo." << endl;
	}
      else
	{
	  if (errno == ETIMEDOUT)
	    {

	      cout << "timedout semaforo" << endl;
	      cout << "Reintentos: " << reintentos << endl;

	      sleep (2);

	      reintentos++;
	      if (reintentos == 5)
		{
		  cout << "Se cerro el servidor torneo" << endl;
		  shmdt (puntaje);
		  shmctl(shmId, IPC_RMID, 0);
		  sem_close(semPartida);
		  sem_unlink(shmIds->semNamePartida);
		  sem_close(semTorneo);
		  sem_unlink(shmIds->semNameTorneo);

		  /*string message (CD_TORNEO_CAIDO);
		  message.append (Helper::fillMessage ("0"));
		  if (cliente1_conectado)
		    Helper::encolar (&message, &sender1_queue, &mutex_sender1);
		  if (cliente2_conectado)
		    Helper::encolar (&message, &sender2_queue, &mutex_sender2);*/
		  stop = true;

		}
	      //TODO Murio el servidor asi que tengo que cancelar todo y cerrar todo.
	    }
	}
      usleep (POLLING_DEADTIME);
    }

}

int
randomRalphMovement ()
{
  return rand () % (EDIFICIO_COLUMNAS);
}

int
randomPaloma (int nivel)
{
  if (nivel == 0)
    return rand () % (EDIFICIO_FILAS_1);
  else if (nivel == 1)
    return rand () % (EDIFICIO_FILAS_2);
  return 0;
}

char*
randomTorta ()
{
  char location[3];
  char aux[2];

  sprintf (aux, "%d", rand () % (EDIFICIO_FILAS_1));
  strcpy (location, aux);
  sprintf (aux, "%d", rand () % (EDIFICIO_COLUMNAS));
  strcat (location, aux);

  return location;
}

char *
randomPersiana ()
{
  char location[3];
  char aux[2];

  //No hay persiana cerrada. Actualizo el edificio.
  if (filaPreviaPersiana != 0 && columnaPreviaPersiana != 0)
    {
      sprintf (aux, "%d", rand () % (EDIFICIO_FILAS_1));
      strcpy (location, aux);
      filaPreviaPersiana = atoi (aux);
      sprintf (aux, "%d", rand () % (EDIFICIO_COLUMNAS));
      strcat (location, aux);
      columnaPreviaPersiana = atoi (aux);
      return location;
    }
  else //Hay persiana cerrada. Debo abrirla.
    {
      sprintf (aux, "%d", filaPreviaPersiana);
      strcpy (location, aux);
      sprintf (aux, "%d", columnaPreviaPersiana);
      strcat (location, aux);
      filaPreviaPersiana = 0;
      columnaPreviaPersiana = 0;
      return location;
    }

}

bool
validateMovement (Felix * felix, int fila, int columna, Edificio * edificio)
{
  if (((fila < edificio->filas || fila >= 0)
      && (columna < edificio->columnas || columna >= 0))
      && !edificio->ventanas[fila][columna].marquesina
      && edificio->ventanas[fila][columna].felix == NULL
      && !edificio->ventanas[fila][columna].persiana)
    {

      edificio->ventanas[felix->posicion_x][felix->posicion_y].felix = NULL;
      edificio->ventanas[fila][columna].felix = felix;
      felix->posicion_x = fila;
      felix->posicion_y = fila;

      return true;
    }
  return false;
}

bool
validateWindowFix (Felix * felix, Edificio * edificio)
{
  if (edificio->ventanas[felix->posicion_x][felix->posicion_y].ventanaRota > 0
      && !edificio->ventanas[felix->posicion_x][felix->posicion_y].persiana)
    {
      felix->puntaje_parcial++;
      edificio->ventanas[felix->posicion_x][felix->posicion_y].ventanaRota--;
      return true;
    }
  return false;
}

bool
validateLives (Felix * felix)
{
  return --felix->cantidad_vidas == 0;
}

void
SIGINT_Handler (int inum)
{
  stop = true;
}

/**
 * Convertir un numero de tipo int a String
 */
string
intToString (int number)
{
  stringstream ss;
  ss << number;
  return ss.str ();
}
