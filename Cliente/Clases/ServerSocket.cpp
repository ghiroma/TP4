/*
 * Socket.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "ServerSocket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_PENDING_CONNECTIONS 20

ServerSocket::ServerSocket (unsigned int port, char * ip)
{
  this->ID = socket(AF_INET,SOCK_STREAM,0);
  if(this->ID>=0)
    {
      struct sockaddr_in laddress;
	laddress.sin_addr.s_addr = inet_addr(ip);
      laddress.sin_port = htons(port);
      laddress.sin_family = AF_INET;
      if(bind(this->ID,(struct sockaddr*)&laddress,sizeof(sockaddr))==0)
	{
	  if((listen(this->ID,MAX_PENDING_CONNECTIONS))!=0)
	    {
	      throw "Error en bind";
	    }
	}
      else //Error en bind.
	{
	  throw "Error en bind";
	}
    }
  else //Error en socket
    {
      throw "Error en socket";
    }
}

ServerSocket::ServerSocket (unsigned int port)
{
  this->ID = socket(AF_INET,SOCK_STREAM,0);
   if(this->ID>=0)
     {
       struct sockaddr_in laddress;
       laddress.sin_addr.s_addr = INADDR_ANY;
       laddress.sin_port = htons(port);
       laddress.sin_family = AF_INET;
       if(bind(this->ID,(struct sockaddr*)&laddress,sizeof(sockaddr))==0)
 	{
 	  if((listen(this->ID,MAX_PENDING_CONNECTIONS))!=0)//Error listen.
 	    {
 	      throw "Error en listen";
 	    }
 	}
       else //Error en bind.
 	{
	   throw "Error en bind";
 	}
     }
   else //Error en socket
     {
       throw "Error en socket";
     }
}

CommunicationSocket* ServerSocket::Accept()
{
  struct sockaddr_in clientData;
  unsigned int clientDataLen = sizeof(clientData);
  int csSocket = accept(this->ID,(struct sockaddr*) &clientData, &clientDataLen);
  return new CommunicationSocket(csSocket,&clientData);
}

ServerSocket::~ServerSocket ()
{
  // TODO Auto-generated destructor stub
  close(this->ID);
}
