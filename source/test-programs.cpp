/*	test-programs.cpp
**	combined test routines and programs
**	__TEST_I2CSENSOR__	tests for I2C IMU sensor
**	__TEST_VECTOR__		tests for Vector classes
**	__TEST_RTIMULIB__	tests for RTIMULib orientation sensing
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

/*!	\brief	combined source of test programs
 *
 *	__TEST_I2CSENSOR__
 *	__TEST_VECTOR__
 *	__TEST_RTIMULIB__
 */

//#if defined(__TEST_I2CSENSOR__)
#	include "I2Csensor.hpp"
#	include "IMU.hpp"
//#elif defined(__TEST_VECTOR__)
#	include "AstroVector.hpp"
//#elif defined(__TEST_RTIMULIB__)
#	include "AstroTime.hpp"
#	include "AstroVector.hpp"
#	include "Telescope.hpp"
//#else
//#	error "NO TARGET SPECIFIED FOR COMPILING: " __FILE__
//#endif // defined(__TEST_I2CSENSOR__) || defined(__TEST_VECTOR__) || defined(__TEST_RTIMULIB__)

#include <unistd.h>
#include <cstdio>
#include <iostream>

static volatile bool keep_running = true;
#ifdef WIN32
	static BOOL signal_handler(DWORD fdwCtrlType)
	{
		switch (fdwCtrlType)
		{
			case CTRL_C_EVENT:
			case CTRL_BREAK_EVENT:
			case CTRL_CLOSE_EVENT:
			case CTRL_SHUTDOWN_EVENT:
				keep_running = false;
				return TRUE;
			case CTRL_LOGOFF_EVENT:
				break;
		}
		return FALSE;
	}
#else
#	include <signal.h>
	static void signal_handler(int signum)
	{
		switch (signum)
		{
			case SIGINT:
			case SIGQUIT:
			case SIGTERM:
				keep_running = false;
				break;
			default:
				// just ignore
				break;
		}
	}
#endif

int test_i2csensor(int argc, char* argv[], char* envp[])
{
	//	parameters may be unused
	(void)argc;
	(void)argv;
	(void)envp;
	//	main routine
	rpiScope::I2Csensor imu(rpiScope::I2C_AutoIdentify,-1,"/dev/i2c-1");
	imu.pthread_I2Creading();
	while(keep_running)
	{
		if(rpiScope::I2C_NoSensor == imu.sensortype)
		{
			break;
		}
		rpiScope::IMU_Vector*	Acceleration = imu.IMUvalue.Acceleration();
		rpiScope::IMU_Vector*	Gyroscope = imu.IMUvalue.Gyroscope();
		rpiScope::IMU_Vector*	Magnetometer = imu.IMUvalue.Magnetometer();
		rpiScope::IMU_Vector*	euler = imu.IMUvalue.Orientation();
		rpiScope::IMU_Vector*	fusion = imu.IMUvalue.Fusion3D();
#		if defined(DEBUG3)
		std::cout << "\t" << "a=" << Acceleration->X << "," << Acceleration->Y << "," << Acceleration->Z
			<< "\t" << "a=" << Acceleration->scaledX() << "," << Acceleration->scaledY() << "," << Acceleration->scaledZ()
			<< std::endl;
		std::cout << "\t" << "g=" << Gyroscope->X << "," << Gyroscope->Y << "," << Gyroscope->Z
			<< "\t" << "g=" << Gyroscope->scaledX() << "," << Gyroscope->scaledY() << "," << Gyroscope->scaledZ()
			<< std::endl;
		std::cout << "\t" << "m=" << Magnetometer->X << "," << Magnetometer->Y << "," << Magnetometer->Z
			<< "\t" << "m=" << Magnetometer->scaledX() << "," << Magnetometer->scaledY() << "," << Magnetometer->scaledZ()
			<< std::endl;
#		elif defined(DEBUG2)
		std::cout
			<< "\t" << "a=" << Acceleration->scaledX() << "," << Acceleration->scaledY() << "," << Acceleration->scaledZ()
			<< "\t" << "g=" << Gyroscope->scaledX() << "," << Gyroscope->scaledY() << "," << Gyroscope->scaledZ()
			<< "\t" << "m=" << Magnetometer->scaledX() << "," << Magnetometer->scaledY() << "," << Magnetometer->scaledZ()
			<< std::endl;
#		endif
		std::cout
			<< "\t" << "E=" << euler->scaledX() << "," << euler->scaledY() << "," << euler->scaledZ()
			<< "\t" << "F=" << fusion->scaledX() << "," << fusion->scaledY() << "," << fusion->scaledZ()
			<< std::endl;
		//	free vector data
		delete(Acceleration);
		delete(Gyroscope);
		delete(Magnetometer);
		delete(euler);
		delete(fusion);
		sleep(1);
	}
	return(0);
}

int test_vector(int argc, char* argv[], char* envp[])
{
	//	parameters may be unused
	(void)argc;
	(void)argv;
	(void)envp;

	//	test Location
	piScope::MHLocation here(TESTLOCATION);
	fprintf(stdout, "\tLocation:\t%s\n", here.ToString());
	fprintf(stdout, "\tLocation:\t%s\n", (new piScope::MHLocation(-23.9999999999,-89.123456789,987.654321))->ToString());
	fprintf(stdout, "\tLocation:\t%s\n", (new piScope::MHLocation(-99.9999999,-270.23456789,-987.654321))->ToString());
	fprintf(stdout, "\tLocation:\t%s\n", (new piScope::MHLocation(-79.9999999,-90.23456789,-987.654321))->ToString());
	fprintf(stdout, "\tLocation:\t%s\n", (new piScope::MHLocation(99.9999999,270.23456789,987.654321))->ToString());
	fprintf(stdout, "\tLocation:\t%s\n", (new piScope::MHLocation(79.9999999,90.23456789,987.654321))->ToString());

	//	test time stamp
	piScope::MHAstroTime now(1,&here);
	fprintf(stdout, "\tTimeStamp:\t%s\n", now.ToString());
	fprintf(stdout, "\tTimeStamp:\t%s\n", now.ToString(-1));
	fprintf(stdout, "\tTimeStamp:\t%s\n", now.ToString(1));
	fprintf(stdout, "\tTimeStamp:\t%s\n", now.ToString(0));
	fprintf(stdout, "\tJulianDate:\t%f\n", now.GetJulianDate());
	fprintf(stdout, "\tModified JulianDate:\t%f\n", now.GetJulianDate(true));

	//	test 3D vector
	piScope::MHVector3D testloc = *here.ToVector();
	fprintf(stdout, "\tTESTLOCATION:\t%s\n", testloc.ToString());
	piScope::MHVector3D* ecef = testloc.Convert2ECEF();
	fprintf(stdout, "\tTESTLOCATION:\t%s\n", ecef->ToString());

	//	exit
	return(0);
}

int test_rtimulib(int argc, char* argv[], char* envp[])
{
	//	parameters may be unused
	//(void)argc;
	//(void)argv;
	(void)envp;

	piScope::MHTelescope scope("myScope");
	int loglevel = 6;
	const char* logfile = "myScope.log";
	for(int pos = 1; argc > pos; ++pos)
	{
		if(0 == strncmp(argv[pos],"--debug=",8))
		{
			sscanf(argv[pos] +8, "%d", &loglevel);
		}
		else if(0 == strncmp(argv[pos],"--logfile=",10))
		{
			logfile = (argv[pos] +10);
		}
	}
	scope.SetLogLevel(loglevel);
	scope.SetLogFile(logfile);
	scope.SetLocation( TESTLOCATION );
	fprintf(stdout, "Telescope:\t%s\n", scope.ToString());

	keep_running = scope.InitIMUSensor();
	if(!keep_running)
	{
		scope.printLog(0,"scope:\tInitIMUSensor failed\n");
		return(1);
	}

	scope.IMUpthread_start();
	while(keep_running)
	{
		piScope::MHAstroVector* ori = scope.GetOrientation();
		double RA,DEC;
		scope.GetOrientation(&RA, &DEC);
		if(NULL != ori && ori->Validate())
		{
			scope.printLog(3,"IMU:\torientation %s [%f,%f]\n", ori->ToString(), RA, DEC);
		}
		sleep(1);
	}

	//	exit
	return(0);
}

int main(int argc, char* argv[], char* envp[])
{
	//	parameters may be unused
	(void)argc;
	(void)envp;
	//	programm greeting
	std::cout << argv[0] << "\t" << "(build " << __DATE__ << ")" << std::endl;

	// prepare signal handling
	{
#		ifdef WIN32
		if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)signal_handler, TRUE))
		{
			std::cerr << "SetConsoleCtrlHandler failed" << std::endl;
			return 127;
		}
#		else
		//	ignore SIGPIPE
		signal(SIGPIPE, SIG_IGN);
		signal(SIGINT, signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGQUIT, signal_handler);
		//	?keep running after SIGHUP
		//signal(SIGHUP,signal_handler);
#		endif
	}

#	if defined(__TEST_I2CSENSOR__)
		test_i2csensor(argc, argv, envp);
#	elif defined(__TEST_VECTOR__)
		test_vector(argc, argv, envp);
#	elif defined(__TEST_RTIMULIB__)
		test_rtimulib(argc, argv, envp);
#	else
	for(int pos = 1; argc > pos; ++pos)
	{
		if(0 != strncmp(argv[pos],"--test",6))
		{
			//	not a program call
		}
		else if(NULL != strstr(argv[pos],"i2csensor"))
		{
			test_i2csensor(argc, argv, envp);
		}
		else if(NULL != strstr(argv[pos],"vector"))
		{
			test_vector(argc, argv, envp);
		}
		else if(NULL != strstr(argv[pos],"rtimulib"))
		{
			test_rtimulib(argc, argv, envp);
		}
	}
#	endif // defined(__TEST_I2CSENSOR__) || defined(__TEST_VECTOR__) || defined(__TEST_RTIMULIB__)

	//	done
	fprintf(stdout, "Bye.\n");
	return(0);
}
