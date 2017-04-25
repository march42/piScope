/*
**	TimeStamp (.hpp/.cpp)
**	handling class for time stamp
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

#include "TimeStamp.hpp"
#include "MACROS.h"

#include <cstdlib>
//#include <cstring>
#include <cstdio>
//#include <cmath>
//#include <climits>
//#include <exception>
//#include <stdexcept>
#include <cassert>
#include <ctime>
//#include <cstdarg>
#include <unistd.h>

namespace piScope
{

	TimeStamp::TimeStamp(time_t ts)
	{
		this->Set(ts);
	}
	TimeStamp::~TimeStamp()
	{
	}

	time_t TimeStamp::Set(time_t ts)
	{
		if(1 == ts)
		{
			this->UTC = std::time(NULL);
		}
		else
		{
			this->UTC = ts;
		}
		return(this->UTC);
	}

	time_t TimeStamp::Get(void) const
	{
		return(this->UTC);
	}
	double TimeStamp::GetJulianDate(int modified) const
	{
		double JD = (modified ?TIME_UTC2MJD(this->UTC) :TIME_UTC2JD(this->UTC));
		return(JD);
	}

	const char* TimeStamp::ToString(void) const
	{
		//	get time stamp string
		static char cval[30] = { "\0" };
		size_t pos = 0;
		cval[pos] = '\0';
		struct tm * tmval = std::gmtime(&this->UTC);
		assert(NULL != tmval);
		if(0 == (pos += std::strftime(&cval[pos], sizeof(cval) -pos, "%Y%m%dT%H%M%SZ", tmval)))
		{
			std::sprintf(&cval[0], "failed:%ld", this->UTC);
		}
		assert('\0' != cval[0]);
		return(&cval[0]);
	}

};
