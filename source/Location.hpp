/*
**	Location (.hpp/.cpp)
**	handling class for telescope location
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

/*!	\brief	class MHLocation
 *
 *	Declaration of class, members and methods.
 *	Special vector data definition and calculation methods for location data.
 *	Specializes Vector3D for Latitude, Longitude and Height.
 */

#ifndef _LOCATION_HPP_
#	define _LOCATION_HPP_

#	include "config.hpp"
#	include "Vector3D.hpp"

#	include <unistd.h>

namespace piScope
{

	class MHLocation : public MHVector3D
	{
	private:	/* private members are accessible only from within the same class or "friends" */
		char* Name;	/*!< Name of Location */
		/*
		double& Latitude = X;	//	degrees north, -90=south, 0=equator, +90=north
		double& Longitude = Y;	//	degrees east, -90=west, 0=greenwich, +90=east, -180/+180=opposite greenwich
		double& Height = Z;	//	meter height above sea level
		*/

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		MHLocation(MHLocation* loc);	/*!< constructor */
		MHLocation(double lat =0.0, double lon =0.0, double height =0.0, const char* name =NULL);	/*!< constructor */
		~MHLocation();	/*!< destructor */

		//	public manipulation methods
		MHLocation* Set(double lat, double lon, double height, const char* name =NULL);	/*!< set new location */
		const char* SetName(const char* name);	/*!< set new location name */

		//	public access methods
		const char* ToString(MHLocation* loc =NULL) const;	/*!< get simple output string */
		MHVector3D* ToVector(MHLocation* loc =NULL) const;	/*!< get Vector3D from Location */
		double GetLatitude(void) const;	/*!< get locations latitude */
		double GetLongitude(void) const;	/*!< get locations longitude */
		double GetHeight(void) const;	/*!< get locations height above mean sea level */
		const char* GetName(const char* NULLRETURN="UNNAMED") const;	/*!< get locations Name */
	};

};

#endif	/* _LOCATION_HPP_ */
