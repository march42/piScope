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
		time_t UTC;

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		TimeStamp(time_t ts =1);	//	==1 flag, to get current time
		~TimeStamp();

		//	public access methods
		time_t Set(time_t ts =1);

		//	public access methods
		time_t Get(void) const;	//	get time stamp (-1=UTC, 0=LMST, 1=GMST, 2=JD, 3=MJD)
		double GetJulianDate(int modified =false) const;	//	get Julian Date on prime meridian (0=JD, 1=MJD)
		const char* ToString(void) const;	//	get time stamp (-1=UTC, 0=LMST, 1=GMST, 2=JD, 3=MJD)
	};

};

#endif	/* _TIMESTAMP_HPP_ */
