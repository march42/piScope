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
			int16_t	X;	//	X,Y,Z
			int16_t	Y;	//	X,Y,Z
			int16_t	Z;	//	X,Y,Z
			IMU_Vector();
			IMU_Vector(int16_t X, int16_t Y, int16_t Z);
			IMU_Vector& set(int16_t X, int16_t Y, int16_t Z);
		protected:
		private:
	};

	class IMU_MARGdata
	{
		public:
			IMU_MARGdata(size_t LPFValues=1);
			~IMU_MARGdata();
			void LPF_resize(size_t LPFValues);
			IMU_Vector*	Magnetometer(void);
			IMU_Vector*	Acceleration(void);
			IMU_Vector*	Gyroscope(void);
			void	PushMagnetometer(int16_t X, int16_t Y, int16_t Z);
			void	PushAcceleration(int16_t X, int16_t Y, int16_t Z);
			void	PushGyroscope(int16_t X, int16_t Y, int16_t Z);
		protected:
			size_t LPF_MaxValues;
			std::deque<IMU_Vector>	LPF_Magnetometer;
			std::deque<IMU_Vector>	LPF_Acceleration;
			std::deque<IMU_Vector>	LPF_Gyroscope;
			pthread_mutex_t pthread_mutex_mag;
			pthread_mutex_t pthread_mutex_acc;
			pthread_mutex_t pthread_mutex_gyro;
			void InitMutex(void);
			void DestroyMutex(void);
		private:
	};

};
#endif	/* _IMU_HPP_ */
