/*
 * CommunicationSocket.h
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#ifndef COMMUNICATIONSOCKET_H_
#define COMMUNICATIONSOCKET_H_

class CommunicationSocket
{
private:
  struct sockaddr_in * ClientData;
public:
  int ID;
  //TODO crear otro constructor po hostname.
  CommunicationSocket (unsigned short int, char *);
  CommunicationSocket(int ,struct sockaddr_in *);
  CommunicationSocket(int);
  int SendBloq(const char *,int dataSize);
  int ReceiveBloq(char *,int bufferSize);
  int SendNoBloq(const char *, int dataSize);
  int ReceiveNoBloq(char *, int bufferSize);
  const char * GetClientIP();
  unsigned short int GetClientPort();
  virtual
  ~CommunicationSocket ();
};



#endif /* COMMUNICATIONSOCKET_H_ */
