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

#include "MACROS.h"
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
		: Name(NULL), Location(NULL)
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
		//this->Orientation = new MHAstroVector(VectorType_3DONLY, 0.0,0.0,0.0, 0.0, this->Location);
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
		//	limit queue to 1000 values
		while(1000 < this->Orientation.size())
		{
			this->Orientation.pop_front();
		}
		MHAstroVector* vec = new MHAstroVector(this->Orientation.back());
		this->printLog(9,"last orientation:\t%s\n", vec->ToString());
		//	remove all values older than 300 seconds or differ more than 1 percent
		//	leave a minimum of 100 values in queue
		while(100 < this->Orientation.size() && (300 < this->Orientation.front()->GetElapsed()
			|| 0.01 < abs(this->Orientation.front()->GetOffsetX(vec->GetX()))
			|| 0.01 < abs(this->Orientation.front()->GetOffsetY(vec->GetY()))
			|| 0.01 < abs(this->Orientation.front()->GetOffsetZ(vec->GetZ()))))
		{
			this->printLog(9,"remove orientation:\t%s\n", this->Orientation.front()->ToString());
			this->Orientation.pop_front();
		}
		//	calculate average
		double px = 0, py = 0, pz = 0;
		if(1 < this->Orientation.size())
		{
			size_t pos;
			for(pos=0; pos < this->Orientation.size(); ++pos)
			{
				px += this->Orientation.at(pos)->GetX();
				py += this->Orientation.at(pos)->GetY();
				pz += this->Orientation.at(pos)->GetZ();
			}
			px /= pos;
			py /= pos;
			pz /= pos;
			this->printLog(9,"averaging orientation:\t%d %f,%f,%f\n", pos, px,py,pz);
			vec->Set(px, py, pz, 0);
		}
		return(vec);
	}
	bool MHTelescope::GetOrientation(double* RA, double* DEC)
	{
		if(NULL == RA || NULL == DEC || 0 == this->Orientation.size())
		{
			return(false);
		}
		//	get orientation
		MHAstroVector* vec = this->GetOrientation();
		if(NULL == vec || VectorType_INVALID == vec->GetType())
		{
			return(false);
		}
		//	convert:	local hour angle = local sidereal time - right ascension
		*RA = vec->GetLocalSiderealAngle();
		//	declination = elevation + ecliptic
		double rad = asin (vec->GetY() / (sqrt(pow(vec->GetX(),2) + pow(vec->GetY(),2) + pow(vec->GetZ(),2))));
		*DEC = RAD2DEG(rad);
		//	return
		return(true);
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
		//	always prepare log to same name
		this->SetLogName(value);
		//	thread safe switch to name buffer
		this->Name = value;
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
			//	successfully read
			if(!this->IMUpthread_running)
			{
				//	no output, if threaded reading
				this->printLog(9,"PollIMUSensor:\t%s\n", "IMU sensor read");
			}
			this->ImuData = this->ImuSensor->getIMUData();
			//	new data
			if(this->ImuData.accelValid && this->ImuData.compassValid)
			{
#				if defined(CALCULATE_ORIENTATION)
				RTVector3 accel = this->ImuData.accel;
				//accel.normalize();	//	x+y+z=1
				RTVector3 compass = this->ImuData.compass;
				//compass.normalize();	//	x,y,z = x,y,z /sqrt(x^2 + y^2 + z^2)
				/*	value ranges and fusion
				**	accelerometer (-1 .. length .. +1 g) measuring earth gravitational vector pointing to center of mass
				**	compass (-25-65 .. length .. +25-65 microtesla) measuring earth magnetic field lines (ranging from 25 to 65 microtesla)
				**	RTIMULib compass scale for +/- 4gauss = 0.014 -> 4/32768=0.00014 so values are in microtesla (10000gauss=1tesla, 1gauss=100microtesla)
				**	compass value needs to be fit to -1..+1 with x,y,z = x,y,z /(x+y+z)
				*/
				double compassLength = sqrt(pow(compass.x(),2) + pow(compass.y(),2) + pow(compass.z(),2));	// length of compass vector for normalize
				RTVector3 compass1(compass.x() /compassLength, compass.y() /compassLength, compass.z() /compassLength);	// normalized compass vector
				//	calculate angle
				/*	tri axis tilt sensing
				**	alpha = arcsin( ax1 / g )
				**	beta  = arcsin( ay1 / g )
				**	gamma = arccos( az1 / g )
				*/
				/*	tri axis trigonometric
				**	alpha=pitch = arctan( ax1 / sqrt( ay1^2 + az1^2 ) )	// -pi<atan2(y,x)<+pi
				**	beta=roll   = arctan( ay1 / sqrt( ax1^2 + az1^2 ) )
				*/
				double ALPHA = asin(accel.x() / sqrt((accel.x()*accel.x()) + (accel.y()*accel.y()) + (accel.z()*accel.z())));	// -1<a<+1
				double BETA = asin(accel.y() / sqrt((accel.x()*accel.x()) + (accel.y()*accel.y()) + (accel.z()*accel.z())));	// -1<a<+1
				//double GAMMA = acos(accel.z() / sqrt((accel.x()*accel.x()) + (accel.y()*accel.y()) + (accel.z()*accel.z())));	// -1<a<+1
				/*	tilt compensation
				**	transform magnetometer data to horizontal plane (compassHorizontalX,compassHorizontalY)
				**	calculate compensated angle = atan2( -HoriY / HoriX )
				*/
				double compassHorizontalX = (compass1.x() * cos(BETA)) + (compass1.y() * sin(BETA) * sin(ALPHA)) + (compass1.z() * sin(BETA) * cos(ALPHA));
				double compassHorizontalY = (compass1.y() * cos(ALPHA)) + (compass1.z() * sin(ALPHA));
				double GAMMA = atan2(-1 * compassHorizontalY, compassHorizontalX);
#				else
				RTVector3 pose = this->ImuSensor->getMeasuredPose();	//	roll,pitch,yaw
				double ALPHA = pose.x();	//	?pitch
				double BETA = pose.y();		//	?roll
				double GAMMA = pose.z();	//	yaw
#				endif
				//	create vector and push to queue
				MHAstroVector* ori = new MHAstroVector(VectorType_LocalRPY, ALPHA, BETA, GAMMA, 0);
				#if defined(DONT_OPTIMIZE_TIMESTAMP)
				//	this->ImuData.timestamp = RTMath::currentUSecsSinceEpoch()
				struct timeval tvnow;
				gettimeofday(&tvnow, NULL);
				time_t tstamp = time(NULL);
				tvnow.tv_sec -= this->ImuData.timestamp / 1000000;	//	get offset
				tstamp -= tvnow.tv_sec;
				ori->SetTime(tstamp);
				#else
				ori->SetTime(this->ImuData.timestamp /1000000);
				#endif
				this->Orientation.push_back(ori);
				return(true);	//	successfully polled IMU sensor
			}
		}
		else
		{
			//	no new data, or reading failed
			if(!this->IMUpthread_running)
			{
				//	no output, if threaded reading
				this->printLog(2,"PollIMUSensor:\t%s\n", "error reading or no new data");
			}
		}
		return(false);	//	polling failed, no new data or accel/compass not valid
	}

	bool MHTelescope::ImuNotMoving(void)
	{
		bool tsValid = true;
		#if defined(USE_TIMESTAMP_IMUNOTMOVING)
		MHAstroTime tsimu(this->ImuData.timestamp, this->Position);
		tsValid = 100 > tsimu.GetElapsed();
		#endif
		//	not moving, if x+y+z below 0.1 radians per second
		return(tsValid && this->ImuData.gyroValid && 0.1 > (this->ImuData.gyro.x() + this->ImuData.gyro.y() + this->ImuData.gyro.z()));
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
					mother->printLog(8,"IMU:\t%dHz\tgyro%s=[%f,%f,%f]\tacc%s=[%f,%f,%f]\tmag%s=[%f,%f,%f]\n", read_rate
						, (mother->ImuData.gyroValid ?"" :"!"), mother->ImuData.gyro.x(),mother->ImuData.gyro.y(),mother->ImuData.gyro.z()
						, (mother->ImuData.accelValid ?"" :"!"), mother->ImuData.accel.x(),mother->ImuData.accel.y(),mother->ImuData.accel.z()
						, (mother->ImuData.compassValid ?"" :"!"), mother->ImuData.compass.x(),mother->ImuData.compass.y(),mother->ImuData.compass.z());
				}
				//	clear Orientation deque on movement
				if(mother->ImuData.gyroValid && read_rate < (int)mother->Orientation.size()
					&& 0.2 < (abs(mother->ImuData.gyro.x()) + abs(mother->ImuData.gyro.y()) + abs(mother->ImuData.gyro.z())))
				{
					mother->printLog(8,"IMU:\tclear on movement [%f,%f,%f]\n", abs(mother->ImuData.gyro.x()), abs(mother->ImuData.gyro.y()), abs(mother->ImuData.gyro.z()));
					mother->Orientation.clear();
				}
				usleep(1000000 / read_rate);	//	calculate micro seconds from polling rate
			}
			else
			{
				usleep(1000);	//	poll rate of 1kHz, if polling fails
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
		//	clear orientation buffer
		this->Orientation.clear();
	}
#	endif

};
