/*
 * Semaforo.h
 *
 *  Created on: Jun 8, 2014
 *      Author: ghiroma
 */

#ifndef SEMAFORO_H_
#define SEMAFORO_H_

#define PERMISOS 0644

#include <semaphore.h>

class Semaforo
{
private:
  sem_t *sem;
  const char* nombre;
  pid_t pid;
public:
  Semaforo (const char*, int); //Semaforo s1((char*)"/s1", 1);
  void
  getName ();
  int
  getValue ();
  int
  P ();
  int
  V ();
  void
  close ();
  virtual
  ~Semaforo ();

};

#endif /* SEMAFORO_H_ */
