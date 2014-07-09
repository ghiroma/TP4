/*
 * Timer.h
 *
 *  Created on: Jul 4, 2014
 *      Author: ghiroma
 */

#ifndef TIMER_H_
#define TIMER_H_

using namespace std;

#include <string>


class Timer {
private:
	time_t startingTimeKeepAlive;
	time_t startingTimeRalph;
	time_t startingTimePaloma;
	time_t startingTimePersiana;
	time_t startingTimeTorta;

	int filaPersianaAnterior;
	int columnaPersianaAnterior;

	bool TimeDifference(int timeDifference, time_t startingTime);
	int randomRalphMovement();
	int randomPaloma(int nivel);
	char* randomTorta();
	char * randomPersiana();

public:
	Timer();
	string keepAlive();
	string ralph(int nivel);
	string paloma(int nivel);
	string * torta(int nivel);
	virtual ~Timer();
};

#endif /* TIMER_H_ */
