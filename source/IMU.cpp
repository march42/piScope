/*	IMU
 *	handling of IMU (inertial measurement unit)
 */

#include "MACROS.h"
#include "IMU.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <cmath>
#include <climits>

#if defined(USE_MADGWICK_AHRS)
	namespace Madgwick { extern "C" {
#		include "../MadgwickAHRS/MadgwickAHRS.h"
	}};
#endif // defined

using namespace std;
namespace rpiScope
{
/*
#		ifdef DEBUG
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cdevice", "constructor begin", "");
#		endif
*/

/*	basics
	accelerometer
		To get easily the orientation of a non-moving object (pitch and roll), a 3-axis accelerometer (how does it work?) can be used.
		For a static object, it gives the value of the gravity field on 3 axes, therefore its direction. And since it always points to
		the center of the earth, we can therefore know how the accelerometer is inclined with the help of man's best friend, trigonometry.
	gyroscope
		The most common used sensor in quadcopter control boards is the gyroscope sensor. It  gives the angular rate around the 3 axes of
		space in deg/s, so, as for the accelerometer, some simple maths are needed to compute the actual angle by integration.

	sensor sombinations
		The idea of the complementary filter is the following : the filtered accelerometer value of the angle  is not subject to drift,
		so we use it to "correct" the value given by the gyroscope(more precise and less subject to vibrations noises). How do we do that?
		We combine the two values like so:
			Angle_{accurate} = (GyroPercentage) * Angle_{Gyro} +(1-GyroPercentage)* Angle_{Accel}
		GyroPercentage is just a floating value between 0 and 1. It is typically ranged from 0.9 to almost 1, depending on how much you
		can trust your gyroscope and accelerometer.

		On the yaw axis, the same complementary filter is used  with a compass (which gives the direction of the earth's magnetic field),
		giving us the 3D orientation of the quadcopter like so:
			Yaw_{accurate} = (GyroPercentage) * Yaw_{Gyro} +(1-GyroPercentage)* Yaw_{Compass}

	Gieren (engl. yaw):	Drehung um die Z-Achse des Referenzsystems (Gier-, Hoch- oder Vertikalachse).
		Für den Richtungswinkel werden dabei mitunter auch die Bezeichnungen heading oder Azimut gebraucht.
	Nicken (engl. pitch, selten auch nick): Drehung um die Y-Achse des Fahrzeugs (Nick- oder Querachse).
	Rollen (engl. roll): Drehung um die in Längsrichtung des Fahrzeugs verlaufende X-Achse (Roll-, Wank- oder Längsachse).
		Für den Querneigungswinkel wird dabei auch die Bezeichnung banking gebraucht.

	ZY′X″-Konvention
		Die Rotationen beziehen sich immer auf die zunächst raumfeste rechtshändige Orthogonalbasis.
		Die Konvention der Orientierung der Winkel folgt der in der Mathematik üblichen Rechte-Hand-Regel, ein positiver Winkel zeigt also
		entgegen dem Uhrzeigersinn. Alle weiteren Rotationsachsen ergeben sich dynamisch gemäß der definierten Reihenfolge der Rotationen.

	full scale factor - get values from chip datasheet
		16bit signed integer = -32768/+32767
		gyroscope	+/- 245 dps = 245/32768
		accelerometer	+/- 2 g = 2/32768
		magnetometer	+/- 4 gauss = 4/32768
*/

	IMU_Vector::IMU_Vector()
	{
		this->X = 1.0;
		this->Y = 0.0;
		this->Z = 0.0;
		this->FullScale = 1.0;
		this->Length = 1.0;
	};
	IMU_Vector::IMU_Vector(IMU_Vector* value)
	{
		this->X = value->X;
		this->Y = value->Y;
		this->Z = value->Z;
		this->FullScale = value->FullScale;
		this->Length = value->Length;
	};
	IMU_Vector::IMU_Vector(double valX, double valY, double valZ, double valScale)
	{
		this->X = valX;
		this->Y = valY;
		this->Z = valZ;
		this->FullScale = (0.0 == valScale) ?1.0 :valScale;
		this->Length = 1.0;
	};
	IMU_Vector::IMU_Vector(int16_t valX, int16_t valY, int16_t valZ, double valScale)
	{
		this->X = valX *1.0;
		this->Y = valY *1.0;
		this->Z = valZ *1.0;
		this->FullScale = (0.0 == valScale) ?1.0 :valScale;
		this->Length = 1.0;
	};
	IMU_Vector::IMU_Vector(int32_t valX, int32_t valY, int32_t valZ, double valScale)
	{
		this->X = valX *1.0;
		this->Y = valY *1.0;
		this->Z = valZ *1.0;
		this->FullScale = (0.0 == valScale) ?1.0 :valScale;
		this->Length = 1.0;
	};
	IMU_Vector& IMU_Vector::set(double valX, double valY, double valZ, double valScale)
	{
		this->X = valX;
		this->Y = valY;
		this->Z = valZ;
		if(0.0 != valScale)
		{
			this->FullScale = valScale;
		}
		this->Length = 1.0;
		return(*this);
	};
	IMU_Vector& IMU_Vector::set(int16_t valX, int16_t valY, int16_t valZ, double valScale)
	{
		this->X = valX *1.0;
		this->Y = valY *1.0;
		this->Z = valZ *1.0;
		if(0.0 != valScale)
		{
			this->FullScale = valScale;
		}
		this->Length = 1.0;
		return(*this);
	};
	IMU_Vector& IMU_Vector::set(int32_t valX, int32_t valY, int32_t valZ, double valScale)
	{
		this->X = valX *1.0;
		this->Y = valY *1.0;
		this->Z = valZ *1.0;
		if(0.0 != valScale)
		{
			this->FullScale = valScale;
		}
		this->Length = 1.0;
		return(*this);
	};
	double IMU_Vector::scaledX(void)
	{
		return(this->X * this->FullScale);
	}
	double IMU_Vector::scaledY(void)
	{
		return(this->Y * this->FullScale);
	}
	double IMU_Vector::scaledZ(void)
	{
		return(this->Z * this->FullScale);
	}
	IMU_Vector& IMU_Vector::Normalize(void)
	{
		this->Length = std::sqrt( std::pow(this->X,2) + std::pow(this->Y,2) + std::pow(this->Z,2) );
		if(0 != this->Length)
		{
			this->X /= this->Length;
			this->Y /= this->Length;
			this->Z /= this->Length;
		}
		return(*this);
	}
	IMU_Vector* IMU_Vector::ToEuler(void)
	{
		IMU_Vector* value = new IMU_Vector(this);
		//	normalize vector
		value->Normalize();
		//	compute Euler angle
		double EulerX = std::atan2( value->Y, value->Z );
		double EulerY = -std::atan2( value->X, std::sqrt( std::pow(value->Y,2) + std::pow(value->Z,2) ) );
#		if !defined(NDEBUG)
		printf("\t%s\t%f,%f\t%f,%f\n", "IMU_Vector::ToEuler", EulerX, EulerY, value->Length, value->FullScale);
#		endif
		//	set vector to temporary values
		value->X = EulerX;
		value->Y = EulerY;
		value->Z = 0.0;
		value->FullScale = 1.0;
		value->Length = 1.0;
		return(value);
	}

	IMU_Data::IMU_Data()
	{
		this->LPF_MaxValues = 32;
		this->FullScale = 1.0;
		this->InitMutex();
	}
	IMU_Data::~IMU_Data()
	{
#		if defined(DEBUG4)
		printf("\t%s\t%s\t%s\n", "IMU_Data", "destructor", "begin");
#		endif
		this->DestroyMutex();
#		if defined(DEBUG4)
		printf("\t%s\t%s\t%s\n", "IMU_Data", "destructor", "done");
#		endif
	}
	void IMU_Data::LPF_resize(size_t LPFValues)
	{
		if(32 < LPFValues)
			this->LPF_MaxValues = LPFValues;
		else if(16 < LPFValues)
			this->LPF_MaxValues = 32;
		else if(8 < LPFValues)
			this->LPF_MaxValues = 16;
		else if(4 < LPFValues)
			this->LPF_MaxValues = 8;
		else if(1 < LPFValues)
			this->LPF_MaxValues = 4;
		else
			this->LPF_MaxValues = 1;
	}
	void IMU_Data::InitMutex(void)
	{
		//	init mutex
		pthread_mutex_init(&this->pthread_mutex, NULL);
	}
	void IMU_Data::DestroyMutex(void)
	{
#		if defined(DEBUG4)
		printf("\t%s\t%s\t%s\n", "IMU_Data", "DestroyMutex", "");
#		endif
		//	destroy mutex
		pthread_mutex_lock(&this->pthread_mutex);
		pthread_mutex_destroy(&this->pthread_mutex);
	}
	IMU_Vector*	IMU_Data::vector(void)
	{
		//	calculate average
		double X = 0;
		double Y = 0;
		double Z = 0;
		pthread_mutex_lock(&this->pthread_mutex);
		size_t count = this->LPF_Values.size();
		if(0 < count)
		{
			for(size_t pos=0; pos<count; ++pos)
			{
				X += this->LPF_Values[pos].X;
				Y += this->LPF_Values[pos].Y;
				Z += this->LPF_Values[pos].Z;
			}
			X /= count;
			Y /= count;
			Z /= count;
		}
		pthread_mutex_unlock(&this->pthread_mutex);
		IMU_Vector* value = new IMU_Vector(X, Y, Z, this->FullScale);
		return(value);
	}
	int16_t IMU_Data::rawX(void)
	{
		return(this->LPF_Values.empty() ?0 :this->LPF_Values.back().X);
	}
	double IMU_Data::scaledX(void)
	{
		return(this->LPF_Values.empty() ?0.0 :(this->LPF_Values.back().X * this->FullScale));
	}
	int16_t IMU_Data::rawY(void)
	{
		return(this->LPF_Values.empty() ?0 :this->LPF_Values.back().Y);
	}
	double IMU_Data::scaledY(void)
	{
		return(this->LPF_Values.empty() ?0.0 :(this->LPF_Values.back().Y * this->FullScale));
	}
	int16_t IMU_Data::rawZ(void)
	{
		return(this->LPF_Values.empty() ?0 :this->LPF_Values.back().Z);
	}
	double IMU_Data::scaledZ(void)
	{
		return(this->LPF_Values.empty() ?0.0 :(this->LPF_Values.back().Z * this->FullScale));
	}
	void	IMU_Data::Push(int16_t X, int16_t Y, int16_t Z)
	{
		IMU_Vector value(X,Y,Z);
		pthread_mutex_lock(&this->pthread_mutex);
		this->LPF_Values.push_back(value);
		while(this->LPF_MaxValues < this->LPF_Values.size())
		{
			this->LPF_Values.pop_front();
		}
		pthread_mutex_unlock(&this->pthread_mutex);
	}

	IMU_MARGdata::IMU_MARGdata(size_t LPFValues)
	{
		this->LPF_resize(LPFValues);
	}
	IMU_MARGdata::~IMU_MARGdata()
	{
#		if defined(DEBUG4)
		printf("\t%s\t%s\t%s\n", "IMU_MARGdata", "destructor", "");
#		endif
	}

	void IMU_MARGdata::LPF_resize(size_t LPFValues)
	{
		size_t LPF_MaxValues = 1;
		if(32 < LPFValues)
			LPF_MaxValues = LPFValues;
		else if(16 < LPFValues)
			LPF_MaxValues = 32;
		else if(8 < LPFValues)
			LPF_MaxValues = 16;
		else if(4 < LPFValues)
			LPF_MaxValues = 8;
		else if(1 < LPFValues)
			LPF_MaxValues = 4;
		//	set to childs
		this->DataMagnetometer.LPF_resize(LPF_MaxValues);
		this->DataAcceleration.LPF_resize(LPF_MaxValues);
		this->DataGyroscope.LPF_resize(LPF_MaxValues);
	}

	IMU_Vector* IMU_MARGdata::Magnetometer(void)
	{
		return(this->DataMagnetometer.vector());
	}
	IMU_Vector* IMU_MARGdata::Acceleration(void)
	{
		return(this->DataAcceleration.vector());
	}
	IMU_Vector* IMU_MARGdata::Gyroscope(void)
	{
		return(this->DataGyroscope.vector());
	}

	void IMU_MARGdata::PushMagnetometer(int16_t X, int16_t Y, int16_t Z)
	{
		this->DataMagnetometer.Push(X,Y,Z);
	}
	void IMU_MARGdata::PushAcceleration(int16_t X, int16_t Y, int16_t Z)
	{
		this->DataAcceleration.Push(X,Y,Z);
	}
	void IMU_MARGdata::PushGyroscope(int16_t X, int16_t Y, int16_t Z)
	{
		this->DataGyroscope.Push(X,Y,Z);
	}
	void IMU_MARGdata::SetFullScale(double gyro, double acc, double mag)
	{
		this->DataGyroscope.FullScale = gyro;	//dps (degrees/sec)
		this->DataAcceleration.FullScale = acc;	//g (1g = 9,8 m/s^2 earth gravity)
		this->DataMagnetometer.FullScale = mag;	//gauss
	}

	void IMU_MARGdata::MadgwickAHRSupdate(void)
	{
#		if defined(USE_MADGWICK_AHRS)
		//	need gyro data in radian per second (not dps) =*PI/180
		double gyroscale = DEG2RAD(this->DataGyroscope.FullScale);
		double gx = gyroscale * this->DataGyroscope.rawX();
		double gy = gyroscale * this->DataGyroscope.rawY();
		double gz = gyroscale * this->DataGyroscope.rawZ();
		double accscale = this->DataAcceleration.FullScale;
		double ax = accscale * this->DataAcceleration.rawX();
		double ay = accscale * this->DataAcceleration.rawY();
		double az = accscale * this->DataAcceleration.rawZ();
		double magscale = this->DataMagnetometer.FullScale;
		double mx = magscale * this->DataMagnetometer.rawX();
		double my = magscale * this->DataMagnetometer.rawY();
		double mz = magscale * this->DataMagnetometer.rawZ();
		if(0.1 < (gx+gy+gz))
		{
			//	2dps=0.035rps, 10dps=0.175rps
			Madgwick::beta = 0.001;
		}
		else //if(0.1 < (gx+gy+gz))
		{
			Madgwick::beta = 0.1;
		}
		Madgwick::MadgwickAHRSupdate(gx,gy,gz, ax,ay,az, mx,my,mz );
#		endif
	}

	IMU_Vector* IMU_MARGdata::Orientation(void)
	{
#		if defined(USE_MADGWICK_AHRS)
		//	q0,q1,q2,q3 orientation quaternion
		//float getPitch(){return atan2f(2.0f * q2 * q3 - 2.0f * q0 * q1, 2.0f * q0 * q0 + 2.0f * q3 * q3 - 1.0f);};
		//float getRoll(){return -1.0f * asinf(2.0f * q1 * q3 + 2.0f * q0 * q2);};
		//float getYaw(){return atan2f(2.0f * q1 * q2 - 2.0f * q0 * q3, 2.0f * q0 * q0 + 2.0f * q1 * q1 - 1.0f);};
		double roll = std::atan2(Madgwick::q0*Madgwick::q1 + Madgwick::q2*Madgwick::q3, 0.5f - Madgwick::q1*Madgwick::q1 - Madgwick::q2*Madgwick::q2);
		double pitch = std::asin(-2.0f * (Madgwick::q1*Madgwick::q3 - Madgwick::q0*Madgwick::q2));
		double yaw = std::atan2(Madgwick::q1*Madgwick::q2 + Madgwick::q0*Madgwick::q3, 0.5f - Madgwick::q2*Madgwick::q2 - Madgwick::q3*Madgwick::q3);
		roll *= (180.0L / M_PI);
		pitch *= (180.0L / M_PI);
		yaw *= (180.0L / M_PI);
		return(new IMU_Vector(roll,pitch,yaw));
#		else
		//	get sensor data and normalize
		IMU_Vector* gyro = this->Gyroscope();
		gyro->Normalize();
		IMU_Vector* acc = this->Acceleration();
		acc->Normalize();
		//	calculate
		double gyroFactor = 0.95L;
		double valX = (acc->X *(1.0-gyroFactor)) + (gyro->X *gyroFactor);
		double valY = (acc->Y *(1.0-gyroFactor)) + (gyro->Y *gyroFactor);
		double valZ = (acc->Z *(1.0-gyroFactor)) + (gyro->Z *gyroFactor);
		double EulerX = atan2( valY, valZ );
		double EulerY = -atan2( valX, sqrt( (valY*valY) + (valZ*valZ) ) );
		IMU_Vector* value = new IMU_Vector(EulerX,EulerY,0.0, (180/M_PI) );
		//	free buffers
		delete(gyro);
		delete(acc);
		//	return
		return(value);
#		endif // defined
	}

	IMU_Vector* IMU_MARGdata::Fusion3D(void)
	{
		//	get sensor data an d normalize
		IMU_Vector* acc = this->Acceleration();
		acc->Normalize();
		IMU_Vector* gyro = this->Gyroscope();
		gyro->Normalize();
		IMU_Vector* mag = this->Magnetometer();
		mag->Normalize();
		//	Pitch&Roll Euler angle
		double gyroFactor = 0.95L;
		double valX = (acc->X *(1.0-gyroFactor)) + (gyro->X *gyroFactor);
		double valY = (acc->Y *(1.0-gyroFactor)) + (gyro->Y *gyroFactor);
		double valZ = (acc->Z *(1.0-gyroFactor)) + (gyro->Z *gyroFactor);
		double EulerPitch = std::atan2( valY, valZ );
		double EulerRoll = -std::atan2( valX, std::sqrt(std::pow(valY,2) + std::pow(valZ,2)) );
		//	Tilt compensation of Magnetometer
		double radXH = (mag->X * std::cos(EulerPitch)) + (mag->Y * std::sin(EulerPitch) * std::sin(EulerRoll)) + (mag->Z * std::sin(EulerPitch) * std::cos(EulerRoll));
		double radYH = (mag->Y * std::cos(EulerRoll)) + (mag->Z * std::sin(EulerRoll));
		double MagYaw = std::atan2(-radYH, radXH);
		//	done
		IMU_Vector* value = new IMU_Vector(EulerPitch,EulerRoll, MagYaw, (180/M_PI));
		return(value);
	}

};
