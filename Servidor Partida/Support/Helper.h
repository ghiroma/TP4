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

class Helper {
public:
	Helper();
	static string fillMessage(string message);
	virtual ~Helper();
};

#endif /* HELPER_H_ */
