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
		this->FullScale = 1.0;
	};
	IMU_Vector::IMU_Vector(int16_t valX, int16_t valY, int16_t valZ, float valScale)
	{
		this->X = valX;
		this->Y = valY;
		this->Z = valZ;
		this->FullScale = valScale;
	};
	IMU_Vector& IMU_Vector::set(int16_t valX, int16_t valY, int16_t valZ, float valScale)
	{
		this->X = valX;
		this->Y = valY;
		this->Z = valZ;
		this->FullScale = valScale;
		return(*this);
	};
	float IMU_Vector::scaledX(void)
	{
		return(this->X * this->FullScale);
	}
	float IMU_Vector::scaledY(void)
	{
		return(this->Y * this->FullScale);
	}
	float IMU_Vector::scaledZ(void)
	{
		return(this->Z * this->FullScale);
	}

	IMU_Data::IMU_Data()
	{
		this->LPF_MaxValues = 32;
		this->FullScale = 1.0;
		this->InitMutex();
	}
	IMU_Data::~IMU_Data()
	{
#		ifdef DEBUG2
		printf("\t%s\t%s\t%s\n", "IMU_Data", "destructor", "begin");
#		endif
		this->DestroyMutex();
#		ifdef DEBUG2
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
#		ifdef DEBUG2
		printf("\t%s\t%s\t%s\n", "IMU_Data", "DestroyMutex", "");
#		endif
		//	destroy mutex
		pthread_mutex_lock(&this->pthread_mutex);
		pthread_mutex_destroy(&this->pthread_mutex);
	}
	IMU_Vector*	IMU_Data::vector(void)
	{
		//	calculate average
		int32_t X = 0;
		int32_t Y = 0;
		int32_t Z = 0;
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
		IMU_Vector* value = new IMU_Vector(X &0xFFFF, Y &0xFFFF, Z &0xFFFF, this->FullScale);
		return(value);
	}
	int16_t IMU_Data::rawX(void)
	{
		return(this->LPF_Values.empty() ?0 :this->LPF_Values.back().X);
	}
	float IMU_Data::scaledX(void)
	{
		return(this->LPF_Values.empty() ?0.0 :(this->LPF_Values.back().X * this->FullScale));
	}
	int16_t IMU_Data::rawY(void)
	{
		return(this->LPF_Values.empty() ?0 :this->LPF_Values.back().Y);
	}
	float IMU_Data::scaledY(void)
	{
		return(this->LPF_Values.empty() ?0.0 :(this->LPF_Values.back().Y * this->FullScale));
	}
	int16_t IMU_Data::rawZ(void)
	{
		return(this->LPF_Values.empty() ?0 :this->LPF_Values.back().Z);
	}
	float IMU_Data::scaledZ(void)
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
#		ifdef DEBUG2
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
	void IMU_MARGdata::SetFullScale(float gyro, float acc, float mag)
	{
		this->DataGyroscope.FullScale = gyro;
		this->DataAcceleration.FullScale = acc;
		this->DataMagnetometer.FullScale = mag;
	}


};
