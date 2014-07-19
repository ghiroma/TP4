/*
 * Helper.h
 *
 *  Created on: Jun 22, 2014
 *      Author: ghiroma
 */

#ifndef HELPER_H_
#define HELPER_H_

using namespace std;

#include <string>
#include <pthread.h>
#include <queue>

class Helper {
public:
	Helper();
	static bool timeDifference(int timeDifference, time_t startingTime);
	static string fillMessage(string message);
	static void encolar(string *message,queue<string> *cola,pthread_mutex_t *mutex);
	static string desencolar(queue<string> *cola, pthread_mutex_t *mutex);
	virtual ~Helper();
};

#endif /* HELPER_H_ */
