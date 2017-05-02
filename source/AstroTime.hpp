/*
**	AstroTime (.hpp/.cpp)
**	handling class for astronomical time
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

#ifndef _ASTROTIME_HPP_
#	define _ASTROTIME_HPP_

#	include "TimeStamp.hpp"
#	include "Location.hpp"

#	include <ctime>

namespace piScope
{

	class MHAstroTime : public MHTimeStamp
	{
	private:	/* private members are accessible only from within the same class or "friends" */

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */
		MHLocation* TimeLocation;

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		MHAstroTime(MHAstroTime* ts);
		MHAstroTime(time_t ts =1, MHLocation* loc =NULL);	//	ts==1 flag, to get current time
		~MHAstroTime();

		//	public manipulation methods
		time_t Set(time_t ts =1);
		MHLocation* SetLocation(MHLocation* loc =NULL);
		MHLocation* SetLongitude(float lon =0.0);

		//	public access methods
		time_t Get(int type =-1) const;	//	get time stamp (-1=UTC, 0=LMST, 1=GMST)
		double GetAngleMST(int GMST=true) const;
		MHLocation* GetLocation(void) const;
		const char* ToString(int type =-1) const;	//	get time stamp (-1=UTC, 0=LMST, 1=GMST)
	};

};

#endif	/* _ASTROTIME_HPP_ */
