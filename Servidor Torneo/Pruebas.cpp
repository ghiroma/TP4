/*
 * Pruebas.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include "Support/Constantes.h"
#include <fstream>
#include <string>
#include <list>
#include <cstdio>

using namespace std;

void
SetConfigKey (char *, fstream *file);

int
main (int argc, char ** argv)
{
  fstream configFile;
  configFile.open ("config.txt", ios_base::in | ios_base::out);
  string line;
  list<string> configList;

  if (configFile.peek () == ifstream::traits_type::eof ())
    {
      SetConfigKey(ARRIBA,&configFile);
      SetConfigKey(IZQUIERDA,&configFile);
      SetConfigKey(ABAJO,&configFile);
      SetConfigKey(DERECHA,&configFile);
      SetConfigKey(REPARAR,&configFile);
      SetConfigKey(SALIR,&configFile);
    }
  else
    {
      while (getline (configFile, line))
	{
	  configList.push_back (line);
	}

    }

  configFile.close ();
}

void
SetConfigKey (char * key, fstream *file)
{
  char tecla;
  string aux;
  aux = key;
  cout << "Ingrese la tecla para " << aux << endl;
  tecla = cin.get ();
  aux += ":";
  aux += tecla;
  (*file).write (aux.c_str(), sizeof(aux.c_str()));
  (*file).flush();
}
