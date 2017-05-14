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

/*!	\brief	class MHAstroVector
 *
 *	Declaration of class, members and methods.
 *	Special vector data definition and calculation methods for astronomical vector.
 *	Combines Vector3D with Location, Offset and TimeStamp.
 */

#ifndef _ASTROVECTOR_HPP_
#	define _ASTROVECTOR_HPP_

#	include "config.hpp"
#	include "Vector3D.hpp"
#	include "AstroTime.hpp"

#	include <unistd.h>

namespace piScope
{

	/*	type of vector	(see Vector3D.hpp)
	**	3DONLY defines a vector which timestamp is unneeded or not changing
	**	ECEF (earth centered, earth fixed) origin at center of mass of the earth X/Y axis lying on equatorial plane
	**		X axis from lon=180 (antipodal meridian) for negative values to lon=0 (prime meridian) for positive values
	**		Y axis from lon=90W for negative values to lon=90E for positive values
	**		Z axis on rotation axis of earth, negative values pointing south and positive values north
	**	LATLON are geo coordinates, fixed on Earth's mean surface
	**		X = latitude (degrees North), Y = longitude (degrees East), Z = heights above mean sea level
	**	LocalENU (east, north, up) centered/fixed on a local tangent/geodetic plane, east=x, north=y, up=z
	**	LocalNED (north, east, down) centered/fixed on a local tangent/geodetic plane, north=x', east=y', down=z'
	**	J2000 (ECI) fixed on Earth's Mean Equator and Equinox at 12:00 Terrestrial Time on 1 January 2000
	**		X axis aligned with mean equinox, Y axis 90° east on celestial equator, Z axis aligned with celestial pole
	*/

	class MHAstroVector : public MHVector3D
	{
	private:	/* private members are accessible only from within the same class or "friends" */

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */
		MHLocation* LocationOffset;	/*!< positional offset, must always remain LATLON vector type */
		MHVector3D* BaseOffset;	/*!< base offset, may be len=0 just to indicate rotation */
		MHAstroTime* TS;	/*!< time stamp of vector */

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		MHAstroVector(MHAstroVector* vec);	/*!< constructor */
		MHAstroVector(MHVectorType_t vecType=VectorType_3DONLY, double vecX=0.0, double vecY=0.0, double vecZ=1.0
			, double vecLen=1.0, MHLocation* vecLoc=NULL);	/*!< constructor */
		~MHAstroVector();	/*!< destructor */

		//	public manipulation methods
		MHLocation* SetLocation(MHLocation* loc =NULL);	/*!< set new Location */
		MHLocation* SetLocation(double latX, double lonY, double heightZ);	/*!< set new Location */
		MHVector3D* SetBase(double vecX, double vecY, double vecZ, double vecLen);	/*!< set new Base Offset */
		MHAstroTime* SetTime(time_t ts =1);	/*!< set new time stamp */

		//	public access methods
		const char* ToString(void) const;	/*!< simple data output */
		time_t GetElapsed(time_t ts =1) const;	/*!< get elapsed seconds since time stamp */
		double GetLocalSiderealAngle(void) const;	/*!< get angle of vector according to local sidereal time */
	};

};

#endif	/* _ASTROVECTOR_HPP_ */
