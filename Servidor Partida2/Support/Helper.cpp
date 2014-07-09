/*
 * Helper.cpp
 *
 *  Created on: Jun 22, 2014
 *      Author: ghiroma
 */

using namespace std;

#include "Helper.h"
#include "Constantes.h"

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

void Helper::encolar(string *message, queue<string> *cola,pthread_mutex_t *mutex)
{

	pthread_mutex_lock(mutex);
	cola->push(*message);
	pthread_mutex_unlock(mutex);
}

void Helper::encolar(Mensaje message, queue<Mensaje> *cola,pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
	cola->push(message);
	pthread_mutex_unlock(mutex);
}

string Helper::desencolar(queue<string> *cola, pthread_mutex_t *mutex)
{
	string message;
	pthread_mutex_lock(mutex);
	message = cola->front();
	cola->pop();
	pthread_mutex_unlock(mutex);
	return message;
}

Mensaje Helper::desencolar(queue<Mensaje> *cola, pthread_mutex_t *mutex)
{
	Mensaje mensaje;
	pthread_mutex_lock(mutex);
	mensaje = cola->front();
	cola->pop();
	pthread_mutex_unlock(mutex);
	return mensaje;
}
