/*
**	AstroVector (.hpp/.cpp)
**	handling class for astronomical 3D vector with time extension, base and positional offset
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

	MHAstroVector::MHAstroVector(MHAstroVector* vec)
		: LocationOffset(NULL), BaseOffset(NULL), TS(NULL)
	{
		this->Type = vec->Type;
		this->X = vec->X;
		this->Y = vec->Y;
		this->Z = vec->Z;
		this->Length = vec->Length;
#		if defined(TRACE)
		fprintf(stderr, "DEBUG:\tvec=%p, loc=%p, base=%p, ts=%p\n", vec,vec->LocationOffset,vec->BaseOffset,vec->TS);
#		endif
		if(NULL != vec->LocationOffset)
		{
			this->LocationOffset = new MHLocation(vec->LocationOffset);
		}
		if(NULL != vec->BaseOffset)
		{
			this->BaseOffset = new MHVector3D(vec->BaseOffset);
		}
		if(NULL != vec->TS)
		{
			this->TS = new MHAstroTime(vec->TS);
		}
	}
	MHAstroVector::MHAstroVector(MHVectorType_t vecType, double vecX, double vecY, double vecZ, double vecLen, MHLocation* vecLoc)
			: MHVector3D(vecType,vecX,vecY,vecZ,vecLen), LocationOffset(vecLoc), BaseOffset(NULL), TS(NULL)
	{
		if(NULL == this->LocationOffset)
		{
			this->LocationOffset = new MHLocation(0,0,0,"UNSET");
		}
		assert(NULL != this->LocationOffset);	//	Location will be at least set to 0,0,0 3DONLY in Vector3D constructor
		if(NULL == this->TS)
		{
			this->TS = new MHAstroTime(1, this->LocationOffset);
		}
	}
	MHAstroVector::~MHAstroVector()
	{
	}

	MHLocation* MHAstroVector::SetLocation(MHLocation* loc)
	{
		return(this->LocationOffset = new MHLocation(loc));
	}
	MHLocation* MHAstroVector::SetLocation(double latX, double lonY, double heightZ)
	{
		if(NULL != this->LocationOffset)
		{
			this->LocationOffset->Set(latX,lonY,heightZ);
		}
		else
		{
			this->LocationOffset = new MHLocation(latX,lonY,heightZ);
		}
		return(this->LocationOffset);
	}

	MHVector3D* MHAstroVector::SetBase(double vecX, double vecY, double vecZ, double vecLen)
	{
		if(NULL != this->BaseOffset)
		{
			this->BaseOffset->Set(VectorType_3DONLY,vecX,vecY,vecZ,vecLen);
		}
		else
		{
			this->BaseOffset = new MHVector3D(VectorType_3DONLY,vecX,vecY,vecZ,vecLen);
		}
		return(this->BaseOffset);
	}

	MHAstroTime* MHAstroVector::SetTime(time_t ts)
	{
		if(NULL != this->TS)
		{
			this->TS->Set(ts);
		}
		else
		{
			this->TS = new MHAstroTime(ts,this->LocationOffset);
		}
		return(this->TS);
	}

	const char* MHAstroVector::ToString(void) const
	{
		return(MHVector3D::ToString());
	}

	time_t MHAstroVector::GetElapsed(time_t ts) const
	{
		return(this->TS->GetElapsed(ts));
	}

};
