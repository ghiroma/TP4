/*
 * Log.h
 *
 *  Created on: Jun 14, 2014
 *      Author: maikel
 */

#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <fstream>
using namespace std;

namespace Log {

class Log {
private:
	FILE *fpArchivoLog;
	Log(string nombre);
public:
	void grabar(string msj);
	virtual ~Log();
};

} /* namespace Log */

#endif /* LOG_H_ */
