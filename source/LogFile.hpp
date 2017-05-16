/*
**	LogFile (.hpp/.cpp)
**	handling class for telescope
**
**	piScope project https://github.com/march42/piScope
**	(C) Copyright 2017 by Marc Hefter
**
**	This program is free software; you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation; either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program; if not, write to the Free Software
**	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
**	MA 02110-1301 USA.
*/

/*!	\brief	class MHLogFile
 *
 *	Declaration of class, members and methods.
 *	Adds simple methods for log file handling.
 */

#ifndef _LOGFILE_HPP_
#	define _LOGFILE_HPP_

#	include "../config.h"

#	include <unistd.h>
#	include <cstdio>

namespace piScope
{

	class MHLogFile
	{
	private:	/* private members are accessible only from within the same class or "friends" */
		//	logging
		char NAME[128];	/*!< NAME to use for log file output clarification */
		FILE* LOGFILE;	/*!< the log file handle */
		int LOGLEVEL;	/*!< the current highest level to filter output to log file */
		const char* FILENAME;	/*!< filename given on open */
		long int MAXSIZE;	/*!< maximum file size, size<=0 to disable rotation */

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */
		//	internal methods
		const char* TimeStampUTC(void) const;	/*!< get current time stamp in UTC */
		const char* TimeStamp(void) const;	/*!< get current time stamp in local time */

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		MHLogFile();	/*!< constructor */
		MHLogFile(const char* file, int level, const char* name);	/*!< constructor with filename, output level and clarification name */
		~MHLogFile();	/*!< destructor */

		//	configuration methods
		FILE* SetLogFile(const char* file=NULL);	/*!< set new log file */
		int SetLogLevel(int level);	/*!< set new filter level */
		long int SetMaxSize(long int maxsize);	/*!< set new maximum file size */
		const char* SetLogName(const char* name);	/*!< set new clarification name */
		//	output methods
		void rotateLog(void);	/*!< set new clarification name */
		int printLog(int level, const char * format, ... );	/*!< special printf function for log file */
	};

};

#endif	/* _LOGFILE_HPP_ */
