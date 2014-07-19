#include <semaphore.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "Semaforo.h"

using namespace std;

Semaforo::Semaforo(const char* nombre, int valor) {

    this->sem = sem_open(nombre, O_CREAT, PERMISOS, valor);
    if (this->sem == (sem_t*) - 1) {
        throw "Error creando el semaforo";
    }
    this->nombre = nombre;
    this->pid = getpid();
}

Semaforo::Semaforo(const char* nombre) {
    this->sem = sem_open(nombre, O_CREAT, PERMISOS);
    if (this->sem == (sem_t*) - 1) {
        throw "Error creando el semaforo";
    }
    this->nombre = nombre;
    this->pid = getpid();
}

int Semaforo::timedWait(long int uSegundos) {
	struct timespec tiempoMaximoDeEspera;
	tiempoMaximoDeEspera.tv_nsec = (long int)uSegundos*100;

	return sem_timedwait(this->sem, &tiempoMaximoDeEspera);
}
const char* Semaforo::getName() {
    return this->nombre;
}

int Semaforo::getValue() {
    int value;
    sem_getvalue(this->sem, &value);
    return value;
}

sem_t * Semaforo::getSem_t(){
	return this->sem;
}

void Semaforo::setSem_t(sem_t* dirSem){
	this->sem = dirSem;
}

int Semaforo::P() {
    if (sem_wait(this->sem) == -1) {
        throw "Error al realizar P de semaforo";
        return -1;
    }
    return 1;
}

int Semaforo::V() {
    if (sem_post(this->sem) == -1) {
        throw "Error al realizar V del semaforo";
        return -1;
    }
    return 1;
}

void Semaforo::close() {
	sem_close(this->sem);       
	//sem_unlink(this->nombre);
}

Semaforo::~Semaforo() {
    this->close();
}
