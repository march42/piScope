/*
**	MACROS (.h)
**	macros to use with calculation and formulas
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

#ifndef _MACROS_H_
#	define _MACROS_H_

	//	DMS2DEG = D + (M /60) + (S /3600)
#	define LATLON_DMS2DEG(D,M,S) ((D) + ((M) /60.0L) + ((S) /3600.0L))
	//	HMS2DEG = (H *360/24) + (M *15/60) + (S *15/3600)
#	define LATLON_HMS2DEG(D,M,S) (((H) *(360.0L/24)) + ((M) *(15.0L/60.0L)) + ((S) *(15.0L/3600.0L)))
	//	LATLON_MSL=6371km mean sea level - average radius of earth sphere
#	define LATLON_EARTHMR (6371000.0L)

	//	ECEF_MEANSOLARDAY = 86400 seconds - mean solar day
#	define ECEF_MEANSOLARDAY (86400.0L)
	//	ECEF_ROTATION = 360.9856 degrees / mean solar day
#	define ECEF_ROTATION (360.9856L)
	//	ECEF_TIMEOFFSET = LongitudeDegrees * MeanSolarDaySeconds / MeanSolarDayRotationDegrees
#	define ECEF_TIMEOFFSET(lon) ((lon) * ECEF_MEANSOLARDAY / ECEF_ROTATION)
	/*	ECEF_SIDEREALDAY = 86164.09053083288 seconds - sidereal day
	**	sidereal day is slightly shorter than mean solar day
	**	the earth turns exactly 360 degrees during a sidereal day
	*/
#	define ECEF_SIDEREALDAY (86164.09053083288L)

	//	J2000_OBLIQUITY= 23° 26' 21.406" obliquity - angle between equatorial plane and ecliptic
#	define J2000_OBLIQUITY LATLON_DMS2DEG(23,26,21.406)
	//	J2000.0 epoch is exactly 12.00 noon in terrestrial time TT
#	define J2000_EPOCH_TT (946728000)
	//	J2000.0 epoch is January 1, 2000, 11:58:55.816 UTC (~ 12.00 GMT)
#	define J2000_EPOCH_UTC (946727936)
	//	J2000.0 epoch in Julian Day format is 2451545.0
#	define J2000_EPOCH_JD (2451545.0L)
	//	Modified Julian Date is number of days since midnight on November 17, 1858. (=2400000.5 days after day 0 of the Julian calendar)
#	define J2000_EPOCH_MJD (2451544.0L - 2400000.5L)
	//	J2000-JulianDay = count of days since J2000.0 epoch
#	define J2000_JULIANDAY(utc) (((utc) - J2000_EPOCH_UTC) / ECEF_MEANSOLARDAY)
	/*	GMST and LMST with 0.1sec accuracy
	**	GMST and LMST are no real time stamp
	**	GMST/LMST is the RA (right ascension in hour angle) of vernal equinox or celestial meridian
	**	either on prime meridian (GMST) or observers locality
	*/
#	define J2000_UTC2GMST(utc) ((18.697374558L + (24.06570982441908L * J2000_JULIANDAY(utc))) * 3600.0L)
#	define J2000_GMSTFIX(gmst) {while(ECEF_MEANSOLARDAY <= gmst) gmst -= ECEF_MEANSOLARDAY; while(0 > gmst) gmst += ECEF_MEANSOLARDAY;}
#	define J2000_UTC2LMST(utc,lmst,lon) {for(lmst = (((18.697374558L + (24.06570982441908L * J2000_JULIANDAY(utc))) * 3600.0L) \
						+ ((lon) * ECEF_MEANSOLARDAY / ECEF_ROTATION)); ECEF_MEANSOLARDAY <= lmst; \
						lmst -= ECEF_MEANSOLARDAY){}; while(0 > lmst) lmst += ECEF_MEANSOLARDAY;}

	//	WGS84_EQUATORIALRADIUS = 6 378 137.0 m
#	define WGS84_EQUATORIALRADIUS (6378137.0L)
	//	WGS84_POLARRADIUS = 6 356 752.314 245 m
#	define WGS84_POLARRADIUS (6356752.314245L)

	//	UT1 (Universal Time) is essentially the same as GMT (Greenwich Mean Time)
	//	UT1 = UTC - DUT1 (see ftp://maia.usno.navy.mil/ser7/ser7.dat)
#	define TIME_UTC2UT1(utc) ((utc) - +0.4)
	//	Julian dates (JD) = count of days since noon Universal Time on January 1, 4713 BC (Julian calendar)
#	define TIME_UTC2JD(utc) (J2000_EPOCH_JD + (((utc) - J2000_EPOCH_UTC) / ECEF_MEANSOLARDAY))
#	define TIME_UTC2MJD(utc) (J2000_EPOCH_MJD + (((utc) - J2000_EPOCH_UTC) / ECEF_MEANSOLARDAY))
	//	Day since epoch
#	define DAY_SINCE_EPOCH(time,epoch) (((time) - epoch) / ECEF_MEANSOLARDAY)
	/*	GMST = 280.46061837 + 360.98564736629 * d + 0.000388 * t^2
	**	d = Julian Days since J2000.0
	**	t = Julian Centuries since J2000.0 = d / 36525
	*/

	//	converting degree/radian
#	define DEG2RAD(deg) ((deg) * M_PI / 180.0)
#	define RAD2DEG(rad) ((rad) * 180.0 / M_PI)

#endif	/* _MACROS_H_ */
