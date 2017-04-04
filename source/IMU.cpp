/*	IMU
 *	handling of IMU (inertial measurement unit)
 */

#include "IMU.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <cmath>
#include <climits>

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

	scale factor
		16bit signed integer ==32768
		gyroscope	+/- 245 dps = 32768/(2x245)
		accelerometer	+/- 2 g = 32768/(2x2)
		magnetometer	+/- 4 gauss = 32768/(2x4)
*/

	IMU_Vector::IMU_Vector()
	{
		this->X = 0;
		this->Y = 0;
		this->Z = 0;
	};
	IMU_Vector::IMU_Vector(int16_t X, int16_t Y, int16_t Z)
	{
		this->X = X;
		this->Y = Y;
		this->Z = Z;
	};
	IMU_Vector& IMU_Vector::set(int16_t X, int16_t Y, int16_t Z)
	{
		this->X = X;
		this->Y = Y;
		this->Z = Z;
		return(*this);
	};

	IMU_MARGdata::IMU_MARGdata(size_t LPFValues)
	{
		this->LPF_resize(LPFValues);
		this->InitMutex();
	}
	IMU_MARGdata::~IMU_MARGdata()
	{
		this->DestroyMutex();
	}

	void IMU_MARGdata::LPF_resize(size_t LPFValues)
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

	void IMU_MARGdata::InitMutex(void)
	{
		//	init mutex
		pthread_mutex_init(&this->pthread_mutex_gyro, NULL);
		pthread_mutex_init(&this->pthread_mutex_acc, NULL);
		pthread_mutex_init(&this->pthread_mutex_mag, NULL);
	}
	void IMU_MARGdata::DestroyMutex(void)
	{
		//	destroy mutex
		pthread_mutex_lock(&this->pthread_mutex_gyro);
		pthread_mutex_destroy(&this->pthread_mutex_gyro);
		pthread_mutex_lock(&this->pthread_mutex_acc);
		pthread_mutex_destroy(&this->pthread_mutex_acc);
		pthread_mutex_lock(&this->pthread_mutex_mag);
		pthread_mutex_destroy(&this->pthread_mutex_mag);
	}

	IMU_Vector*	IMU_MARGdata::Magnetometer(void)
	{
		static IMU_Vector value;
		//	calculate average
		int32_t X = 0;
		int32_t Y = 0;
		int32_t Z = 0;
		pthread_mutex_lock(&this->pthread_mutex_mag);
		size_t count = this->LPF_Magnetometer.size();
		if(0 < count)
		{
			for(size_t pos=0; pos<count; ++pos)
			{
				X += this->LPF_Magnetometer[pos].X;
				Y += this->LPF_Magnetometer[pos].Y;
				Z += this->LPF_Magnetometer[pos].Z;
			}
			X /= count;
			Y /= count;
			Z /= count;
			value.set(X &0xFFFF, Y &0xFFFF, Z &0xFFFF);
		}
		pthread_mutex_unlock(&this->pthread_mutex_mag);
		return(&value);
	}
	IMU_Vector*	IMU_MARGdata::Acceleration(void)
	{
		static IMU_Vector value;
		//	calculate average
		int32_t X = 0;
		int32_t Y = 0;
		int32_t Z = 0;
		pthread_mutex_lock(&this->pthread_mutex_acc);
		size_t count = this->LPF_Acceleration.size();
		if(0 < count)
		{
			for(size_t pos=0; pos<count; ++pos)
			{
				X += this->LPF_Acceleration[pos].X;
				Y += this->LPF_Acceleration[pos].Y;
				Z += this->LPF_Acceleration[pos].Z;
			}
			X /= count;
			Y /= count;
			Z /= count;
			value.set(X &0xFFFF, Y &0xFFFF, Z &0xFFFF);
		}
		pthread_mutex_unlock(&this->pthread_mutex_acc);
		return(&value);
	}
	IMU_Vector*	IMU_MARGdata::Gyroscope(void)
	{
		static IMU_Vector value;
		//	calculate average
		int32_t X = 0;
		int32_t Y = 0;
		int32_t Z = 0;
		pthread_mutex_lock(&this->pthread_mutex_gyro);
		size_t count = this->LPF_Gyroscope.size();
		if(0 < count)
		{
			for(size_t pos=0; pos<count; ++pos)
			{
				X += this->LPF_Gyroscope[pos].X;
				Y += this->LPF_Gyroscope[pos].Y;
				Z += this->LPF_Gyroscope[pos].Z;
			}
			X /= count;
			Y /= count;
			Z /= count;
			value.set(X &0xFFFF, Y &0xFFFF, Z &0xFFFF);
		}
		pthread_mutex_unlock(&this->pthread_mutex_gyro);
		return(&value);
	}

	void	IMU_MARGdata::PushMagnetometer(int16_t X, int16_t Y, int16_t Z)
	{
		IMU_Vector value(X,Y,Z);
		pthread_mutex_lock(&this->pthread_mutex_mag);
		this->LPF_Magnetometer.push_back(value);
		while(this->LPF_MaxValues < this->LPF_Magnetometer.size())
		{
			this->LPF_Magnetometer.pop_front();
		}
		pthread_mutex_unlock(&this->pthread_mutex_mag);
	}
	void	IMU_MARGdata::PushAcceleration(int16_t X, int16_t Y, int16_t Z)
	{
		IMU_Vector value(X,Y,Z);
		pthread_mutex_lock(&this->pthread_mutex_acc);
		this->LPF_Acceleration.push_back(value);
		while(this->LPF_MaxValues < this->LPF_Acceleration.size())
		{
			this->LPF_Acceleration.pop_front();
		}
		pthread_mutex_unlock(&this->pthread_mutex_acc);
	}
	void	IMU_MARGdata::PushGyroscope(int16_t X, int16_t Y, int16_t Z)
	{
		IMU_Vector value(X,Y,Z);
		pthread_mutex_lock(&this->pthread_mutex_gyro);
		this->LPF_Gyroscope.push_back(value);
		while(this->LPF_MaxValues < this->LPF_Gyroscope.size())
		{
			this->LPF_Gyroscope.pop_front();
		}
		pthread_mutex_unlock(&this->pthread_mutex_gyro);
	}


};
