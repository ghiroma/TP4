/*
 * ServidorTorneo.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "Clases/ServerSocket.h"
#include "Clases/CommunicationSocket.h"
#include "Funciones.h"
#include "Jugador.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <list>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

void GetConfiguration(int * port, char * ip,int * cantVidas);
void SIGINT_Handler(int inum);

using namespace std;

int main(int argc, char * argv[])
{
  int *port = 0;
  char * ip = '\0';
  int *cantVidas = 0;
  int clientId=1;
  pid_t pid;
  list<Jugador> listJugadores;

  signal(SIGINT,SIGINT_Handler);

  //GetConfiguration(port,ip,cantVidas);
  if(*port == 0 || *ip=='\0' || *cantVidas==0)
    cout<<"Error al obtener configuracion."<<endl;

  //TODO sacar hardcodeo.
  ServerSocket sSocket(5555,"127.0.0.1");

  while(true)
    {
      CommunicationSocket * cSocket = sSocket.Accept();

      pid=fork();
      if((pid)==0) //Proceso hijo. Hacer exec
	{

	}
      else if(pid<0) //Hubo error
	{
	  cout<<"Error al forkear"<<endl;
	}
      else //Soy el padre.
	{
	  delete(cSocket);
	  string nombreTemp = "juancito";//TODO Sacar el hardcodeo y obtener nombre.
	  Jugador jugador(clientId,nombreTemp);
	  listJugadores.push_back(jugador);
	  clientId++;
	}
    }

}

void SIGINT_Handler(int inum)
{

}

