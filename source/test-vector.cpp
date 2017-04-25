/*
**
*/

#include "AstroVector.hpp"

#include <unistd.h>
#include <cstdio>

using namespace piScope;

int main(int argc, char* argv[], char* envp[])
{
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

	//	done
	std::fprintf(stdout, "Bye.\n");
	return(0);
}
