/*
 * Funciones.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "FuncionesServidorPartida.h"
#include <iostream>
#include "Support/Constantes.h"
#include <string.h>
#include <unistd.h>

using namespace std;

bool stop = false;
queue<string> timer_queue;
queue<string> receive1_queue;
queue<string> receive2_queue;
CommunicationSocket * cSocket1;
CommunicationSocket * cSocket2;
/*
CommunicationSocket * cSocket1;
CommunicationSocket * cSocket2;
*/


bool TimeDifference(int timeDifference,time_t startingTime )
{
  if((time(0)-startingTime)>timeDifference)
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
  time_t startingTimeRalph = time (0);
  time_t startingTimePaloma = time (0);
  time_t startingTimePersiana = time (0);
  time_t startingTimeTorta = time (0);

  //TODO sacar hardcodeo.
  while (stop == false)
    {
      if (TimeDifference (5, startingTimeRalph) == true)
	{
	  string message(CD_MOVIMIENTO_RALPH);
	  //Agregarle hacia donde se va a mover ralph
	  timer_queue.push (message);
	  startingTimeRalph = time (0);
	}

      if (TimeDifference (20, startingTimePaloma) == true)
	{
	  string message(CD_PALOMA);
	  //Agregarle donde aparece
	  timer_queue.push (message);
	  startingTimePaloma = time (0);
	}

      if (TimeDifference (35, startingTimeTorta) == true)
	{
	  string message (CD_TORTA);
	  //Agregarle donde aparece.
	  timer_queue.push (message);
	  startingTimeTorta = time (0);
	}

      if (TimeDifference (60, startingTimePersiana) == true)
	{
	  string message (CD_PERSIANA);
	  //Agregarle donde aparece
	  timer_queue.push (message);
	  startingTimePersiana = time (0);
	}
      sleep (1);
    }

  pthread_exit (0);
}

void*
sender_thread (void * arguments)
{
  char buffer[512] =
    { '\0' };

  while (stop == false)
    {
      if (!receive1_queue.empty ())
	{
	  string message = receive1_queue.front ();
	  cout << "Mensaje recibido del receve1: " << message.c_str () << endl;
	  cSocket1->SendBloq (message.c_str (), message.length ());
	  receive1_queue.pop ();
	}

      if (!receive2_queue.empty ())
	{
	  string message = receive2_queue.front ();
	  //cSocket2->SendBloq(message.c_str(),message.length());
	  receive2_queue.pop ();
	}

      if (!timer_queue.empty ())
	{
	  //Lo que venga del timer, se replica a ambos jugadores.
	  string message = timer_queue.front ();
	  timer_queue.pop ();
	  cout << "Mensaje a enviar: " << message.c_str () << endl;
	  cSocket1->SendBloq (message.c_str (), message.length ());
	  //cSocket2->SendBloq(message.c_str(),message.length());
	}
      sleep (1);
    }

  pthread_exit (0);
}


void*
receiver1_thread (void * fd)
{
  char buffer[512];
  bzero(buffer,sizeof(buffer));

  while (stop == false)
    {
      cSocket1->ReceiveNoBloq (buffer, sizeof(buffer));
      if (strlen (buffer) > 0)
	{
	  string aux (buffer);
	  //TODO Realizar accion.
	  receive1_queue.push (aux);
	}
      sleep (1);
    }

  pthread_exit (0);
}

void*
receiver2_thread (void * fd)
{
  char buffer[512];
  bzero(buffer,sizeof(buffer));

  while (stop == false)
    {
      cSocket2->ReceiveNoBloq (buffer, sizeof(buffer));
     if(strlen(buffer)>0)
	{
	 string aux(buffer);
	  receive2_queue.push(aux);
	}
      //TODO realizar accion.
      //usleep (1000);
      sleep (1);
    }

  pthread_exit (0);
}

void
SIGINT_Handler (int inum)
{
  stop = true;
}
