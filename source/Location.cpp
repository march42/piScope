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

	MHLocation::MHLocation(MHLocation* loc)
		: MHVector3D((MHVector3D*)loc)
	{
		if(NULL != loc->Name)
		{
			this->Name = new char[strlen(loc->Name) +1];
			strcpy(this->Name, loc->Name);
		}
		//	call Vector3D copy
		assert(this->Type == loc->Type);
		assert(this->X == loc->X);
		assert(this->Y == loc->Y);
		assert(this->Z == loc->Z);
		assert(this->Length == loc->Length);
	}
	MHLocation::MHLocation(double lat, double lon, double height, const char* name)
		: MHVector3D(VectorType_LATLON, lat, lon, height, 1.0), Name(NULL)
	{
		this->SetName(name);	//	set name, if given
		this->Validate();
	}
	MHLocation::~MHLocation()
	{
		this->SetName(NULL);	//	free name buffer
	}

	MHLocation* MHLocation::Set(double lat, double lon, double height, const char* name)
	{
		this->Type = VectorType_LATLON;
		this->X = lat;
		this->Y = lon;
		this->Z = height;
		this->Length = 1.0L;
		this->SetName(name);
		this->Validate();	//	just, to be sure
		return(this);
	}
	const char* MHLocation::SetName(const char* name)
	{
		/*if(NULL != this->Name)
		{
			delete(this->Name);
			this->Name = NULL;
		}*/
		this->Name = NULL;
		if(NULL != name)
		{
			this->Name = new char[strlen(name) +1];
			*(this->Name) = '\0';
			strcpy(this->Name, name);
		}
		return(this->Name);
	}

	const char* MHLocation::ToString(MHLocation* loc) const
	{
		MHLocation* source = (NULL==loc ?(MHLocation*)this :loc);
		return(source->MHVector3D::ToString());
	}

	MHVector3D* MHLocation::ToVector(MHLocation* loc) const
	{
		MHLocation* source = (NULL==loc ?(MHLocation*)this :loc);
		return(new MHVector3D(VectorType_LATLON,source->X,source->Y,source->Z,source->Length));
	}

	double MHLocation::GetLatitude(void) const
	{
		return(this->X);
	}
	double MHLocation::GetLongitude(void) const
	{
		return(this->Y);
	}
	double MHLocation::GetHeight(void) const
	{
		return(this->Z);
	}
	const char* MHLocation::GetName(const char* NULLRETURN) const
	{
		return(NULL==this->Name ?NULLRETURN :this->Name);
	}

};
