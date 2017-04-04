/*	I2Csensor
 *	handling of 9DOF sensors using I2C bus
 *	reading absolute data from gyroscope,accelerometer,geomagnetic
 *	reading/calculating orientation and converting to J2000
 */

#ifndef _I2CSENSOR_HPP_
#define _I2CSENSOR_HPP_

#include "IMU.hpp"

#include <cstdlib>
#include <cstddef>
#include <pthread.h>
using namespace std;
namespace rpiScope
{

	class I2Cdevice
	{
		public:
			I2Cdevice(const int i2cdeviceaddress=-1, const char* i2cbusdevice=NULL);
			~I2Cdevice();
		protected:
			int fdbus;	//	i2c bus device file descriptor
			const char* devbus;	//	i2c bus device file
			unsigned char i2caddress;	//	i2c device address
			unsigned long i2cfuncs;	//	supported i2c device functions
			I2Cdevice* I2Copen(const char* i2cbusdevice="/dev/i2c-1");
			I2Cdevice* I2Cclose(void);
			I2Cdevice* I2Cselect(const int i2cdeviceaddress=-1);
			I2Cdevice* I2Cwrite(char address, const unsigned char value);
			I2Cdevice* I2Cwrite(char address, const unsigned char* value);
			I2Cdevice* I2Cread(char address, unsigned char* value);
			I2Cdevice* I2Cread(char address, unsigned char* value, int length);
			bool I2Cready(void);
		private:
	};

	typedef enum I2Csensortype
	{
		I2C_NoSensor=-1,
		I2C_AutoIdentify=0,
		I2C_LSM9DS1,
		I2C_BNO055,
	}	I2Csensortype;
	class I2Csensor : public I2Cdevice
	{
		public:
			I2Csensor(I2Csensortype i2csensor=I2C_AutoIdentify, const int i2cdeviceaddress=-1, const char* i2cbusdevice=NULL);
			~I2Csensor();
			/*	i2c device addresses
			 *	0x29	BNO055 9DOF (default address COM3=hi)
			 *	0x28	BNO055 9DOF (alternative address COM3=lo)
			 *	0x40	BNO055 9DOF (HID-I2C COM3=open)
			 *	0x6B	LSM9DS1 accelerometer,gyroscope
			 *	0x1E	LSM9DS1 geomagnetic
			 *	0x1D	LSM9DS0 accelerometer,geomagnetic
			 *	0x6B	LSM9DS0 gyroscope
			 *	0x3C	HMC5883L write 3-axis geomagnetic compass
			 *	0x3D	HMC5883L read 3-axis geomagnetic compass
			 *	0x53	ADXL345 accelerometer
			 *	0x38	FT6206 touch interface
			 */
			I2Csensortype sensortype;
			void I2Cinitialize(void);
#			define I2C_BUFFER_MAXPAGE 4
#			define I2C_BUFFER_PAGESIZE 256
			unsigned char DataBuffer[I2C_BUFFER_MAXPAGE*I2C_BUFFER_PAGESIZE];
			void I2Cread2buffer(void);
			void I2Creadimu(void);
			IMU_MARGdata IMUvalue;
			void pthread_I2Creading(void);
			void pthread_stopp(void);
		protected:
			unsigned char i2caddress_gyro;	//	i2c device address, gyroscope
			unsigned char i2caddress_acc;	//	i2c device address, accelerometer
			unsigned char i2caddress_mag;	//	i2c device address, geomagnetic
			bool Identify_LSM9DS1(void);
			bool Identify_BNO055(void);
			//	threading
			bool pthread_stopping;
			pthread_t pthread_read;
			pthread_attr_t pthread_attributes;
			void DebugDataBuffer(void);
			friend void *pthread_DataReading(void *data);
		private:
	};
	void *pthread_DataReading(void *data);

};
#endif	/* _I2CSENSOR_HPP_ */
