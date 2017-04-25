/*
**	AstroVector (.hpp/.cpp)
**	handling class for astronomical 3D vector with time extension, base and positional offset
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

#include "AstroVector.hpp"
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

	AstroVector::AstroVector(VectorType_t vecType, double vecX, double vecY, double vecZ, double vecLen, Location* vecLoc)
			: Vector3D(vecType,vecX,vecY,vecZ,vecLen), LocationOffset(vecLoc), BaseOffset(NULL), TS(NULL)
	{
		assert(NULL != this->LocationOffset);
		if(NULL == this->TS)
		{
			this->TS = new AstroTime(1, this->LocationOffset);
		}
	}
	AstroVector::~AstroVector()
	{
	}

	Location* AstroVector::SetLocation(double latX, double lonY, double heightZ)
	{
		if(NULL != this->LocationOffset)
		{
			this->LocationOffset->Set(latX,lonY,heightZ);
		}
		else
		{
			this->LocationOffset = new Location(latX,lonY,heightZ);
		}
		return(this->LocationOffset);
	}

	Vector3D* AstroVector::SetBase(double vecX, double vecY, double vecZ, double vecLen)
	{
		if(NULL != this->BaseOffset)
		{
			this->BaseOffset->Set(VectorType_3DONLY,vecX,vecY,vecZ,vecLen);
		}
		else
		{
			this->BaseOffset = new Vector3D(VectorType_3DONLY,vecX,vecY,vecZ,vecLen);
		}
		return(this->BaseOffset);
	}

	AstroTime* AstroVector::SetTime(time_t ts)
	{
		if(NULL != this->TS)
		{
			this->TS->Set(ts);
		}
		else
		{
			this->TS = new AstroTime(ts,this->LocationOffset);
		}
		return(this->TS);
	}

	const char* AstroVector::ToString(void) const
	{
		return(Vector3D::ToString());
	}

};
