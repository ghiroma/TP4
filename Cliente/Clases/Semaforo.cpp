/*
 * Semaforo.cpp
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

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
        cout << "Error creando el semaforo: " << nombre << endl;
    }
    this->nombre = nombre;
    this->pid = getpid();
}

void Semaforo::getName() {
    cout << "Nombre: " << this->nombre << endl;
}

int Semaforo::getValue() {
    int value;
    sem_getvalue(this->sem, &value);
    return value;
}

int Semaforo::P() {
    if (sem_wait(this->sem) == -1) {
        cout << "Error - " << this->nombre << ".P()" << endl;
        return -1;
    }
    return 1;
}

int Semaforo::V() {
    if (sem_post(this->sem) == -1) {
        cout << "Error - " << this->nombre << ".V()" << endl;
        return -1;
    }
    return 1;
}

void Semaforo::close() {
    if (this->pid == getpid()) {
        if (sem_close(this->sem) == -1) {
            cout << "Error en sem_close(" << this->nombre << ")" << endl;
        }
        if (sem_unlink(this->nombre) == -1) {
            cout << "Error en sem_unlink(" << this->nombre << ")" << endl;
        }
    }
}

Semaforo::~Semaforo() {
    this->close();
}
