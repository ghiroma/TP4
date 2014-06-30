/*
 * ServidorPartida.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "Clases/Semaforo.h"
#include "Clases/CommunicationSocket.h"
#include "Clases/ServerSocket.h"
#include "Clases/Felix.h"
#include "Clases/Edificio.h"
#include "Support/Constantes.h"
#include "FuncionesServidorPartida.h"
#include "Support/Estructuras.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/shm.h>


using namespace std;

struct args_struct
{
  int fd1;
  int fd2;
};

int
main (int argc, char * argv[])
{

  cout << "Servidor Partida iniciado" << endl;

  int puerto;
  int cantVidas;
  int response = 0;
  int cantClientes = 0;

  struct idsSharedResources ids;
  struct timeval timeout;

  fd_set fds;

  cSocket1 = NULL;
  cSocket2 = NULL;

  felix1 = NULL;
  felix2 = NULL;

  pthread_t thread_timer;
  pthread_t thread_receiver1;
  pthread_t thread_receiver2;
  pthread_t thread_sender1;
  pthread_t thread_sender2;
  pthread_t thread_validator;
  pthread_t thread_sharedMemory;

  signal (SIGINT, SIGINT_Handler);

  srand (time (NULL));

  if (argc == 2)
    {
      puerto = atoi (argv[0]);
      cantVidas = atoi (argv[1]);
      cout << "Numero de puerto " << puerto << endl;
      cout << "Cantidad de vidas " << cantVidas << endl;
      ids.shmId = ftok ("/bin/ls", puerto);
      if (ids.shmId == -1)
	{
	  cout << "Error al generar el shmId el error es: " << endl;
	}
      cout << "SRV PARTIDA CREO SHMID: " << ids.shmId << endl;

      string nombreSemaforoPartida = "/" + intToString (puerto) + "_Partida";
      string nombreSemaforoTorneo = "/" + intToString (puerto) + "_Torneo";
      ids.semNamePartida = nombreSemaforoPartida.c_str ();
      ids.semNameTorneo = nombreSemaforoTorneo.c_str ();

      cout << "SERVIDOR PARTIDA --> sem name partida" << ids.semNamePartida
	  << endl;
      cout << "SERVIDOR PARTIDA --> sem name torneo" << ids.semNameTorneo
	  << endl;

    }
  else
    {
      //TODO Error y cerrar servidor partida porque faltan datos.
      cout << "No recibi cant correcta datos" << endl;
      exit (1);
    }

  cout << "Parametros seteados" << endl;
//TODO Temporalmente hago que el servidor de partida sea un servidor de torneo.
  ServerSocket sSocket (puerto);
  cout << "Puerto servidor creado" << endl;

  timeout.tv_sec = SERVERSOCKET_TIMEOUT;
  timeout.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(sSocket.ID, &fds);

  //TODO Hacer algo si estoy mucho tiempo en el accept y no se conecta nadie.
  do
    {
      if (int response = select (sSocket.ID + 1, &fds, NULL, NULL, &timeout)
	  > 0)
	{
	  if (cSocket1 == NULL)
	    {
	      cSocket1 = sSocket.Accept ();
	      cout << "Conexion recibida 1" << endl;
	    }
	  else
	    {
	      cSocket2 = sSocket.Accept ();
	      cout << "Conexion recibida 2" << endl;
	    }
	  cantClientes++;
	}
      else if (response == 0)
	{
	  //Timeout
	  cout << "El cliente no se ha conectado,no se pudo iniciar la partida."
	      << endl;
	  if (cSocket1 != NULL)
	    {
	      delete (cSocket1);
	    }
	  //TODO Enviar mensaje al cliente de que su oponente no se conecto.
	  exit (1);
	}
    }
  while (cantClientes < 2);

  char buffer[LONGITUD_CODIGO + LONGITUD_CONTENIDO];

  FD_ZERO(&fds);
  FD_SET(cSocket1->ID, &fds);
  FD_SET(cSocket2->ID, &fds);

  while (felix1 == NULL && felix2 == NULL)
    {
      cSocket1->ReceiveNoBloq (buffer, sizeof(buffer));
      string message (buffer);
      cout << "SERVIDOR PARTIDA: id1 "
	  << atoi (
	      message.substr (LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str ())
	  << endl;
      if (message.substr (0, LONGITUD_CODIGO) == CD_ID_JUGADOR)
	felix1 = new Felix (
	    cantClientes,
	    atoi (
		message.substr (LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str ()));
      cSocket2->ReceiveBloq (buffer, sizeof(buffer));
      string message2 (buffer);
      cout << "SERVIDOR PARTIDA: id2 "
	  << atoi (
	      message2.substr (LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str ())
	  << endl;
      if (message2.substr (0, LONGITUD_CODIGO) == CD_ID_JUGADOR)
	felix2 =
	    new Felix (
		cantClientes,
		atoi (
		    message2.substr (LONGITUD_CODIGO, LONGITUD_CONTENIDO).c_str ()));
    }

  edificio = new Edificio (EDIFICIO_FILAS_1, EDIFICIO_COLUMNAS, 0);

//Creo los 4 thread.
  pthread_create (&thread_timer, NULL, timer_thread, NULL);
  pthread_create (&thread_receiver1, NULL, receiver1_thread, NULL);
  pthread_create (&thread_receiver2, NULL, receiver2_thread, NULL);
  pthread_create (&thread_sender1, NULL, sender1_thread, NULL);
  pthread_create (&thread_sender2, NULL, sender2_thread, NULL);
  pthread_create (&thread_validator, NULL, validator_thread, NULL);
  pthread_create (&thread_sharedMemory, NULL, sharedMemory_thread,
		  (void *) &ids);

  pthread_join (thread_timer, NULL);
  pthread_join (thread_receiver1, NULL);
  pthread_join (thread_receiver2, NULL);
  pthread_join (thread_sender1, NULL);
  pthread_join (thread_sender2, NULL);
  pthread_join (thread_validator, NULL);
  pthread_join (thread_sharedMemory, NULL);

//TODO finalizada la partida, enviar los puntajes actualizados.

  pthread_mutex_destroy (&mutex_receiver1);
  pthread_mutex_destroy (&mutex_receiver2);
  pthread_mutex_destroy (&mutex_sender1);
  pthread_mutex_destroy (&mutex_sender2);

  cout << "Se finalizara la partida" << endl;

  delete (cSocket1);
  delete (cSocket2);
  shmdt (puntaje);
  sem_close (semPartida);
  sem_unlink (ids.semNamePartida);
  sem_close (semTorneo);
  sem_unlink (ids.semNameTorneo);
  return 0;
}

