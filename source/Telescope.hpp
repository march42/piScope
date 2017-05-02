/*
**	Telescope (.hpp/.cpp)
**	handling class for telescope
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

#ifndef _TELESCOPE_HPP_
#	define _TELESCOPE_HPP_

#	include "LogFile.hpp"
#	include "TimeStamp.hpp"
#	include "AstroTime.hpp"
#	include "Vector3D.hpp"
#	include "AstroVector.hpp"
#	include "Location.hpp"

#	include <unistd.h>
#	include <pthread.h>
#	include <deque>

#	if defined(USE_RTIMULIB)
#		include <RTIMULib.h>
#	endif

namespace piScope
{

	class MHTelescope
		: public MHLogFile
	{
	private:	/* private members are accessible only from within the same class or "friends" */

	protected:	/* protected members are accessible from the same class or "friends" and derived classes */
		char* Name;
		MHLocation* Location;
		std::deque<MHAstroVector*> Orientation;

	/*	RTIMULib members, for inertial measurement sensors
	**	ImuSetting
	**	ImuSensor
	**	ImuData
	*/
#	if defined(USE_RTIMULIB)
		RTIMUSettings* ImuSetting;
		RTIMU* ImuSensor;
		RTIMU_DATA ImuData;
		//	threading for RTIMULib
		bool IMUpthread_stopping;
		bool IMUpthread_running;
		pthread_t IMUpthread;
		pthread_attr_t IMUpthread_attributes;
		friend void *IMUpthread_Polling(void *data);
#	endif

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		MHTelescope(const char* name="telescope");
		~MHTelescope();

		//	access methods
		const char* GetName(const char* NULLRETURN="UNNAMED") const;
		const char* ToString(void) const;
		MHAstroVector* GetOrientation(void);
		bool GetOrientation(double* RA, double* DEC);

		//	preparation and manipulation methods
		const char* SetName(const char* name);
		MHLocation* SetLocation(double latitude, double longitude, double height=0, const char* name=NULL);

	/*	RTIMULib members, for inertial measurement sensors
	**	InitIMUSensor
	**	PollIMUSensor
	*/
#	if defined(USE_RTIMULIB)
		bool InitIMUSensor(void);
		bool PollIMUSensor(void);
		bool ImuNotMoving(void);
		void IMUpthread_start(void);
		void IMUpthread_stopp(void);
#	endif

	};

};

#endif	/* _TELESCOPE_HPP_ */
