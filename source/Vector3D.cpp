/*
**	Vector3D (.hpp/.cpp)
**	handling class for 3D vector
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

#include "Vector3D.hpp"
#include "MACROS.h"

//#include <unistd.h>
#include <cassert>
#include <cstdio>
//#include <cstdlib>
//#include <cstring>
#include <cmath>
//#include <climits>

namespace piScope
{

	MHVector3D::MHVector3D(MHVectorType_t vecType, double vecX, double vecY, double vecZ, double vecLen)
			: Type(vecType), X(vecX), Y(vecY), Z(vecZ), Length(vecLen)
	{
	}
	MHVector3D::MHVector3D(MHVector3D* vec)
	{
		this->Type = vec->Type;
		this->X = vec->X;
		this->Y = vec->Y;
		this->Z = vec->Z;
		this->Length = vec->Length;
	}
	MHVector3D::~MHVector3D()
	{
	}

	MHVector3D* MHVector3D::Set(MHVectorType_t vecType, double vecX, double vecY, double vecZ, double vecLen)
	{
		this->Type = vecType;
		this->X = vecX;
		this->Y = vecY;
		this->Z = vecZ;
		this->Length = vecLen;
		return(this);
	}
	MHVector3D* MHVector3D::Set(double vecX, double vecY, double vecZ, double vecLen)
	{
		this->X = vecX;
		this->Y = vecY;
		this->Z = vecZ;
		this->Length = vecLen;
		return(this);
	}

	bool MHVector3D::Validate(bool checkonly)
	{
		bool invalid = false;
		switch(this->Type)
		{
		case VectorType_INVALID:
			invalid = true;
			#if !defined(NDEBUG)
				fprintf(stderr, "debug:\t%s:\t%s\n", "Vector3D::Validate", "VectorType==INVALID");
			#endif
			break;

		case VectorType_LATLON:
			// check given latitude to +-90 and longitude to +-180
			invalid |= (-90 > this->X || 90 < this->X);
			invalid |= (-180 > this->Y || 180 < this->Y);
			#if !defined(NDEBUG)
				if(invalid)
				{
					fprintf(stderr, "debug:\t%s:\tinvalid LATLON %f,%f\n", "Vector3D::Validate", this->X,this->Y);
				}
			#endif
			if(invalid && !checkonly)
			{
				this->FixLatLon();
			}
			break;

		case VectorType_AziElevTilt:
			//	rotation only, so length==0
			invalid |= (0 != this->Length);
			invalid |= M_PI < fabs(this->X);
			invalid |= M_PI < fabs(this->Y);
			invalid |= M_PI < fabs(this->Z);
			if(invalid && !checkonly)
			{
				this->Length = 0.0;
				while(M_PI < fabs(this->X))
				{
					this->X += (0<this->X ?-2 :+2) * M_PI;
				}
				while(M_PI < fabs(this->Y))
				{
					this->Y += (0<this->Y ?-2 :+2) * M_PI;
				}
				while(M_PI < fabs(this->Z))
				{
					this->Z += (0<this->Z ?-2 :+2) * M_PI;
				}
			}
			break;

		case VectorType_LocalENU:
		case VectorType_LocalNED:
		case VectorType_LocalRPY:
			//	rotation only, so length==0
			invalid |= (0 != this->Length);
			if(invalid && !checkonly)
			{
				this->Length = 0.0;
			}
			break;

		case VectorType_ECEF:
			//	checking to be implemented
		case VectorType_J2000:
			//	checking to be implemented
			#if !defined(NDEBUG)
				fprintf(stderr, "debug:\t%s:\t%s\n", "Vector3D::Validate", "VectorType checking, is not implemented");
			#endif

		case VectorType_3DONLY:	//	no further checking
		default:
			break;
		}
		return(!invalid);	// negate result
	}
	bool MHVector3D::FixLatLon(double* lat, double* lon)
	{
		if(NULL == lat && NULL == lon && VectorType_LATLON != this->Type)
		{
			return(-1);
		}
		double LAT = (NULL==lat ?this->X :*lat);
		double LON = (NULL==lon ?this->Y :*lon);
		int fullrotation;
		if(-90 > LAT || 90 < LAT)
		{
			//	just remove full rotations
			if(0 != (fullrotation = LAT / 360))
			{
				//	400 will give 1, -400 => -1, -1234 => -3
				LAT -= (360 * fullrotation);
			}
			//	rot<360 needs flipping on zenith crossing
			/*	given values
			**	0<=a<=90	-0, PRETTY
			**	90<a<180	-90, flip lon-180
			**	180<=a<270	-180, flip lon-180
			**	270<a<360	-360
			*/
			if(90 >= LAT && -90 <= LAT)
			{
				//	PRETTY
			}
			else if(270 <= LAT)
			{
				LAT -= 360;
			}
			else if(-270 >= LAT)
			{
				LAT += 360;
			}
			else //if((90 < LAT && 270 > LAT) || (-270 < LAT && -90 > LAT))
			{
				//	90<x<270 needs flip -180, -90>x>-270 needs flip +180
				//	LON flip <0 +180, >0 -180
				LAT += (0<LAT ?-180 :+180);	//	flip LAT
				LON += (0<LON ?-180 :+180);	//	flip LON
			}

		}
		if(180 < LON || -180 > LON)
		{
			if(0 != (fullrotation = LON / 360))
			{
				LON -= (360.0L * fullrotation);	// -400 => -360 = -40, PRETTY
			}
			/*	given values
			**	0<=y<=180	PRETTY
			**	180<y<360	-360
			*/
			if(180 < LON)
			{
				LON -= 360;
			}
			else //if(-180 > LON)
			{
				LON += 360;
			}
		}
		//	just to be sure
		assert(90 >= LAT && -90 <= LAT);
		assert(180 >= LON && -180 <= LON);
		//	store values, if changed
		if((NULL == lat && NULL == lon) && (LAT != this->X || LON != this->Y))
		{
			#if !defined(NDEBUG)
				fprintf(stderr, "debug:\t%s:\tfixing to %f,%f\n", "Vector3D::FixLatLon", LAT,LON);
			#endif
			this->X = LAT;
			this->Y = LON;
			return(true);	//	values have changed
		}
		else
		{
			if(NULL != lat)
			{
				*lat = LAT;
			}
			if(NULL != lon)
			{
				*lon = LON;
			}
		}
		//	nothing changed
		return(false);
	}

	MHVector3D* MHVector3D::Convert2ECEF(MHVector3D* convertVector)
	{
		MHVector3D* source = (NULL==convertVector ?this :convertVector);
		MHVector3D* destination = (NULL==convertVector ?(new MHVector3D) :convertVector);
		//	convert to ECEF
		if(VectorType_LATLON == source->Type)
		{
			//	ECEF-r = EMR + height
			//	ECEF-Z = sin(latitude)
			//	ECEF-r' = cos(latitude)
			//	ECEF-X = r' * cos(longitude)
			//	ECEF-Y = r' * sin(longitude)
			double latlonX = cos(DEG2RAD(source->X)) * cos(DEG2RAD(source->Y));
			double latlonY = cos(DEG2RAD(source->X)) * sin(DEG2RAD(source->Y));
			double latlonZ = sin(DEG2RAD(source->X));
			#if !defined(NDEBUG)
				fprintf(stderr, "debug:\tconverting LATLON=[%f,%f,%f] to ECEF=[%f,%f,%f]\n"
					, source->X, source->Y, source->Z
					, latlonX, latlonY, latlonZ);
				fprintf(stderr, "debug:\tsin(lat)=%f, cos(lat)=%f, sin(lon)=%f, cos(lon)=%f\n"
					, sin(DEG2RAD(source->X)), cos(DEG2RAD(source->X))
					, sin(DEG2RAD(source->Y)), cos(DEG2RAD(source->Y)));
			#endif
			//	prepare destination vector
			destination->Length = EARTH_MEANRADIUS + source->Z;
			destination->X = latlonX;
			destination->Y = latlonY;
			destination->Z = latlonZ;
			destination->Type = VectorType_ECEF;
		}
		/*	no accurate conversion possible, just needs setting of base reference center/fix
		else if(VectorType_3DONLY >= source->VectorType)
		{
		}
		*/
		/*	no conversion possible without known location
		else if(VectorType_LocalENU == source->Type)
		{
		}
		else if(VectorType_LocalNED == source->Type)
		{
		}
		*/
		/*	conversion impossible without time stamp and center/fix
		else if(VectorType_J2000 == source->Type)
		{
			//	calculate Earth's rotation since J2000 epoch
			//	apply ecliptic rotation
		}
		*/
		assert(VectorType_ECEF == destination->Type);
		return(destination);
	}

	const char* MHVector3D::ToString(void) const
	{
		static char buffer[50] = {'\0'};
		const char* type[] = { "3D","ECEF","LATLON","Local ENU","Local NED","Local RPY","Azi/Elev/Tilt","J2000" };
		if(VectorType_LATLON == this->Type)
		{
			//	clean values
			double lat = this->X, latD,latM,latS;
			double lon = this->Y, lonD,lonM,lonS;
			((MHVector3D*)this)->FixLatLon(&lat,&lon);
			//	convert
			latM = modf(lat, &latD);
			latS = modf((latM * 60), &latM);
			latS *= 60;
			lonM = modf(lon, &lonD);
			lonS = modf((lonM * 60), &lonM);
			lonS *= 60;
			//	print to buffer
			snprintf(&buffer[0], sizeof(buffer), "%s [%02d%s%02d\'%f\",%02d%s%02d\'%f\",%fm]", type[this->Type]
				, (0>latD ?-1 :1) * (int)latD, (0>latD ?"S" :"N"), (int)latM, latS
				, (0>lonD ?-1 :1) * (int)lonD, (0>lonD ?"W" :"E"), (int)lonM, lonS
				, this->Z);
		}
		else if(VectorType_LocalRPY == this->Type)
		{
			double roll = RAD2DEG(this->X);
			double pitch = RAD2DEG(this->Y);
			double yaw = RAD2DEG(this->Z);
			//	print to buffer
			snprintf(&buffer[0], sizeof(buffer), "%s [%f,%f,%f]", type[this->Type], roll,pitch,yaw);
		}
		else if(VectorType_LocalNED == this->Type)
		{
			double roll = RAD2DEG(this->X);
			double pitch = RAD2DEG(this->Y);
			double yaw = RAD2DEG(this->Z);
			//	print to buffer
			snprintf(&buffer[0], sizeof(buffer), "%s [%f,%f,%f]", type[this->Type], roll,pitch,yaw);
		}
		else if(VectorType_AziElevTilt == this->Type)
		{
			double azi = RAD2DEG(this->X);
			double elev = RAD2DEG(this->Y);
			double tilt = RAD2DEG(this->Z);
			//	print to buffer
			snprintf(&buffer[0], sizeof(buffer), "%s [%f,%f,%f]", type[this->Type], azi,elev,tilt);
		}
		else if(0 > this->Type || (char)sizeof(type) <= this->Type)
		{
			snprintf(&buffer[0],sizeof(buffer), "INVALID [%f,%f,%f] l=%f"
				, this->X,this->Y,this->Z, this->Length);
		}
		else
		{
			snprintf(&buffer[0],sizeof(buffer), "%s [%f,%f,%f] l=%f", type[this->Type]
				, this->X,this->Y,this->Z, this->Length);
		}
		return(&buffer[0]);
	}

	double MHVector3D::GetX(void) const
	{
		return(this->X);
	}
	double MHVector3D::GetOffsetX(double value) const
	{
		return((this->X - value) / value);
	}
	double MHVector3D::GetY(void) const
	{
		return(this->Y);
	}
	double MHVector3D::GetOffsetY(double value) const
	{
		return((this->Y - value) / value);
	}
	double MHVector3D::GetZ(void) const
	{
		return(this->Z);
	}
	double MHVector3D::GetOffsetZ(double value) const
	{
		return((this->Z - value) / value);
	}
	MHVectorType_t MHVector3D::GetType(void) const
	{
		return(this->Type);
	}

	const char* Angle_Deg2HMS(double angle, double* H, double* M, double* S)
	{
		static char value[30] = {'\0'};
		size_t pos = 0;
		value[pos] = '\0';
		//	bring to 0<=angle<=360
		while(0 > angle)	angle += 360;
		while(360 < angle)	angle -= 360;
		//	convert
		//double hour = (angle * (24/360));
		//double minute = ((angle * 60) % 60);
		//double second = remainder((angle * 3600), 60);
		double hour, minute, second;
		minute = modf(angle, &hour);
		second = modf((minute * 60), &minute);
		second *= 60;
		//	print to buffer
		pos += snprintf(&value[pos], sizeof(value) -pos, "%02dH%02dM%f", (int)hour, (int)minute, second);
		//	copy to return variables
		/*	H,M,S	values
		**	0,0,0	none
		**	0,0,1	S = h*3600 + m*60 + s
		**	0,1,0	M = h*60 + m + s/60
		**	0,1,1	M = h*60 + m, S=s
		**	1,0,0	H = h + m/60 + s/3600
		**	1,0,1	H=h, S = m*60 + s
		**	1,1,0	H=h, M = m + s/60
		**	1,1,1	H=h, M=m, S=s
		*/
		if(NULL != H)
		{
			*H = hour + (NULL==M ?((minute + (NULL==S ?(second /60) :0)) /60) :0);
		}
		if(NULL != M)
		{
			*M = (NULL==H ?(hour *60) :0) + minute + (NULL==S ?(second /60) :0);
		}
		if(NULL != S)
		{
			*S = (NULL==M ?((minute + (NULL==H ?(hour *60) :0)) *60) :0) + second;
		}
		//	done
		return(&value[0]);
	}
	const char* Angle_Rad2HMS(double angle, double* H, double* M, double* S)
	{
		return(Angle_Deg2HMS(RAD2DEG(angle), H,M,S));
	}
	const char* Angle_Deg2DMS(double angle, double* D, double* M, double* S)
	{
		static char value[30] = {'\0'};
		size_t pos = 0;
		value[pos] = '\0';
		//	bring to 0<=angle<=360
		while(0 > angle)	angle += 360;
		while(360 < angle)	angle -= 360;
		//	convert
		double degree, minute, second;
		minute = modf(angle, &degree);
		second = modf((minute * 60), &minute);
		second *= 60;
		//	print to buffer
		pos += snprintf(&value[pos], sizeof(value) -pos, "%02d° %02d' %f\"", (int)degree, (int)minute, second);
		//	copy to return variables
		if(NULL != D)
		{
			*D = degree + (NULL==M ?((minute + (NULL==S ?(second /60) :0)) /60) :0);
		}
		if(NULL != M)
		{
			*M = (NULL==D ?(degree *60) :0) + minute + (NULL==S ?(second /60) :0);
		}
		if(NULL != S)
		{
			*S = (NULL==M ?((minute + (NULL==D ?(degree *60) :0)) *60) :0) + second;
		}
		//	done
		return(&value[0]);
	}
	const char* Angle_Rad2DMS(double angle, double* D, double* M, double* S)
	{
		return(Angle_Deg2DMS(RAD2DEG(angle), D,M,S));
	}

};
