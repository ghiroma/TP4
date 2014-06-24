/*
 * Demonio.cpp
 *
 *  Created on: Jun 23, 2014
 *      Author: ghiroma
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <list>
#include "CommunicationSocket.h"
#include "ServerSocket.h"
#include "Constantes.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

int main(int argc, char * argv[]) {

	int port;
	int pid;

	if(argc!=2)
	{
		exit(EXIT_FAILURE);
	}
	else
	{
		port = atoi(argv[1]);
	}

	if ((pid = fork()) < 0) {
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	setsid();
	umask(0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	if (chdir("/tmp") < 0) {
		exit(EXIT_FAILURE);
	}

	ServerSocket sSocket(port);
	CommunicationSocket * cSocket = sSocket.Accept();

}

