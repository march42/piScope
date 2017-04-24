/*
**	Location (.hpp/.cpp)
**	handling class for telescope location
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

#include "Location.hpp"

//#include <unistd.h>
#include <cstdio>
//#include <cstdlib>
#include <cstring>
//#include <cmath>
//#include <climits>

namespace piScope
{

	Location::Location(double lat, double lon, int height, const char* name)
		: latitude(lat), longitude(lon), height(height)
	{
		this->name = NULL;
		if(NULL != name)
		{
			this->name = new char[std::strlen(name) +1];
			std::strcpy(this->name, name);
		}
		this->Validate();
	}
	Location::Location(float lat, float lon, float height)
		: latitude(lat), longitude(lon), height(height)
	{
		this->name = NULL;
		this->Validate();
	}
	Location::~Location()
	{
	}

	Location* Location::Validate(Location* loc)
	{
		Location* source = (NULL==loc ?this :loc);
		while(90.0 < source->latitude || -90.0 > source->latitude)
		{
			#if !defined(NDEBUG)
				std::fprintf(stderr, "debug:\t%s=%f\n", "latitude", source->latitude);
			#endif
			if(90.0 < source->latitude)
			{
				source->latitude -= 180;
			}
			else if(-90.0 > source->latitude)
			{
				source->latitude += 180;
			}
			source->latitude *= -1;
			source->longitude *= -1;
		}
		while(-180.0 > source->longitude)
		{
			#if !defined(NDEBUG)
				std::fprintf(stderr, "debug:\t%s=%f\n", "longitude", source->longitude);
			#endif
			source->longitude += 360;
		}
		while(180.0 < source->longitude)
		{
			#if !defined(NDEBUG)
				std::fprintf(stderr, "debug:\t%s=%f\n", "longitude", source->longitude);
			#endif
			source->longitude -= 360;
		}
		return(source);
	}

	const char* Location::ToString(Location* loc) const
	{
		Location* source = (NULL==loc ?(Location*)this :loc);
		static char value[80] = {'\0'};
		size_t pos = 0;
		if(NULL != source->name)
		{
			pos += std::snprintf(&value[pos], sizeof(value) -pos, "%s ", source->name);
		}
		pos += std::snprintf(&value[pos], sizeof(value) -pos, "[%f%s,%f%s,%dm]"
			, (0>source->latitude ?-1 :1) * source->latitude, (0>source->latitude ?"S" :"N")
			, (0>source->longitude ?-1 :1) * source->longitude, (0>source->longitude ?"W" :"E")
			, source->height);
		return(&value[0]);
	}

	Vector3D* Location::ToVector(Location* loc) const
	{
		Location* source = (NULL==loc ?(Location*)this :loc);
		return(new Vector3D(VectorType_LATLON,source->latitude,source->longitude,source->height,0.0));
	}

};
