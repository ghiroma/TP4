/*
 * ServidorPartida.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "Clases/Semaforo.h"
#include "Clases/CommunicationSocket.h"
#include "Clases/ServerSocket.h"
#include "Support/Constantes.h"
#include <queue>
#include <string>
#include "Funciones.h"
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>

using namespace std;

void* timer_thread(void* argument);
void* receiver1_thread(void * argument);
void* receiver2_thread(void * argument);
void* sender_thread(void * arguments);
void SIGINT_Handler(int inum);

bool stop = false;
queue<string> timer_queue;
queue<string> receive1_queue;
queue<string> receive2_queue;

struct args_struct{
 int fd1;
 int fd2;
};

CommunicationSocket * cSocket1;
CommunicationSocket * cSocket2;

int main(int argc, char * argv[])
{
  int fdJugador1;
  int fdJugador2;

  struct args_struct args;

  pthread_t thread_timer;
  pthread_t thread_receiver1;
  pthread_t thread_receiver2;
  pthread_t thread_sender;

  signal(SIGINT,SIGINT_Handler);

  //TODO Temporalmente hago que el servidor de partida sea un servidor de torneo.
  ServerSocket sSocket(5555,"127.0.0.1");
  cSocket1 = sSocket.Accept();
  //CommunicationSocket *cSocket2 = sSocket.Accept();

  //Fin TODO Temporalmente.


  //Recibo todos los datos del servidortorneo.

  //Creo los 4 thread.
  pthread_create(&thread_timer,NULL,timer_thread,NULL);
  pthread_create(&thread_receiver1,NULL,receiver1_thread,(void *)&cSocket1->ID);
  //pthread_create(&thread_receiver2,NULL,receiver2_thread,NULL);
  args.fd1=cSocket1->ID;
  //args.fd2=cSocket2->ID;
  pthread_create(&thread_sender,NULL,sender_thread,(void *)&args);

  cSocket1->SendBloq("a",sizeof("a"));

  pthread_join(thread_timer,NULL);
  pthread_join(thread_receiver1,NULL);
  //pthread_join(thread_receiver2,NULL);
  pthread_join(thread_sender,NULL);
  
  delete(cSocket1);
  return 0;
  //TODO envio puntaje al padre.
}

void* timer_thread(void* arg)
{
  time_t startingTime = time(0);
  //TODO sacar hardcodeo.
  while(stop==false)
    {
      if(TimeDifference(2,startingTime)==true)
	{
	  //intervalo ralph.
	  string message("Ralph");//Poner constantes de mensaje.
	  timer_queue.push(message);
	  //timer_sendList.push_back(message);
	}

//      if(TimeDifference(3,startingTime)==true)
//	{
//	  //intervalo pajaro.
//	  string message("2");
//	  timer_queue.push(message);
//	  //timer_sendList.push_back(message);
//	}

//      if(TimeDifference(4,startingTime)==true)
//	{
//	  //intervalo torta.
//	  string message("3");
//	  timer_queue.push(message);
//	  //timer_sendList.push_back(message);
//	}
//
//      if(TimeDifference(5,startingTime)==true)
//	{
//	  //intervalo persiana.
//	  string message("4");
//	  timer_queue.push(message);
//	  //timer_sendList.push_back(message);
//	}
      usleep(1000);
    }

  pthread_exit(0);
}

void* receiver1_thread(void * fd)
{
  //CommunicationSocket cSocket(*(int *)fd);
  char buffer[512];

  while(stop==false)
    {
      cSocket1->ReceiveNoBloq(buffer,sizeof(buffer));
      string aux(buffer);
      //TODO Realizar accion.
     	receive1_queue.push(aux);

     	usleep(1000);
    }

    pthread_exit(0);
}

void* receiver2_thread(void * fd)
{
  //CommunicationSocket cSocket(*(int *)fd);
  char buffer[512];

  while(stop==false)
    {
      cSocket2->ReceiveNoBloq(buffer,sizeof(buffer));
      //TODO realizar accion.
      //receive2_queue.push_back();
      usleep(1000);
    }

  pthread_exit(0);
}

void* sender_thread(void * arguments)
{
  struct args_struct *args = (struct args_struct *)arguments;

  //CommunicationSocket cSocket1(args->fd1);
  //CommunicationSocket cSocket2(fd2);

  while(stop==false)
    {
      //if(receive_sendList.size()>0)
      if(receive1_queue.size()>0)
	{
	  string message = receive1_queue.front();
	  receive1_queue.pop();
	  //switch(message.c_str())
	  //{

	  //}
	  cSocket1->SendBloq(message.c_str(),sizeof(512));
	}
	
	if(receive2_queue.size()>0)
	{
		string message= receive2_queue.front();
		receive2_queue.pop();
		
	}

      if(timer_queue.size()>0)
	{
	  string message = timer_queue.front();
	  timer_queue.pop();
	  //switch(message.c_str())
	  //{

	  //}
	}

      usleep(1000);
    }

  pthread_exit(0);
}


void SIGINT_Handler(int inum)
{
  stop=true;
}
