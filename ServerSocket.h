/*
 * ServerSocket.h
 *
 *  Created on: May 31, 2014
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
  CommunicationSocket* Accept();
  int SendBloq(char *);
  int ReceiveBloq(char *);
  virtual
  ~ServerSocket ();
};

#endif /* SERVERSOCKET_H_ */
