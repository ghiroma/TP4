/*
 * Funciones.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "Funciones.h"
#include <iostream>

using namespace std;

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
/*
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
	  //intervalo ralph.
	  string message = "Ralph"; //Poner constantes de mensaje.
	  timer_queue.push (message);
	  startingTimeRalph = time (0);
	}

      if (TimeDifference (20, startingTimePaloma) == true)
	{
	  //intervalo pajaro.
	  string message ("Pajaro");
	  timer_queue.push (message);
	  startingTimePaloma = time(0);
	}

      if (TimeDifference (35, startingTimePersiana) == true)
	{
	  //intervalo torta.
	  string message ("Torta");
	  timer_queue.push (message);
	  startingTimeTorta = time(0);
	}

      if (TimeDifference (60, startingTimeTorta) == true)
	{
	  //intervalo persiana.
	  string message ("Persiana");
	  timer_queue.push (message);
	  startingTimePersiana = time(0);
	}
      //usleep (1000);
      sleep(1);
    }

  pthread_exit (0);
}

void*
receiver1_thread (void * fd)
{
  //CommunicationSocket cSocket(*(int *)fd);
  char buffer[512];

  while (stop == false)
    {
      cSocket1->ReceiveNoBloq (buffer, sizeof(buffer));
      string aux (buffer);
      //TODO Realizar accion.
      receive1_queue.push (aux);

      //usleep (1000);
      sleep(1);
    }

  pthread_exit (0);
}

void*
receiver2_thread (void * fd)
{
  //CommunicationSocket cSocket(*(int *)fd);
  char buffer[512];

  while (stop == false)
    {
      cSocket2->ReceiveNoBloq (buffer, sizeof(buffer));
      //TODO realizar accion.
      //receive2_queue.push_back();
      //usleep (1000);
      sleep(1);
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
      if (!receive1_queue.empty()) {
       string message = receive1_queue.front();
       //switch(message.c_str())
       //{

       //}
       //cSocket1->SendBloq(message.c_str(), message.lenth());
       receive1_queue.pop();
       }


      if (!receive2_queue.empty()) {
       string message = receive2_queue.front();
       //switch(message.c_str())
       //{

       //}
       //cSocket2->SendBloq(message.c_str(),message.length());
       receive2_queue.pop();
       }

      if (!timer_queue.empty ())
	{
	  string message = timer_queue.front ();
	  timer_queue.pop ();
	  //switch(message.c_str())
	  //{

	  //}
	  cout << "Mensaje a enviar: " << message.c_str() << endl;
	  cSocket1->SendBloq(message.c_str(),message.length());
	  //cSocket2->SendBloq(message.c_str(),message.length());
	}
      sleep (1);
      //usleep(1000);
    }

  pthread_exit (0);
}

*/
