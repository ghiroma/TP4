/*
 * Log.cpp
 *
 *  Created on: Jun 14, 2014
 *      Author: maikel
 */

#include "Log.h"

namespace Log {

Log::Log(string nombre) {
/*
	const char * nombreArchivo = "log-"+nombre+".out";
	this->fpArchivoLog = fopen(nombreArchivo, "a");

	if (this->fpArchivoLog==NULL) {
		cout<<"No se pudo crear el archivo de Logs"<<endl;
		//fputs ("File error",stderr);
		//exit (1);
	}*/
}

void Log::grabar(string msj){
	//fprintf(this->fpArchivoLog, (const char *)msj);
}

Log::~Log() {
	fclose (this->fpArchivoLog);
}

} /* namespace Log */
