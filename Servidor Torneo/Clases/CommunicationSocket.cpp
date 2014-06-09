/*
 * CommunicationSocket.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include "CommunicationSocket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <strings.h>

CommunicationSocket::CommunicationSocket (unsigned short int port, char * ip)
{
  // TODO Auto-generated constructor stub
  this->ID = socket(AF_INET,SOCK_STREAM,0);
  if(this->ID>=0) //No hubo error.
    {
	struct sockaddr_in caddress;
	caddress.sin_family = AF_INET;
	caddress.sin_port = htons(port);
	caddress.sin_addr.s_addr = inet_addr(ip);

	if(connect(this->ID,(struct sockaddr*)&caddress,sizeof(sockaddr))!=0) // Error al conectarse
	  {
	    throw "Error en connect";
	  }
    }
  else //Error al crear socket.
    {
      throw "Error al crear socket";
    }
}

CommunicationSocket::CommunicationSocket(int ID,struct sockaddr_in * ClientData)
{
  this->ID = ID;
  this->ClientData = ClientData;
}

int CommunicationSocket::SendBloq(const char * data,int dataSize)
{
  return send(this->ID,data,dataSize,0);
}

int CommunicationSocket::ReceiveBloq(char * buffer,int bufferSize)
{
  bzero(buffer,sizeof(buffer));
  return recv(this->ID,buffer,bufferSize,0);
}

const char * CommunicationSocket::GetClientIP()
{
  return inet_ntoa(this->ClientData->sin_addr);
}

unsigned short int CommunicationSocket::GetClientPort()
{
  return ntohs(this->ClientData->sin_port);
}

CommunicationSocket::~CommunicationSocket ()
{
  close(this->ID);
}



