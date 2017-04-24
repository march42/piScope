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

	TimeStamp::TimeStamp(time_t ts, float lon)
	{
		this->Set(ts);
		this->SetLongitude(lon);
	}
	TimeStamp::~TimeStamp()
	{
	}

	time_t TimeStamp::Set(time_t ts)
	{
		if(1 == ts)
		{
			this->UTC_ts = std::time(NULL);
		}
		else
		{
			this->UTC_ts = ts;
		}
		return(this->UTC_ts);
	}
	float TimeStamp::SetLongitude(float lon)
	{
		this->longitude = lon;
		return(this->longitude);
	}

	time_t TimeStamp::Get(int type) const
	{
		//	get time stamp (-1=UTC, 0=LMST, 1=GMST, 2=JD, 3=MJD)
		if(0 == type)
		{
			time_t LMST;
			J2000_UTC2LMST(this->UTC_ts, LMST, this->longitude);
			return(LMST);
		}
		else if(1 == type)
		{
			time_t GMST;
			J2000_UTC2LMST(this->UTC_ts, GMST, 0);
			return(GMST);
		}
		else if(2 == type)
		{
			double JD = TIME_UTC2JD(this->UTC_ts);
			return(JD);
		}
		else if(3 == type)
		{
			double MJD = TIME_UTC2MJD(this->UTC_ts);
			return(MJD);
		}
		return(this->UTC_ts);
	}
	double TimeStamp::GetJulianDate(int modified) const
	{
		double JD = (0==modified ?TIME_UTC2JD(this->UTC_ts) :TIME_UTC2MJD(this->UTC_ts));
		return(JD);
	}

	const char* TimeStamp::ToString(int type) const
	{
		//	get time stamp (-1=UTC, 0=LMST, 1=GMST, 2=JD, 3=MJD)
		static char cval[20] = { "MIST\0" };
		size_t pos = 0;
		if(2 == type && 0 == (pos += std::snprintf(&cval[pos], sizeof(cval) -pos, "%f JD", this->GetJulianDate())))
		{
			std::sprintf(&cval[0], "snprintf failed.");
		}
		else if(3 == type && 0 == (pos += std::snprintf(&cval[pos], sizeof(cval) -pos, "%f JD", this->GetJulianDate(1))))
		{
			std::sprintf(&cval[0], "snprintf failed.");
		}
		else
		{
			time_t tsval = this->Get(type);
			struct tm * tmval = std::gmtime(&tsval);
			assert(NULL!=tmval);
			if(-1 == type && 0 == (pos += std::strftime(&cval[pos], sizeof(cval) -pos, "%Y%m%dT%H%M%SZ", tmval)))
			{
				std::sprintf(&cval[0], "strftime failed. %ld", tsval);
			}
			else if(1 == type && 0 == (pos += std::strftime(&cval[pos], sizeof(cval) -pos, "%H:%M:%S GMST", tmval)))
			{
				std::sprintf(&cval[0], "strftime failed. %ld", tsval);
			}
			else if(0 == type && 0 == (pos += std::strftime(&cval[pos], sizeof(cval) -pos, "%H:%M:%S LMST", tmval)))
			{
				std::sprintf(&cval[0], "strftime failed. %ld", tsval);
			}
			else
			{
				pos += std::strftime(&cval[pos], sizeof(cval) -pos, "%Y%m%dT%H%M%S ????", tmval);
			}
		}
		#if !defined(NDEBUG)
		std::fprintf(stderr, "debug:\tToString: %d = %s\n", type, &cval[0]);
		#endif
		return(&cval[0]);
	}

};
