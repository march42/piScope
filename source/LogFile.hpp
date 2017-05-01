/*
**	LogFile (.hpp/.cpp)
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

#ifndef _LOGFILE_HPP_
#	define _LOGFILE_HPP_

#	include <unistd.h>
#	include <cstdio>

namespace piScope
{

	class MHLogFile
	{
	private:	/* private members are accessible only from within the same class or "friends" */
		//	logging
		char NAME[128];
		FILE* LOGFILE;
		int LOGLEVEL;

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */
		//	internal methods
		const char* TimeStampUTC(void) const;
		const char* TimeStamp(void) const;

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		MHLogFile();
		MHLogFile(const char* file, int level, const char* name);
		~MHLogFile();

		//	configuration methods
		FILE* SetLogFile(const char* file=NULL);
		int SetLogLevel(int level);
		const char* SetLogName(const char* name);
		//	output methods
		int printLog(int level, const char * format, ... ) const;
	};

};

#endif	/* _LOGFILE_HPP_ */
