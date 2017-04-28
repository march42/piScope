/*	test-programs.cpp
**	combined test routines and programs
**	use -D __TEST_I2CSENSOR__ or -D __TEST_VECTOR__ while compiling
*/

#if defined(__TEST_I2CSENSOR__)
#	include "I2Csensor.hpp"
#	include "IMU.hpp"
	using namespace rpiScope;
#elif defined(__TEST_VECTOR__)
#	include "AstroVector.hpp"
	using namespace piScope;
#else
#	error "NO TARGET SPECIFIED FOR COMPILING: " __FILE__
#endif // defined(__TEST_I2CSENSOR__)

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

int main(int argc, char* argv[], char* envp[])
{
	//	programm greeting
	std::cout << argv[0] << "\t" << "(build " << __DATE__ << ")" << std::endl;

	// prepare signal handling
#	ifdef WIN32
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)signal_handler, TRUE))
	{
		std::cerr << "SetConsoleCtrlHandler failed" << std::endl;
		return 127;
	}
#	else
	//	ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	//	?keep running after SIGHUP
	//signal(SIGHUP,signal_handler);
#	endif

#if defined(__TEST_I2CSENSOR__)
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

#elif defined(__TEST_VECTOR__)
	//	unused parameters
	(void)argc;
	(void)envp;

	//	programm greeting
	std::fprintf(stdout, "%s\t(build %s)\n", argv[0], __DATE__);

	//	test Location (49.964608N 9.146783E)
	double lat = 49.964608;
	double lon = 9.146783;
	double height = 160;
	Location here(lat,lon,height,"Aschaffenburg");
	std::fprintf(stdout, "\tLocation:\t%s\n", here.ToString());
	std::fprintf(stdout, "\tLocation:\t%s\n", (new Location(-23.9999999999,-89.123456789,987.654321))->ToString());
	std::fprintf(stdout, "\tLocation:\t%s\n", (new Location(-99.9999999,-270.23456789,-987.654321))->ToString());
	std::fprintf(stdout, "\tLocation:\t%s\n", (new Location(-79.9999999,-90.23456789,-987.654321))->ToString());
	std::fprintf(stdout, "\tLocation:\t%s\n", (new Location(99.9999999,270.23456789,987.654321))->ToString());
	std::fprintf(stdout, "\tLocation:\t%s\n", (new Location(79.9999999,90.23456789,987.654321))->ToString());

	//	test time stamp
	AstroTime now(1,&here);
	std::fprintf(stdout, "\tTimeStamp:\t%s\n", now.ToString());
	std::fprintf(stdout, "\tTimeStamp:\t%s\n", now.ToString(-1));
	std::fprintf(stdout, "\tTimeStamp:\t%s\n", now.ToString(1));
	std::fprintf(stdout, "\tTimeStamp:\t%s\n", now.ToString(0));
	std::fprintf(stdout, "\tJulianDate:\t%f\n", now.GetJulianDate());
	std::fprintf(stdout, "\tModified JulianDate:\t%f\n", now.GetJulianDate(true));

	//	test 3D vector
	Vector3D aburg = *here.ToVector();
	std::fprintf(stdout, "\tAschaffenburg:\t%s\n", aburg.ToString());
	Vector3D* ecef = aburg.Convert2ECEF();
	std::fprintf(stdout, "\tAschaffenburg:\t%s\n", ecef->ToString());
#endif // defined(__TEST_I2CSENSOR__)

	//	done
	std::fprintf(stdout, "Bye.\n");
	return(0);
}
