/*
**	MACROS (.h)
**	macros to use with calculation and formulas
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

/*!	\brief	calculation macros, constants and formulas
 *
 *	Declaration of constants, formulas and calculation macros
 *	for use in C/C++ code.
 *	Mostly for ease of coding and bug correction.
 */

#ifndef _MACROS_H_
#	define _MACROS_H_

#	include "../config.h"

	/*	conversion constants
	**
	*/
#	define FULLCIRCLE_DEGREE (360)
#	define FULLCIRCLE_RADIAN (M_PI * 2)
	/*	earth geometry
	**	RADIUS in m
	**	Gravity in m/s^2
	*/
#	define EARTH_MODELRADIUS (6378000.0L)
#	define EARTH_MEANRADIUS (6371008.0L)
#	define EARTH_EQUATORIALRADIUS (6378137.0L)
#	define EARTH_POLARRADIUS (6356752.314245L)
#	define EARTH_MEANGRAVITY (9.80665L)
	/*	earth sidereal parameters
	**	time values in seconds
	**	Seconds Per Day - seconds in a day for system time calculation
	**	Sidereal Day - seconds for earth to turn once (distant star crossing the meridian)
	**	Mean Solar Day - seconds between sun crossing meridian
	**	Julian day in hours
	*/
#	define EARTH_SECONDSPERDAY (86400)
#	define EARTH_SIDEREALDAY (86164.09053083288L)
#	define EARTH_MEANSOLARDAY (86400.002L)
#	define EARTH_JULIANDAY (24.06570982441908L)
	/*	earth sidereal parameters
	**	angles in degrees
	*/
#	define EARTH_ROTATIONSOLARDAY (360.9856L)
#	define EARTH_ROTATIONSIDEREAL (360.0L)
	/*	earth orbital parameters
	**	Solar Distance in km (converted from 10^6km)
	**	Orbit times in mean solar days
	**	Inclination - tilt angle at equatorial plane in degrees
	*/
#	define EARTH_ORBITAXIS (149.598262L * 10E+6L)
#	define EARTH_SIDEREALORBIT (365.256)
#	define EARTH_TROPICALORBIT (365.242)
#	define EARTH_PERIHELION (147.09L * 10E+6L)
#	define EARTH_APHELION (152.10L * 10E+6L)
#	define EARTH_INCLINATION (23.4393)

	//	converting degree/radian
#	define DEG2RAD(deg) ((deg) * FULLCIRCLE_RADIAN / FULLCIRCLE_DEGREE)
#	define RAD2DEG(rad) ((rad) * FULLCIRCLE_DEGREE / FULLCIRCLE_RADIAN)

	//	converting from seconds to Mean Sidereal Time hour angle (seconds per degree)
#	define EARTH_ROTATIONSPD (EARTH_MEANSOLARDAY / EARTH_ROTATIONSIDEREAL)

	//	DMS2DEG = D + (M /60) + (S /3600)
#	define LATLON_DMS2DEG(D,M,S) ((D) + ((M) /60.0L) + ((S) /3600.0L))
	//	HMS2DEG = (H *360/24) + (M *15/60) + (S *15/3600)
#	define LATLON_HMS2DEG(D,M,S) (((H) *(360.0L/24)) + ((M) *(15.0L/60.0L)) + ((S) *(15.0L/3600.0L)))

	//	ECEF_TIMEOFFSET = LongitudeDegrees * MeanSolarDaySeconds / MeanSolarDayRotationDegrees
#	define ECEF_TIMEOFFSET(lon) ((lon) * EARTH_MEANSOLARDAY / EARTH_ROTATIONSOLARDAY)

	//	UT1 (Universal Time) is essentially the same as GMT (Greenwich Mean Time)
	//	UT1 = UTC - DUT1 (see ftp://maia.usno.navy.mil/ser7/ser7.dat)
#	define TIME_UTC2UT1(utc) ((utc) - +0.4)
	//	Julian Day/Date of the system epoch (in UTC time)
#	define TIME_MJDEPOCH_JD (2400000.5L)
#	define TIME_UNIXEPOCH_JD (2440587.5L)
#	define TIME_UNIXEPOCH_MJD (TIME_UNIXEPOCH_JD - TIME_MJDEPOCH_JD)
	/*	julianDate =
	**	DAY - 32075
	**	+ 1461 *(YEAR + 4800 +(MONTH - 14)/ 12)/ 4
	**	+ 367 * (MONTH - 2 -(MONTH - 14)/ 12 *12) 2/ 12
	**	- 3 * ((YEAR + 4900 +(MONTH-14)/12)/100)/4;
	*/
#	define JULIAN_SECONDSPERDAY EARTH_SECONDSPERDAY
#	define JULIAN_DAYSPERYEAR (365.25L)
	//	Julian dates (JD) = count of days since noon Universal Time on January 1, 4713 BC (Julian calendar)
	//	Julian Date (simply use JD of epoch and add UTC converted to days)
#	define TIME_UTC2JD(utc) (TIME_UNIXEPOCH_JD + ((utc) / JULIAN_SECONDSPERDAY))
#	define TIME_UTC2MJD(utc) (TIME_UNIXEPOCH_MJD + ((utc) / JULIAN_SECONDSPERDAY))
#	define TIME_JD2UTC(jd) (((jd) - TIME_UNIXEPOCH_JD) * JULIAN_SECONDSPERDAY)
	//	Julian Centuries from reference date
#	define TIME_JULIANCENTURY(utc) (TIME_UTC2JD(utc) / (JULIAN_DAYSPERYEAR * 100))

	//	Day since epoch
#	define DAY_SINCE_EPOCH(time,epoch) (((time) - epoch) / EARTH_SECONDSPERDAY)
	/*	GMST = 280.46061837 + 360.98564736629 * d + 0.000388 * t^2
	**	d = Julian Days since J2000.0
	**	t = Julian Centuries since J2000.0 = d / 36525
	*/

	//	J2000_OBLIQUITY= 23° 26' 21.406" obliquity - angle between equatorial plane and ecliptic
#	define J2000_OBLIQUITY LATLON_DMS2DEG(23,26,21.406)
	//	J2000.0 epoch is exactly 12.00 noon in terrestrial time TT
#	define J2000_EPOCH_TT (946728000)
	//	J2000.0 epoch is January 1, 2000, 11:58:55.816 UTC (~ 12.00 GMT)
#	define J2000_EPOCH_UTC (946727936)
	//	J2000.0 epoch in Julian Day format is 2451545.0
#	define J2000_EPOCH_JD (2451545.0L)
	//	Modified Julian Date is number of days since midnight on November 17, 1858. (=2400000.5 days after day 0 of the Julian calendar)
#	define J2000_EPOCH_MJD (J2000_EPOCH_JD - TIME_MJDEPOCH_JD)
	//	J2000-JulianDay = count of days since J2000.0 epoch
#	define J2000_JULIANDAY(utc) (((utc) - J2000_EPOCH_UTC) / EARTH_SECONDSPERDAY)
#	define J2000_JULIANCENTURY(utc) (J2000_JULIANDAY(utc) / (JULIAN_DAYSPERYEAR * 100))
	/*	GMST and LMST with 0.1sec accuracy
	**	GMST and LMST are no real time stamp
	**	GMST/LMST is the RA (right ascension in hour angle) of vernal equinox or celestial meridian
	**	either on prime meridian (GMST) or observers locality
	*/
#	define J2000_UTC2GMST(utc) ((18.697374558L + (EARTH_JULIANDAY * J2000_JULIANDAY(utc))) * 3600.0L)
#	define J2000_GMSTFIX(gmst) {while(EARTH_SECONDSPERDAY <= gmst) gmst -= EARTH_SECONDSPERDAY; while(0 > gmst) gmst += EARTH_SECONDSPERDAY;}
#	define J2000_UTC2LMST(utc,lmst,lon) {for(lmst = (((18.697374558L + (EARTH_JULIANDAY * J2000_JULIANDAY(utc))) * 3600.0L) \
						+ ((lon) * EARTH_SECONDSPERDAY / EARTH_ROTATIONSOLARDAY)); EARTH_SECONDSPERDAY <= lmst; \
						lmst -= EARTH_SECONDSPERDAY){}; while(0 > lmst){lmst += EARTH_SECONDSPERDAY;}}
#	define J2000_NORTHPOLE_RA(utc) (0.00L - (0.641L * J2000_JULIANCENTURY(utc)))
#	define J2000_NORTHPOLE_DEC(utc) (90.00L - (0.557L * J2000_JULIANCENTURY(utc)))

#endif	/* _MACROS_H_ */
