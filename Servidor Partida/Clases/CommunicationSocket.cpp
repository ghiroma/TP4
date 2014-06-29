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
#include <string.h>
#include <netdb.h>

CommunicationSocket::CommunicationSocket (unsigned short int port, char * ip)
{
  // TODO Auto-generated constructor stub
  this->ID = socket(AF_INET,SOCK_STREAM,0);
  if(this->ID>=0) //No hubo error.
    {
	struct sockaddr_in caddress;
	caddress.sin_family = AF_INET;
	caddress.sin_port = htons(port);
	if(inet_addr(ip)==0)//Pruebo por hostname
	{
		struct hostent * ent = gethostbyname(ip);
		memcpy(&caddress.sin_addr.s_addr,ent->h_addr_list[0],ent->h_length);
	}
	else
	{
		caddress.sin_addr.s_addr = inet_addr(ip);
	}

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

CommunicationSocket::CommunicationSocket(int fd)
{
  this->ID = fd;
}

int CommunicationSocket::SendBloq(const char * data,int dataSize)
{
  return send(this->ID,data,dataSize,0);
}

int CommunicationSocket::SendNoBloq(const char * data, int dataSize)
{
  return send(this->ID,data,dataSize,MSG_DONTWAIT);
}

int CommunicationSocket::ReceiveBloq(char * buffer,int bufferSize)
{
  bzero(buffer,sizeof(buffer));
  return recv(this->ID,buffer,bufferSize,0);
}

int CommunicationSocket::ReceiveNoBloq(char * buffer,int bufferSize)
{
  bzero(buffer,sizeof(buffer));
  return recv(this->ID,buffer,bufferSize,MSG_DONTWAIT);
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



