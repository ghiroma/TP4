/*
 * ServidorPartida.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "Clases/Semaforo.h"
#include "Clases/CommunicationSocket.h"
#include "Support/Constantes.h"
//#include <list>
#inclue <queue>
#include <string>
#include "Funciones.h"
#include <thread>
#include <time.h>

using namespace std;

void timer_thread();
void receiver1_thread(int);
void receiver2_thread(int);
void sender_thread(int, int);

bool stop = false;
queue<string> timer_queue;
queue<string> receive1_queue;
queue<string> receive2_queue;
//list<string> timer_sendList;
//list<string> receive_sendList;

int main(int argc, char * argv[])
{
  int fdJugador1;
  int fdJugador2;

  //Recibo todos los datos del servidortorneo.

  //Creo los 4 thread.
  thread timer(timer_thread);
  thread receiver1(receiver1_thread,fdJugador1);
  thread receiver2(receiver2_thread,fdJugador2);
  thread sender(sender_thread,fdJugador1,fdJugador2);
  
  //TODO envio puntaje al padre.
}

void timer_thread()
{
  time_t startingTime = time(0);
  //TODO sacar hardcodeo.
  while(stop==false)
    {
      if(TimeDifference(2,startingTime)==true)
	{
	  //intervalo ralph.
	  string message("1");//Poner constantes de mensaje.
	  timer_queue.push_back(message);
	  //timer_sendList.push_back(message);
	}

      if(TimeDifference(3,startingTime)==true)
	{
	  //intervalo pajaro.
	  string message("2");
	  timer_queue.push_back(message);
	  //timer_sendList.push_back(message);
	}

      if(TimeDifference(4,startingTime)==true)
	{
	  //intervalo torta.
	  string message("3");
	  timer_queue.push_back(message);
	  //timer_sendList.push_back(message);
	}

      if(TimeDifference(5,startingTime)==true)
	{
	  //intervalo persiana.
	  string message("4");
	  timer_queue.push_back(message);
	  //timer_sendList.push_back(message);
	}
    }
}

void receiver1_thread(int fd)
{
  CommunicationSocket cSocket(fd);
  char buffer[512];

  while(stop==false)
    {
      cSocket.ReceiveBloq(buffer,sizeof(buffer));
      //TODO Realizar accion.
     	receive1_queue.push_back();
    }
}

void receiver2_thread(int fd)
{
  CommunicationSocket cSocket(fd);
  char buffer[512];

  while(stop==false)
    {
      cSocket.ReceiveBloq(buffer,sizeof(buffer));
      //TODO realizar accion.
      receive2_queue.push_back();
    }
}

void sender_thread(int fd1, int fd2)
{
  CommunicationSocket cSocket1(fd1);
  CommunicationSocket cSocket2(fd2);

  while(stop==false)
    {
      //if(receive_sendList.size()>0)
      if(receive1_queue.size()>0)
	{
	  string message = receive1_queue.front();
	  receive1_queue.pop_front();
	  //switch(message.c_str())
	  //{

	  //}
	}
	
	if(receive2_queue.size()>0)
	{
		string message= receive2_queue.front();
		receive2_queue.pop_front();
		
	}

      if(timer_queue.size()>0)
	{
	  string message = receive_queue.front();
	  receive_queue.pop_front();
	  //switch(message.c_str())
	  //{

	  //}
	}
    }
}
