/*
 * Funciones.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "Funciones.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

void GetConfiguration(int * port, const char * ip, int * cantVidas)
{
  string content;
  string line;
  fstream configFile("configFile",fstream::in | fstream::out);
//  configFile.seekg(0,ios::end);
//  content.resize(configFile.tellg());
//  configFile.seekg(0,ios::beg);
//  configFile.read(&content[0],content.size());
  while(getline(configFile,line))
    {
      if(line.find("Puerto")==0)
	{
	  int pos = line.find(":");
	  string sport = line.substr(pos+1,line.length());
	  *port = atoi(sport.c_str());
	}
      else if(line.compare("IP")==0)
	{
	  int pos = line.find(":");
	  string sip = line.substr(pos+1,line.length());
	  ip = sip.c_str();
	}
      else if(line.compare("Cantidad de Vidas")==0)
	{
	  int pos = line.find(":");
	  string scantVida = line.substr(pos+1,line.length());
	  *cantVidas = atoi(scantVida.c_str());
	}
    }
}

