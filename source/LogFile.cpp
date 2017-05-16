/*
**	Telescope (.hpp/.cpp)
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
		: LOGFILE(NULL), LOGLEVEL(3), FILENAME(NULL), MAXSIZE(-1)
	{
		//	prepare log
		this->SetLogName(NULL);
	}
	MHLogFile::MHLogFile(const char* file, int level, const char* name)
		: LOGFILE(NULL), LOGLEVEL(level), FILENAME(NULL), MAXSIZE(-1)
	{
		//	prepare log
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
		this->FILENAME = (NULL==file ?"logfile.txt" :file);
		this->LOGFILE = fopen(this->FILENAME,"a");
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
	long int MHLogFile::SetMaxSize(long int maxsize)
	{
		this->MAXSIZE = maxsize;
		return(this->MAXSIZE);
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

	void MHLogFile::rotateLog(void)
	{
		if(NULL != this->LOGFILE)
		{
			long int fsize = ftell(this->LOGFILE);
			/*rewind(this->LOGFILE);	doesnt work why ever */
			//	create new filename
			char* moveFN = new char[FILENAME_MAX];
			snprintf(moveFN,FILENAME_MAX, "%s.old", this->FILENAME);
			//	remove old previously rotated file
			remove(moveFN);
			//	close file and rename
			fclose(this->LOGFILE);
			rename(this->FILENAME, moveFN);
			//	reopen file with old name
			this->SetLogFile(this->FILENAME);
			//	release buffer
			delete(moveFN);
			//	mark rotation (level=-1, so almost always written to file)
			this->printLog(-1, "log file rotated after %d bytes\n", fsize);
		}
	}
	int MHLogFile::printLog(int level, const char * format, ... )
	{
		int written = 0;
		if(NULL != this->LOGFILE && 0 < this->MAXSIZE && this->MAXSIZE < ftell(this->LOGFILE))
		{
			this->rotateLog();
		}
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
