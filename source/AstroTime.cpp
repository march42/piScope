/*
**	AstroTime (.hpp/.cpp)
**	handling class for astronomical time
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

#include "AstroTime.hpp"
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

	AstroTime::AstroTime(time_t ts, Location* loc)
		//: UTC(ts), TimeLocation(loc)
	{
		//	use methods, so default value handling needs maintenance only once
		this->Set(ts);
		this->SetLocation(loc);
	}
	AstroTime::~AstroTime()
	{
	}

	time_t AstroTime::Set(time_t ts)
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

	Location* AstroTime::SetLocation(Location* loc)
	{
		this->TimeLocation = loc;
		if(NULL == this->TimeLocation)
		{
			this->TimeLocation = new Location(0,0,0,"UNDEFINED");
		}
		return(this->TimeLocation);
	}
	Location* AstroTime::SetLongitude(float lon)
	{
		this->TimeLocation = new Location(0,lon,0,"LON only");
		return(this->TimeLocation);
	}

	time_t AstroTime::Get(int type) const
	{
		//	get time stamp (-1=UTC, 0=LMST, 1=GMST, 2=JD, 3=MJD)
		if(0 == type)
		{
			time_t LMST;
			J2000_UTC2LMST(this->UTC, LMST, this->TimeLocation->GetLongitude());
			return(LMST);
		}
		else if(1 == type)
		{
			time_t GMST;
			J2000_UTC2LMST(this->UTC, GMST, 0);
			return(GMST);
		}
		else if(2 == type)
		{
			double JD = TIME_UTC2JD(this->UTC);
			return(JD);
		}
		else if(3 == type)
		{
			double MJD = TIME_UTC2MJD(this->UTC);
			return(MJD);
		}
		return(this->UTC);
	}

	double AstroTime::GetAngleMST(int GMST) const
	{
		double mst;
		//	get Mean Sidereal Time angle in seconds
		J2000_UTC2LMST(this->UTC, mst, (GMST ?0 :this->TimeLocation->GetLongitude()));
		//	angle = seconds / (86400s / 360°)
		mst /= (ECEF_MEANSOLARDAY / 360.0L);
		return(mst);
	}

	Location* AstroTime::GetLocation(void) const
	{
		return(this->TimeLocation);
	}

	const char* AstroTime::ToString(int type) const
	{
		//	get time stamp (-1=UTC, 0=LMST, 1=GMST, 2=JD, 3=MJD)
		static char cval[30] = { "MIST\0" };
		size_t pos = 0;
		cval[pos] = '\0';
		if(2 == type && 0 == (pos += std::snprintf(&cval[pos], sizeof(cval) -pos, "%f JD", this->GetJulianDate(false))))
		{
			std::sprintf(&cval[0], "snprintf failed.");
		}
		else if(3 == type && 0 == (pos += std::snprintf(&cval[pos], sizeof(cval) -pos, "%f JD", this->GetJulianDate(true))))
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
				std::sprintf(&cval[0], "failed:%ld", tsval);
			}
			else if(1 == type && 0 == (pos += std::strftime(&cval[pos], sizeof(cval) -pos, "%H:%M:%S GMST", tmval)))
			{
				std::sprintf(&cval[0], "failed:%ld", tsval);
			}
			else if(0 == type && 0 == (pos += std::strftime(&cval[pos], sizeof(cval) -pos, "%H:%M:%S LMST", tmval)))
			{
				std::sprintf(&cval[0], "failed:%ld", tsval);
			}
			else if(0 == pos)
			{
				pos += std::strftime(&cval[pos], sizeof(cval) -pos, "%Y%m%dT%H%M%S ????", tmval);
			}
		}
		#if !defined(NDEBUG)
		std::fprintf(stderr, "debug:\tToString: %d = %s\n", type, &cval[0]);
		#endif
		assert('\0' != cval[0]);
		return(&cval[0]);
	}

};
