/*	IMU
 *	handling of IMU (inertial measurement unit)
 */

#ifndef _IMU_HPP_
#define _IMU_HPP_

#include <cstdlib>
#include <cstddef>
#include <deque>
#include <pthread.h>
using namespace std;
namespace rpiScope
{

	class IMU_Vector
	{
		public:
			double X;	//	X,Y,Z
			double Y;	//	X,Y,Z
			double Z;	//	X,Y,Z
			double FullScale;
			IMU_Vector();
			IMU_Vector(double valX, double valY, double valZ, double valScale=1.0);
			IMU_Vector(int16_t valX, int16_t valY, int16_t valZ, double valScale=1.0);
			IMU_Vector(int32_t valX, int32_t valY, int32_t valZ, double valScale=1.0);
			IMU_Vector(IMU_Vector* value);
			IMU_Vector& set(double valX, double valY, double valZ, double valScale=0.0);
			IMU_Vector& set(int16_t valX, int16_t valY, int16_t valZ, double valScale=0.0);
			IMU_Vector& set(int32_t valX, int32_t valY, int32_t valZ, double valScale=0.0);
			double scaledX(void);
			double scaledY(void);
			double scaledZ(void);
			double Length;
			IMU_Vector& Normalize(void);
			IMU_Vector* ToEuler(void);
		protected:
		private:
	};

	class IMU_Data
	{
		friend class IMU_MARGdata;
		public:
			IMU_Data();
			~IMU_Data();
			void LPF_resize(size_t LPFValues);
			IMU_Vector* vector(void);
			int16_t rawX(void);
			double scaledX(void);
			int16_t rawY(void);
			double scaledY(void);
			int16_t rawZ(void);
			double scaledZ(void);
			void	Push(int16_t X, int16_t Y, int16_t Z);
		protected:
			size_t LPF_MaxValues;
			std::deque<IMU_Vector> LPF_Values;
			pthread_mutex_t pthread_mutex;
			void InitMutex(void);
			void DestroyMutex(void);
			double FullScale;
		private:
	};

	class IMU_MARGdata
	{
		public:
			IMU_MARGdata(size_t LPFValues=1);
			~IMU_MARGdata();
			void LPF_resize(size_t LPFValues);
			IMU_Vector* Magnetometer(void);
			IMU_Vector* Acceleration(void);
			IMU_Vector* Gyroscope(void);
			void PushMagnetometer(int16_t X, int16_t Y, int16_t Z);
			void PushAcceleration(int16_t X, int16_t Y, int16_t Z);
			void PushGyroscope(int16_t X, int16_t Y, int16_t Z);
			void SetFullScale(double gyro, double acc, double mag);
			//	get calculated values
			IMU_Vector* Orientation(void);
			IMU_Vector* Fusion3D(void);
		protected:
			IMU_Data DataMagnetometer;
			IMU_Data DataAcceleration;
			IMU_Data DataGyroscope;
		private:
	};

};
#endif	/* _IMU_HPP_ */
