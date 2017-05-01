/*
**	Telescope (.hpp/.cpp)
**	handling class for telescope
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

#include "Telescope.hpp"

//#include <cstdio>
#include <cstdlib>
#include <cstring>
//#include <cmath>
//#include <climits>
#include <unistd.h>

#	if defined(USE_RTIMULIB)
/*	RTIMULib needs to be installed on system
**	https://github.com/RPi-Distro/RTIMULib.git
**	RTIMULib.h loaded in Telescope.hpp
*/
#		include <RTIMULib.h>
//#		include "RTIMUMagCal.h"
//#		include "RTIMUAccelCal.h"
//#		include <termios.h>
#		include <RTMath.h>
#	endif

namespace piScope
{

	MHTelescope::MHTelescope(const char* name)
		: Name(NULL), Location(NULL), Orientation(NULL)
#	if defined(USE_RTIMULIB)
		, ImuSetting(NULL), ImuSensor(NULL), IMUpthread_stopping(true), IMUpthread_running(false), IMUpthread(0)
#	endif
	{
		//	copy values to variables
		this->SetName(name);
		//	log will be set to same name, in SetName
		this->IMUpthread = pthread_self();	//	consider self==invalid for any sub thread
		//	create variable buffers
		this->Location = new MHLocation();
		this->Orientation = new MHAstroVector(VectorType_3DONLY, 0.0,0.0,0.0, 0.0, this->Location);
	}
	MHTelescope::~MHTelescope()
	{
		this->printLog(9,"MHTelescope destructor:\t%s\n", "begin");
		//	stopp IMU polling thread
		if(!pthread_equal(this->IMUpthread,pthread_self()))
		{
			this->IMUpthread_stopp();
		}
		this->printLog(9,"MHTelescope destructor:\t%s\n", "done");
	}

	const char* MHTelescope::GetName(const char* NULLRETURN) const
	{
		return(NULL==this->Name ?NULLRETURN :this->Name);
	}
	const char* MHTelescope::ToString(void) const
	{
		size_t len = 100;
		char* value = new char[len];
		size_t pos = 0;
		//	build return
		pos += snprintf(value+pos,len-pos, "%s", this->GetName());
		if(NULL != this->Location)
		{
			pos += snprintf(value+pos,len-pos, " @%s", this->Location->GetName());
		}
		//	return and leave value to garbage collection, unless needed by user
		return(value);
	}

	MHAstroVector* MHTelescope::GetOrientation(void)
	{
#		if defined(TRACE)
		fprintf(stderr, "DEBUG:\tthis=%p\n", this);
#		endif
		//MHAstroVector* vec = new MHAstroVector(this->Orientation);
		MHAstroVector* vec = this->Orientation;
#		if defined(TRACE)
		fprintf(stderr, "DEBUG:\tvec=%p\n", vec);
#		endif
		return(vec);
	}

	const char* MHTelescope::SetName(const char* name)
	{
		//	allocate new buffer and copy value
		char* value = new char[strlen(name) +1];
		strcpy(value,name);
		/*	maybe the pointer is used in sub classes, so leave delete to garbage collection
		//	free old buffer
		if(NULL != this->Name)
		{
			delete(this->Name);
		}*/
		//	thread safe switch to name buffer
		this->Name = value;
		//	always prepare log to same name
		this->SetLogName(this->Name);
		return(this->Name);
	}

	MHLocation* MHTelescope::SetLocation(double latitude, double longitude, double height, const char* name)
	{
		if(NULL == this->Location)
		{
			this->Location = new MHLocation(latitude,longitude,height, (NULL==name ?this->Name :name));
		}
		else
		{
			this->Location->Set(latitude,longitude,height, (NULL==name ?this->Name :name));
		}
		return(this->Location);
	}

#	if defined(USE_RTIMULIB)
	bool MHTelescope::InitIMUSensor(void)
	{
		//	try loading a settings file
		FILE* fd = NULL;
		char* fn = new char[FILENAME_MAX];
		snprintf(fn,FILENAME_MAX, "%sRTIMULib.ini", this->GetName(""));
		if(NULL != (fd = fopen(fn, "r")))
		{
			this->printLog(2,"found IMU settings %s\n",fn);
			//	file exists
			fclose(fd);
			fd = NULL;
			*(strrchr(fn,'.')) = '\0';
			//this->ImuSetting = new RTIMUSettings(fn);
		}
		else if(NULL != (fd = fopen("/etc/RTIMULib.ini", "r")))
		{
			this->printLog(2,"found IMU settings %s\n","/etc/RTIMULib.ini");
			//	file exists
			fclose(fd);
			fd = NULL;
			this->ImuSetting = new RTIMUSettings("/etc","RTIMULib");
		}
		//	load settings, or create new
		if(NULL == this->ImuSetting)
		{
			this->ImuSetting = new RTIMUSettings(fn);
		}
		//	settings loaded
		if(RTIMU_TYPE_AUTODISCOVER == this->ImuSetting->m_imuType
			&& !this->ImuSetting->m_accelCalValid && !this->ImuSetting->m_gyroBiasValid
			&& !this->ImuSetting->m_compassCalValid)
		{
			//	default settings, without calibration
			this->printLog(1,"IMU not configured\n");
			//m_axisRotation = RTIMU_XNORTH_YEAST;
			return(false);
		}
        //	found valid settings
        else
		{
			this->ImuSensor = RTIMU::createIMU(this->ImuSetting);
			if ((NULL == this->ImuSensor) || (RTIMU_TYPE_NULL == this->ImuSensor->IMUType()))
			{
				this->printLog(0,"no IMU found\n");
				return(false);
			}
			//  set up IMU
			this->printLog(9,"IMU sensor Init\n");
			return(this->ImuSensor->IMUInit() && this->PollIMUSensor());
		}
		return(false);	//	something went wrong
	}
	bool MHTelescope::PollIMUSensor(void)
	{
		if(NULL == this->ImuSensor)
		{
			this->printLog(2,"PollIMUSensor:\t%s\n", "no IMU sensor");
			return(false);	//	no sensor or error
		}
		else if(this->ImuSensor->IMURead())
		{
			if(!this->IMUpthread_running)
			{
				//	no output, if threaded reading
				this->printLog(9,"PollIMUSensor:\t%s\n", "IMU sensor read");
			}
			this->ImuData = this->ImuSensor->getIMUData();
			//	new data
			if(NULL == this->Orientation)
			{
				this->Orientation = new MHAstroVector;
			}
			if(NULL != this->Orientation && this->ImuNotMoving() && this->ImuData.accelValid)
			{
				this->Orientation->Set(VectorType_LocalNED, this->ImuData.accel.x(),this->ImuData.accel.y(),this->ImuData.accel.z(), 0);
				#if defined(DONT_OPTIMIZE_TIMESTAMP)
				//	this->ImuData.timestamp = RTMath::currentUSecsSinceEpoch()
				struct timeval tvnow;
				gettimeofday(&tvnow, NULL);
				time_t tstamp = time(NULL);
				tvnow.tv_sec -= this->ImuData.timestamp / 1000000;	//	get offset
				tstamp -= tvnow.tv_sec;
				this->Orientation->SetTime(tstamp);
				#else
				this->Orientation->SetTime(this->ImuData.timestamp /1000000);
				#endif
			}
		}
		else
		{
			if(!this->IMUpthread_running)
			{
				//	no output, if threaded reading
				this->printLog(2,"PollIMUSensor:\t%s\n", "error reading or no new data");
			}
			return(false);
		}
		return(true);
	}

	bool MHTelescope::ImuNotMoving(void)
	{
		bool tsValid = true;
		#if defined(USE_TIMESTAMP_IMUNOTMOVING)
		MHAstroTime tsimu(this->ImuData.timestamp, this->Position);
		tsValid = 100 > tsimu.GetElapsed();
		#endif
		return(tsValid && this->ImuData.gyroValid && 0.01 > (this->ImuData.gyro.x() + this->ImuData.gyro.y() + this->ImuData.gyro.z()));
	}

	void *IMUpthread_Polling(void *data)
	{
		MHTelescope* mother = (MHTelescope*)data;
		mother->printLog(2,"IMUpthread_Polling starting\n");
		int read_counter = 0;
		int read_rate = 100;
		while(!mother->IMUpthread_stopping)
		{
			mother->IMUpthread_running = true;
			if(mother->PollIMUSensor())
			{
				if(0 == (read_counter++ %(read_rate <<3)))
				{
					int interval = mother->ImuSensor->IMUGetPollInterval();	//	poll interval in ms
					read_rate = (500 < interval ?2 :(1000 / interval));	// 2Hz minimum polling rate
					//	1 second interval
					mother->printLog(9,"IMU:\tgyro%s=[%f,%f,%f]\tacc%s=[%f,%f,%f]\tmag%s=[%f,%f,%f]\n"
						, (mother->ImuData.gyroValid ?"" :"!"), mother->ImuData.gyro.x(),mother->ImuData.gyro.y(),mother->ImuData.gyro.z()
						, (mother->ImuData.accelValid ?"" :"!"), mother->ImuData.accel.x(),mother->ImuData.accel.y(),mother->ImuData.accel.z()
						, (mother->ImuData.compassValid ?"" :"!"), mother->ImuData.compass.x(),mother->ImuData.compass.y(),mother->ImuData.compass.z());
				}
				usleep(1000000 / read_rate);	//	100Hz polling
			}
		}
		mother->IMUpthread_running = false;
		mother->printLog(2,"IMUpthread_Polling stopped\n");
		pthread_exit(NULL);
	}
	void MHTelescope::IMUpthread_start(void)
	{
		this->printLog(9,"starting IMUpthread_Polling\n");
		if((this->IMUpthread_stopping = (NULL == this->ImuSetting || NULL == this->ImuSensor)))
		{
			this->printLog(9,"IMUpthread_Polling %s\n", "sensor not initialized before");
			this->IMUpthread_stopping = !this->InitIMUSensor();
		}
		//	IMU should be ready
		//	prepare thread attributes
		pthread_attr_init(&this->IMUpthread_attributes);
		pthread_attr_setdetachstate(&this->IMUpthread_attributes, PTHREAD_CREATE_JOINABLE);
		//	start thread
		int rc = pthread_create(&this->IMUpthread, &this->IMUpthread_attributes, IMUpthread_Polling, (void *)this);
		if(0 > rc)
		{
			this->printLog(0,"pthread_create failed, for IMUpthread_Polling\n");
		}
		else
		{
			this->printLog(9,"started IMUpthread_Polling\n");
			sleep(1);	//	give time to start the thread
		}
	}
	void MHTelescope::IMUpthread_stopp(void)
	{
		if(!this->IMUpthread_running)
		{
			this->printLog(1,"IMUpthread_stopp:\t%s\n", "thread not running");
			return;
		}
		this->printLog(2,"IMUpthread_stopp:\t%s\n", "stopping IMUpthread_Polling");
		//	set stopp flag
		this->IMUpthread_stopping = true;
		usleep(123456);
		//	destroy attribute
		pthread_attr_destroy(&this->IMUpthread_attributes);
		//	join and wait for thread completion
		pthread_join(this->IMUpthread, NULL);
		this->IMUpthread = pthread_self();
		this->printLog(9,"IMUpthread_stopp:\t%s\n", "stopped IMUpthread_Polling");
	}
#	endif

};
