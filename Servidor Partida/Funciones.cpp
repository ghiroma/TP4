/*
 * Funciones.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: ghiroma
 */

#include "Funciones.h"
#include <time.h>

bool TimeDifference(int timeDifference,time_t startingTime )
{
  if((time(0)-startingTime)>timeDifference)
      return true;
  return false;
}
