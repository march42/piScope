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

#ifndef _VECTOR4D_HPP_
#	define _VECTOR4D_HPP_

#	include "Vector4D.hpp"
#	include "TimeStamp.hpp"

namespace piScope
{

	/*	type of vector
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
	typedef enum
	{
		VectorType_INVALID=-1,
		VectorType_3DONLY=0,
		VectorType_ECEF,
		VectorType_LATLON,
		VectorType_LocalENU,
		VectorType_LocalNED,
		VectorType_J2000,
	}	VectorType_t;

	//	predefine, for cross reference
	class Vector3D;
	class Vector4D;

	class Vector3D
	{
		friend class Vector4D;
	private:	/* private members are accessible only from within the same class or "friends" */

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */
		VectorType_t VectorType;
		double VectorX;
		double VectorY;
		double VectorZ;
		double VectorLength;	//	vector length

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		Vector3D(VectorType_t vecType=VectorType_3DONLY, double vecX=0.0, double vecY=0.0, double vecZ=1.0, double vecLen=0.0);
		~Vector3D();

		//	public conversion methods
		Vector3D* Convert2ECEF(Vector3D* sdVector =NULL);

		//	public access methods
		const char* ToString(void) const;
	};

	class Vector4D : public Vector3D
	{
	private:	/* private members are accessible only from within the same class or "friends" */

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */
		Vector3D VectorOffset;	//	positional offset, must always remain LATLON
		TimeStamp VectorTimestamp;	//	time stamp of vector

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		Vector4D(VectorType_t vecType=VectorType_3DONLY, double vecX=0.0, double vecY=0.0, double vecZ=1.0
			, double vecLen=1.0, Vector3D vecPos=Vector3D(VectorType_3DONLY,0,0,1,0));
		~Vector4D();

		//	public manipulation methods
		Vector3D* SetOffsetLATLON(double latX, double lonY, double heightZ);

		//	public access methods
		const char* ToString(void) const;
	};

};

#endif	/* _VECTOR4D_HPP_ */
