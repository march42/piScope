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

/*!	\brief	class MHTelescope
 *
 *	Declaration of class, members and methods.
 */

#ifndef _TELESCOPE_HPP_
#	define _TELESCOPE_HPP_

#	include "../config.hpp"
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
		char* Name;	/*!< Name of the telescope */
		MHLocation* Location;	/*!< Location of the telescope */
		std::deque<MHAstroVector*> Orientation;	/*!< Orientation of the telescope, deque for statistical precision */

	/*	RTIMULib members, for inertial measurement sensors
	**	ImuSetting
	**	ImuSensor
	**	ImuData
	*/
#	if defined(USE_RTIMULIB)
		RTIMUSettings* ImuSetting;	/*!< setting of RTIMULib IMU sensor*/
		RTIMU* ImuSensor;	/*!< access to RTIMULib IMU sensor */
		RTIMU_DATA ImuData;	/*!< current data of RTIMULib IMU sensor */
		//	threading for RTIMULib
		bool IMUpthread_stopping;	/*!< stopping flag for RTIMULib reading and handling thread */
		bool IMUpthread_running;	/*!< running flag for RTIMULib reading and handling thread */
		pthread_t IMUpthread;	/*!< POSIX thread handler of RTIMULib reading and handling thread */
		pthread_attr_t IMUpthread_attributes;	/*!< POSIX thread attributes of RTIMULib reading and handling thread */
		friend void *IMUpthread_Polling(void *data);	/*!< friend declaration for RTIMULib reading and handling thread */
#	endif

	public:	/* public members are accessible from anywhere */
		//	constructor/destructor
		MHTelescope(const char* name="telescope");	/*!< constructor */
		~MHTelescope();	/*!< destructor */

		//	access methods
		const char* GetName(const char* NULLRETURN="UNNAMED") const;	/*!< access to current Name */
		const char* ToString(void) const;	/*!< simple output function */
		MHAstroVector* GetOrientation(void);	/*!< calculate current orientation from queue */
		bool GetOrientation(double* RA, double* DEC);	/*!< calculate current orientation from queue */

		//	preparation and manipulation methods
		const char* SetName(const char* name);	/*!< set new Name */
		MHLocation* SetLocation(double latitude, double longitude, double height=0, const char* name=NULL);	/*!< set new Location */

	/*	RTIMULib members, for inertial measurement sensors
	**	InitIMUSensor
	**	PollIMUSensor
	*/
#	if defined(USE_RTIMULIB)
		bool InitIMUSensor(void);	/*!< initialize RTIMULib */
		bool PollIMUSensor(void);	/*!< poll RTIMULib sensor */
		bool ImuNotMoving(void);	/*!< check RTIMULib sensor not showing movement*/
		void IMUpthread_start(void);	/*!< start RTIMULib reading and handling thread */
		void IMUpthread_stopp(void);	/*!< stop RTIMULib reading and handling thread */
#	endif

	};

};

#endif	/* _TELESCOPE_HPP_ */
