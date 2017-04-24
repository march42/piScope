/*
**	Vector4D (.hpp/.cpp)
**	handling class for 3D vector with time extension
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

#include "Vector4D.hpp"
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
			: VectorType(vecType), VectorX(vecX), VectorY(vecY), VectorZ(vecZ), VectorLength(vecLen)
	{
	}
	Vector3D::~Vector3D()
	{
	}

	Vector3D* Vector3D::Convert2ECEF(Vector3D* convertVector)
	{
		Vector3D* source = (NULL==convertVector ?this :convertVector);
		Vector3D* destination = (NULL==convertVector ?(new Vector3D) :convertVector);
		//	convert to ECEF
		if(VectorType_LATLON == source->VectorType)
		{
			//	ECEF-r = EMR + height
			//	ECEF-Z = sin(latitude)
			//	ECEF-r' = cos(latitude)
			//	ECEF-X = r' * cos(longitude)
			//	ECEF-Y = r' * sin(longitude)
			double latlonX = std::cos(DEG2RAD(source->VectorX)) * std::cos(DEG2RAD(source->VectorY));
			double latlonY = std::cos(DEG2RAD(source->VectorX)) * std::sin(DEG2RAD(source->VectorY));
			double latlonZ = std::sin(DEG2RAD(source->VectorX));
			#if !defined(NDEBUG)
				std::fprintf(stderr, "debug:\tconverting LATLON=[%f,%f,%f] to ECEF=[%f,%f,%f]\n"
					, source->VectorX, source->VectorY, source->VectorZ
					, latlonX, latlonY, latlonZ);
				std::fprintf(stderr, "debug:\tsin(lat)=%f, cos(lat)=%f, sin(lon)=%f, cos(lon)=%f\n"
					, std::sin(DEG2RAD(source->VectorX)), std::cos(DEG2RAD(source->VectorX))
					, std::sin(DEG2RAD(source->VectorY)), std::cos(DEG2RAD(source->VectorY)));
			#endif
			//	prepare destination vector
			destination->VectorLength = LATLON_EARTHMR + source->VectorZ;
			destination->VectorX = latlonX;
			destination->VectorY = latlonY;
			destination->VectorZ = latlonZ;
			destination->VectorType = VectorType_ECEF;
		}
		/*	no accurate conversion possible, just needs setting of base reference center/fix
		else if(VectorType_3DONLY >= source->VectorType)
		{
		}
		*/
		/*	no conversion possible without known location
		else if(VectorType_LocalENU == source->VectorType)
		{
		}
		else if(VectorType_LocalNED == source->VectorType)
		{
		}
		*/
		/*	conversion impossible without time stamp and center/fix
		else if(VectorType_J2000 == source->VectorType)
		{
			//	calculate Earth's rotation since J2000 epoch
			//	apply ecliptic rotation
		}
		*/
		assert(VectorType_ECEF == destination->VectorType);
		return(destination);
	}

	const char* Vector3D::ToString(void) const
	{
		static char buffer[50] = {'\0'};
		const char* type[] = { "3D","ECEF","LATLON","Local ENU","Local NED","J2000" };
		if(0 > this->VectorType || (char)sizeof(type) <= this->VectorType)
		{
			std::snprintf(&buffer[0],sizeof(buffer), "INVALID [%f,%f,%f] l=%f"
				, this->VectorX,this->VectorY,this->VectorZ, this->VectorLength);
		}
		else
		{
			std::snprintf(&buffer[0],sizeof(buffer), "%s [%f,%f,%f] l=%f", type[this->VectorType]
				, this->VectorX,this->VectorY,this->VectorZ, this->VectorLength);
		}
		return(&buffer[0]);
	}

	Vector4D::Vector4D(VectorType_t vecType, double vecX, double vecY, double vecZ, double vecLen, Vector3D vecPos)
			: Vector3D(vecType,vecX,vecY,vecZ,vecLen), VectorOffset(vecPos)
	{
		this->VectorTimestamp.Set();
		if(VectorType_LATLON == vecPos.VectorType)
		{
			this->VectorTimestamp.SetLongitude(vecPos.VectorY);
		}
	}
	Vector4D::~Vector4D()
	{
	}

	Vector3D* Vector4D::SetOffsetLATLON(double latX, double lonY, double heightZ)
	{
		this->VectorOffset.VectorType = VectorType_LATLON;
		this->VectorOffset.VectorLength = 1.0;
		this->VectorOffset.VectorX = latX;
		this->VectorOffset.VectorY = lonY;
		this->VectorTimestamp.SetLongitude(lonY);
		this->VectorOffset.VectorZ = heightZ;
		return(&this->VectorOffset);
	}

	const char* Vector4D::ToString(void) const
	{
		return(Vector3D::ToString());
	}

};
