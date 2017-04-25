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
#include <cassert>

namespace piScope
{

	Location::Location(double lat, double lon, double height, const char* name)
		: Vector3D(VectorType_LATLON, lat, lon, height, 1.0), Name(NULL)
	{
		this->SetName(name);	//	set name, if given
		this->Validate();
	}
	Location::~Location()
	{
		this->SetName(NULL);	//	free name buffer
	}

	Location* Location::Validate(Location* loc)
	{
		Location* source = (NULL==loc ?this :loc);
		while(90.0 < source->X || -90.0 > source->X)
		{
			#if !defined(NDEBUG)
				std::fprintf(stderr, "debug:\t%s=%f\n", "latitude", source->X);
			#endif
			if(90.0 < source->X)
			{
				source->X -= 180;
			}
			else if(-90.0 > source->X)
			{
				source->X += 180;
			}
			source->X *= -1;
			source->Y *= -1;
		}
		while(-180.0 > source->Y)
		{
			#if !defined(NDEBUG)
				std::fprintf(stderr, "debug:\t%s=%f\n", "longitude", source->Y);
			#endif
			source->Y += 360;
		}
		while(180.0 < source->Y)
		{
			#if !defined(NDEBUG)
				std::fprintf(stderr, "debug:\t%s=%f\n", "longitude", source->Y);
			#endif
			source->Y -= 360;
		}
		assert(90 >= source->X && -90 <= source->X);
		assert(180 >= source->Y && -180 <= source->Y);
		return(source);
	}

	Location* Location::Set(double lat, double lon, double height, const char* name)
	{
		this->Type = VectorType_LATLON;
		this->X = lat;
		this->Y = lon;
		this->Z = height;
		this->Length = 1.0L;
		this->SetName(name);
		return(this);
	}
	const char* Location::SetName(const char* name)
	{
		if(NULL != this->Name)
		{
			delete(this->Name);
			this->Name = NULL;
		}
		if(NULL != name)
		{
			this->Name = new char[std::strlen(name) +1];
			*(this->Name) = '\0';
			std::strcpy(this->Name, name);
		}
		return(this->Name);
	}

	const char* Location::ToString(Location* loc) const
	{
		Location* source = (NULL==loc ?(Location*)this :loc);
		static char value[80] = {'\0'};
		size_t pos = 0;
		pos += std::snprintf(&value[pos], sizeof(value) -pos, "[%f%s,%f%s,%fm]"
			, (0>source->X ?-1 :1) * source->X, (0>source->X ?"S" :"N")
			, (0>source->Y ?-1 :1) * source->Y, (0>source->Y ?"W" :"E")
			, source->Z);
		if(NULL != source->Name)
		{
			pos += std::snprintf(&value[pos], sizeof(value) -pos, " %s", source->Name);
		}
		return(&value[0]);
	}

	Vector3D* Location::ToVector(Location* loc) const
	{
		Location* source = (NULL==loc ?(Location*)this :loc);
		return(new Vector3D(VectorType_LATLON,source->X,source->Y,source->Z,1.0));
	}

	double Location::GetLatitude(void) const
	{
		return(this->X);
	}
	double Location::GetLongitude(void) const
	{
		return(this->Y);
	}
	double Location::GetHeight(void) const
	{
		return(this->Z);
	}

};
