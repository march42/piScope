/*
**	Telescope (.hpp/.cpp)
**	handling class for telescope
**
**	piScope project https://github.com/march42/piScope
**	(C) Copyright 2017 by Marc Hefter
**
**	This program is free software; you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation; either version 2 of the License, or
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

#include "Telescope.hpp"

#include <cstdlib>
#include <cstring>
//#include <cmath>
//#include <climits>
//#include <exception>
//#include <stdexcept>
//#include <cassert>
#include <ctime>
#include <cstdarg>

#include <unistd.h>

namespace piScope
{

	MHLogFile::MHLogFile()
	{
		//	prepare log
		this->LOGFILE = NULL;
		this->SetLogLevel(3);
		this->SetLogName(NULL);
	}
	MHLogFile::MHLogFile(const char* file, int level, const char* name)
	{
		//	prepare log
		this->LOGFILE = NULL;
		this->SetLogLevel(level);
		this->SetLogName(name);
		this->SetLogFile(file);
	}
	MHLogFile::~MHLogFile()
	{
		if(NULL != this->LOGFILE)
		{
			fclose(this->LOGFILE);
			this->LOGFILE = NULL;
		}
	}

	/*	getting a useful timestamp
	*/
	const char* MHLogFile::TimeStampUTC(void) const
	{
		time_t ts; time(&ts);
		static char value[20];
		strftime(&value[0],sizeof(value), "%04Y%02m%02d.%02H%02M%02S %Z", gmtime(&ts));
		return(&value[0]);
	}
	const char* MHLogFile::TimeStamp(void) const
	{
		struct timespec ts;
		::clock_gettime(CLOCK_MONOTONIC, &ts);
		static char value[20];
		snprintf(&value[0],sizeof(value), "%04d.%06d", (int)ts.tv_sec,(int)(ts.tv_nsec /1000));
		return(&value[0]);
	}

	FILE* MHLogFile::SetLogFile(const char* file)
	{
		this->LOGFILE = fopen((NULL==file ?"logfile.txt" :file),"a");
		return(this->LOGFILE);
	}

	/*	loglevel
	**	-1 = NONE
	**	0 = ERROR
	**	1 = WARNING
	**	2 = INFO
	**	3 = (default)
	*/
	int MHLogFile::SetLogLevel(int level)
	{
		this->LOGLEVEL = level;
		return(this->LOGLEVEL);
	}

	const char* MHLogFile::SetLogName(const char* name)
	{
		if(NULL == name)
		{
			memset(&this->NAME[0], '\0', sizeof(this->NAME));
		}
		else
		{
			strncpy(&this->NAME[0], name, sizeof(this->NAME));
		}
		return(&this->NAME[0]);
	}

	int MHLogFile::printLog(int level, const char * format, ... ) const
	{
		int written = 0;
		if(level <= this->LOGLEVEL)
		{
			va_list args;
			va_start(args, format);
			char message[200] = {0};
			written = std::snprintf(&message[0],sizeof(message), "%s:\t%s\t", this->TimeStamp(), &this->NAME[0]);
			written += vsnprintf(&message[written],sizeof(message)-written, format, args);
			va_end(args);
			std::fprintf(stdout, "%s", message);
			if(NULL != this->LOGFILE)	std::fprintf(this->LOGFILE, "%s", message);
		}
		return(written);
	}

};
