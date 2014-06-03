/*
 * ServerSocket.cpp
 *
 *  Created on: May 31, 2014
 *      Author: ghiroma
 */

#include "ServerSocket.h"
#include "CommunicationSocket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

ServerSocket::ServerSocket (unsigned int port, char * ip)
{
  // TODO Auto-generated constructor stub
  this->ID = socket(AF_INET,SOCK_STREAM,0);
  if(this->ID>=0)
    {
      struct sockaddr_in laddress;
      laddress.sin_addr.s_addr = inet_addr(ip);
      laddress.sin_port = htons(port);
      laddress.sin_family = AF_INET;
      if(bind(this->ID,(struct sockaddr*)&laddress,sizeof(sockaddr))==0)
	{
	  if(listen(this->ID,20)==0)
	    {

	    }
	  else //Error en liten.
	    {

	    }
	}
      else //Error en bind.
	{

	}
    }
  else //Error en socket
    {

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

