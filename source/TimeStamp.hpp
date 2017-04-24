/*
**	TimeStamp (.hpp/.cpp)
**	handling class for time stamp
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

#ifndef _TIMESTAMP_HPP_
#	define _TIMESTAMP_HPP_

#	include <ctime>

namespace piScope
{

	class TimeStamp
	{
	private:	/* private members are accessible only from within the same class or "friends" */

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */
		time_t UTC_ts;
		float longitude;

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		TimeStamp(time_t ts =1, float lon =0.0);
		~TimeStamp();

		//	public access methods
		time_t Set(time_t ts =1);
		float SetLongitude(float lon =0.0);

		//	public access methods
		time_t Get(int type =-1) const;	//	get time stamp (-1=UTC, 0=LMST, 1=GMST)
		double GetJulianDate(int modified =0) const;	//	get Julian Date on prime meridian (0=JD, 1=MJD)
		const char* ToString(int type =-1) const;	//	get time stamp (-1=UTC, 0=LMST, 1=GMST)
	};

};

#endif	/* _TIMESTAMP_HPP_ */
