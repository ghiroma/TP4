/*
 * Socket.h
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#ifndef SERVERSOCKET_H_
#define SERVERSOCKET_H_

#include "CommunicationSocket.h"

class ServerSocket
{
public:
  int ID;
  ServerSocket (unsigned int port, char * ip);
  ServerSocket (unsigned int port);
  CommunicationSocket* Accept();
  virtual
  ~ServerSocket ();
};

#endif /* SOCKET_H_ */
