/*
**	Vector3D (.hpp/.cpp)
**	handling class for 3D vector
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

#include "Vector3D.hpp"
#include "MACROS.h"

#include <cassert>
#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <unistd.h>
#include <cmath>
//#include <climits>

namespace piScope
{

	Vector3D::Vector3D(VectorType_t vecType, double vecX, double vecY, double vecZ, double vecLen)
			: Type(vecType), X(vecX), Y(vecY), Z(vecZ), Length(vecLen)
	{
	}
	Vector3D::~Vector3D()
	{
	}

	Vector3D* Vector3D::Set(VectorType_t vecType, double vecX, double vecY, double vecZ, double vecLen)
	{
		this->Type = vecType;
		this->X = vecX;
		this->Y = vecY;
		this->Z = vecZ;
		this->Length = vecLen;
		return(this);
	}

	Vector3D* Vector3D::Convert2ECEF(Vector3D* convertVector)
	{
		Vector3D* source = (NULL==convertVector ?this :convertVector);
		Vector3D* destination = (NULL==convertVector ?(new Vector3D) :convertVector);
		//	convert to ECEF
		if(VectorType_LATLON == source->Type)
		{
			//	ECEF-r = EMR + height
			//	ECEF-Z = sin(latitude)
			//	ECEF-r' = cos(latitude)
			//	ECEF-X = r' * cos(longitude)
			//	ECEF-Y = r' * sin(longitude)
			double latlonX = std::cos(DEG2RAD(source->X)) * std::cos(DEG2RAD(source->Y));
			double latlonY = std::cos(DEG2RAD(source->X)) * std::sin(DEG2RAD(source->Y));
			double latlonZ = std::sin(DEG2RAD(source->X));
			#if !defined(NDEBUG)
				std::fprintf(stderr, "debug:\tconverting LATLON=[%f,%f,%f] to ECEF=[%f,%f,%f]\n"
					, source->X, source->Y, source->Z
					, latlonX, latlonY, latlonZ);
				std::fprintf(stderr, "debug:\tsin(lat)=%f, cos(lat)=%f, sin(lon)=%f, cos(lon)=%f\n"
					, std::sin(DEG2RAD(source->X)), std::cos(DEG2RAD(source->X))
					, std::sin(DEG2RAD(source->Y)), std::cos(DEG2RAD(source->Y)));
			#endif
			//	prepare destination vector
			destination->Length = LATLON_EARTHMR + source->Z;
			destination->X = latlonX;
			destination->Y = latlonY;
			destination->Z = latlonZ;
			destination->Type = VectorType_ECEF;
		}
		/*	no accurate conversion possible, just needs setting of base reference center/fix
		else if(VectorType_3DONLY >= source->VectorType)
		{
		}
		*/
		/*	no conversion possible without known location
		else if(VectorType_LocalENU == source->Type)
		{
		}
		else if(VectorType_LocalNED == source->Type)
		{
		}
		*/
		/*	conversion impossible without time stamp and center/fix
		else if(VectorType_J2000 == source->Type)
		{
			//	calculate Earth's rotation since J2000 epoch
			//	apply ecliptic rotation
		}
		*/
		assert(VectorType_ECEF == destination->Type);
		return(destination);
	}

	const char* Vector3D::ToString(void) const
	{
		static char buffer[50] = {'\0'};
		const char* type[] = { "3D","ECEF","LATLON","Local ENU","Local NED","J2000" };
		if(0 > this->Type || (char)sizeof(type) <= this->Type)
		{
			std::snprintf(&buffer[0],sizeof(buffer), "INVALID [%f,%f,%f] l=%f"
				, this->X,this->Y,this->Z, this->Length);
		}
		else
		{
			std::snprintf(&buffer[0],sizeof(buffer), "%s [%f,%f,%f] l=%f", type[this->Type]
				, this->X,this->Y,this->Z, this->Length);
		}
		return(&buffer[0]);
	}

};
