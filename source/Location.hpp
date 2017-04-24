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

#ifndef _LOCATION_HPP_
#	define _LOCATION_HPP_

#	include "Vector4D.hpp"

#	include <unistd.h>

namespace piScope
{

	class Location
	{
	private:	/* private members are accessible only from within the same class or "friends" */
		double latitude;	//	degrees north, -90=south, 0=equator, +90=north
		double longitude;	//	degrees east, -90=west, 0=greenwich, +90=east, -180/+180=opposite greenwich
		int height;	//	meter height above sea level
		char* name;

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		Location(double lat =0.0, double lon =0.0, int height =0, const char* name =NULL);
		Location(float lat, float lon, float height);
		~Location();

		//	public manipulation methods
		Location* Validate(Location* loc =NULL);

		//	public access methods
		const char* ToString(Location* loc =NULL) const;
		Vector3D* ToVector(Location* loc =NULL) const;
	};

};

#endif	/* _LOCATION_HPP_ */
