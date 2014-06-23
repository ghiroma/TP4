/*
 * Helper.cpp
 *
 *  Created on: Jun 22, 2014
 *      Author: ghiroma
 */

using namespace std;

#include "Helper.h"
#include "Constantes.h"
#include <string>

Helper::Helper() {
	// TODO Auto-generated constructor stub

}

Helper::~Helper() {
	// TODO Auto-generated destructor stub
}

string Helper::fillMessage(string message) {
	string content;
	int cantCeros = LONGITUD_CONTENIDO - message.length();
	content.assign(cantCeros, '0');
	return content.append(message);
}
