/*
**	Vector3D (.hpp/.cpp)
**	handling class for 3D vector
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

/*!	\brief	class MHVector3D
 *
 *	Declaration of class, members and methods.
 *	General vector data definition and calculation methods.
 */

#ifndef _VECTOR3D_HPP_
#	define _VECTOR3D_HPP_

#	include <unistd.h>

namespace piScope
{

	/*	theory of vector
	**
	**	Distance can be noted in many different units. Most commonly used are meter and mile, while in astronomy pc,au,ly are used.
	**	The parsec defined as 1 pc = 648000/pi au is the distance of sun and a star with parallax angle of 1 arcsecond.
	**	The astronomical unit (mean distance between sun and earth) defined as 1au=149,597,870,700m
	**	The light year (distance travelled by light in 1 year) is no standard and  mostly used by non-professionals.
	**	just to answer the question more precisely, speed of light in vacuum 299,792,458m/s and Julian year of 365.25 days (of 86,400 seconds)
	**	btw. Julian year is although not an international SI unit and differs from Sidereal year of 365.25636 SI-days at J2000.0 epoch.
	**
	**	Angles can be noted as radian (0<rho<2pi), degree (0<rho<360), gon/grad/grade/gradian (0<rho<400),
	**	arcminute/minute/prime (0<rho< 21600'=360°*60min), arcsecond/second/double-prime (0<rho< 1296000"=360°*60m*60s),
	**	hour-angle/right-ascension (0<rho<24h) with given minute and second are fractions of hour not arcminute and arcsecond
	**	2pi radian = 360° = 400gon = 360*60' = 360*60*60" = 24h00m00s
	**	astrological SIGN = 1/12th of circle
	**	angular mil = 1/1000th of 1gon = 1mgon = 1/400000 of circle
	**	revolution = 1 circle or full turn or cycle
	**	quadrant = 1/4th turn denoted by roman I,II,III,IV
	**	sextant or sextans = 1/6th turn
	**
	**	linear vector - expressed in equal distance units on all axis, 2D when Z axis, or mathematically either axis, zero
	**		3DONLY where Length is used, to set base unit (Length=0 describes a rotation only axial vector)
	**	polar vector (2D) - expressed in polar distance and angle
	**	cylindrical vector (3D) - expressed in polar distance, angle and height
	**	spherical vector (3D) - expressed in magnitude (distance), azimuth angle on x axis and zenith angle on z axis (in radian)
	**		LATLON
	**
	**	data and used mathematics
	**	simple Phytagoras (a^2 + b^2 = c^2)
	**	sinus and cosinus (angles in radian)
	**	Euler angles (z-x-z, x-y-x, y-z-y, z-y-z, x-z-x, y-x-y) - plane x,y rotates from x around z with angle alpha to N,
	**	around N from z to Z with angle beta, around y from N to X with angle gamma
	**	Quaternion generally represented as [a + b*i + c*j + d*k] Hamilton formula (i^2 = j^2 = k^2 = i*j*k = -1)
	*/
	/*	stored values for types
	**
	**	3DONLY defines a vector which timestamp is unneeded or not changing
	**		X,Y,Z	distance
	**	LATLON are geo coordinates, fixed on Earth's mean surface
	**		X,Y in decimal degree, X = latitude (degrees North), Y = longitude (degrees East)
	**		Z in meter, height above earth's mean radius (mean sea level)
	**	ECEF (earth centered, earth fixed) origin at center of mass of the earth X/Y axis lying on equatorial plane
	**		X axis from lon=180 (antipodal meridian) for negative values to lon=0 (prime meridian) for positive values
	**		Y axis from lon=90W for negative values to lon=90E for positive values
	**		Z axis on rotation axis of earth, negative values pointing south and positive values north
	**	LocalENU (east, north, up) centered/fixed on a local tangent/geodetic plane, east=x, north=y, up=z
	**	LocalNED (north, east, down) centered/fixed on a local tangent/geodetic plane, north=x', east=y', down=z'
	**	LocalRPY
	**		angles in radian X=roll, Y=pitch, Z=yaw
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
		VectorType_LocalRPY,
		VectorType_J2000,
	}	MHVectorType_t;	/*!< Type of the Vector */

	class MHVector3D
	{
	private:	/* private members are accessible only from within the same class or "friends" */

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */
		MHVectorType_t Type;	/*!< Type of the Vector */
		double X;	/*!< X component of the Vector */
		double Y;	/*!< Y component of the Vector */
		double Z;	/*!< Z component of the Vector */
		double Length;	/*!< vector length scale factor, >0 offset, ==0 rotation */

		//	internal handling methods
		bool FixLatLon(double* lat =NULL, double* lon =NULL);	/*!< fix values of LAT/LON, return true=changed */

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		MHVector3D(MHVectorType_t vecType=VectorType_3DONLY, double vecX=0.0, double vecY=0.0, double vecZ=0.0, double vecLen=0.0);
		MHVector3D(MHVector3D* vec);
		~MHVector3D();

		//	public manipulation methods
		MHVector3D* Set(MHVectorType_t vecType, double vecX, double vecY, double vecZ, double vecLen);	/*!< Set new values */
		MHVector3D* Set(double vecX, double vecY, double vecZ, double vecLen);	/*!< set new values */
		bool Validate(bool checkonly =false);	/*!< check values for validity, return true=valid */

		//	public conversion methods
		MHVector3D* Convert2ECEF(MHVector3D* sdVector =NULL);	/*!< convert current values to ECEF */

		//	public access methods
		const char* ToString(void) const;	/*!< simple vector output */
		double GetX(void) const;	/*!< get X component of vector */
		double GetOffsetX(double value) const;	/*!< get X component offset of vector to another */
		double GetY(void) const;	/*!< get Y component of vector */
		double GetOffsetY(double value) const;	/*!< get Y component offset of vector to another */
		double GetZ(void) const;	/*!< get Z component of vector */
		double GetOffsetZ(double value) const;	/*!< get Z component offset of vector to another */
		MHVectorType_t GetType(void) const;	/*!< get Type of vector */
	};

	//	some small helpers
	const char* Angle_Deg2HMS(double angle, double* H=NULL, double* M=NULL, double* S=NULL);	/*!< angle conversion routine */
	const char* Angle_Rad2HMS(double angle, double* H=NULL, double* M=NULL, double* S=NULL);	/*!< angle conversion routine */
	const char* Angle_Deg2DMS(double angle, double* D=NULL, double* M=NULL, double* S=NULL);	/*!< angle conversion routine */
	const char* Angle_Rad2DMS(double angle, double* D=NULL, double* M=NULL, double* S=NULL);	/*!< angle conversion routine */

};

#endif	/* _VECTOR3D_HPP_ */
