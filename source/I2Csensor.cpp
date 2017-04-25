/*	I2Csensor
 *	handling of 9DOF sensors using I2C bus
 *	reading absolute data from gyroscope,accelerometer,geomagnetic
 *	reading/calculating orientation and converting to J2000
 */

#include "I2Csensor.hpp"
#define BUFFER_I2CREAD_BLOCK(regpage,regfirst,reglast) this->I2Cread(regfirst, &(this->DataBuffer[(regpage*I2C_BUFFER_PAGESIZE) +regfirst]), (reglast-regfirst) +1)
#define BUFFER_REGISTER(regpage,regaddr) (this->DataBuffer[((regpage*I2C_BUFFER_PAGESIZE) +regaddr)])

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#if !defined(USE_LINUX_I2CDEV)
#	include "i2c-dev.h"
#else
#	include <linux/i2c-dev.h>
#	include <linux/i2c.h>
#endif
#include <sys/ioctl.h>
#include <fcntl.h>
#include <cmath>
#include <climits>
#include <cassert>

using namespace std;
namespace rpiScope
{

	I2Cdevice::I2Cdevice(const int i2cdeviceaddress, const char* i2cbusdevice)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cdevice", "constructor begin", "");
#		endif
		//	start with invalid values
		this->fdbus = -1;
		this->devbus = NULL;
		this->i2caddress = 0x00;
		this->i2cfuncs = 0;
		//	initialize I2C bus
		this->I2Copen(i2cbusdevice);
		//	remember device address
		if(-1 != i2cdeviceaddress)
		{
			this->i2caddress = i2cdeviceaddress;
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cdevice", "constructor done", "");
#		endif
	}

	I2Cdevice::~I2Cdevice()
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cdevice", "destructor begin", "");
#		endif
		this->I2Cclose();
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cdevice", "destructor done", "");
#		endif
	}

	I2Cdevice* I2Cdevice::I2Copen(const char* i2cbusdevice)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Copen", "begin", "");
#		endif
		//	check for opened device
		if(-1 != this->fdbus)
		{
			return(this);
		}
		//	open
		if(NULL != i2cbusdevice)
		{
			this->devbus = i2cbusdevice;
		}
		this->fdbus = open(this->devbus, O_RDWR);
		if(0 >= this->fdbus)
		{
			perror("I2C bus device open failed");
			return(this);
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Copen", "opened", this->devbus);
#		endif
		return(this);
	}
	I2Cdevice* I2Cdevice::I2Cclose(void)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cclose", "begin", "");
#		endif
		if(-1 != this->fdbus)
		{
			if(0 > close(this->fdbus))
			{
				perror("I2C bus device close failed");
			}
			else
			{
				this->fdbus = -1;
			}
#		if defined(DEBUG4)
			//	function, step, extra
			printf("\t%s\t%s\t%s\n", "I2Cclose", "closed", this->devbus);
#			endif
		}
		return(this);
	}

	I2Cdevice* I2Cdevice::I2Cselect(const int i2cdeviceaddress)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\tnow=0x%02X\tclass=0x%02X\n", "I2Cselect", i2cdeviceaddress, this->i2caddress);
#		endif
		if(-1 != i2cdeviceaddress)
		{
			this->i2caddress = i2cdeviceaddress;
		}
		//	check bus is opened
		if(-1 == this->fdbus)
		{
			this->I2Copen(this->devbus);
		}
		//	check address
		if (0 == this->i2caddress)
		{
			perror("I2C no slave address");
		}
		// check I2C functions
		else if( 0 > ioctl(this->fdbus, I2C_FUNCS, &this->i2cfuncs) )
		{
			perror("I2C ioctl I2C_FUNCS failed");
		}
		else if( 0 == (this->i2cfuncs & I2C_FUNC_I2C) )
		{
			perror("I2C_FUNC_I2C not supported");
		}
		// set to 7-bit addr
		else if ( I2C_FUNC_10BIT_ADDR == (this->i2cfuncs & I2C_FUNC_10BIT_ADDR) && 0 > ioctl(this->fdbus, I2C_TENBIT, 0) )
		{
			perror("I2C ioctl I2C_TENBIT failed");
		}
		// set the address
		else if ( 0 > ioctl(this->fdbus, I2C_SLAVE, this->i2caddress) )
		{
			perror("I2C ioctl I2C_SLAVE failed");
		}
#		if defined(DEBUG4)
		else
		{
			//	function, step, extra
			printf("\t%s\t0x%02X\tI2C_FUNCS =0x%08lX\n", "I2Cselect", this->i2caddress, this->i2cfuncs);
			/*
			if( 0 != (this->i2cfuncs & I2C_FUNC_I2C) )	printf("\t%s\n", "I2C_FUNC_I2C");
			if( 0 != (this->i2cfuncs & I2C_FUNC_10BIT_ADDR) )	printf("\t%s\n", "I2C_FUNC_10BIT_ADDR");
			if( 0 != (this->i2cfuncs & I2C_FUNC_PROTOCOL_MANGLING) )	printf("\t%s\n", "I2C_FUNC_PROTOCOL_MANGLING");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_PEC) )	printf("\t%s\n", "I2C_FUNC_SMBUS_PEC");
			if( 0 != (this->i2cfuncs & I2C_FUNC_NOSTART) )	printf("\t%s\n", "I2C_FUNC_NOSTART");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_BLOCK_PROC_CALL) )	printf("\t%s\n", "I2C_FUNC_SMBUS_BLOCK_PROC_CALL");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_QUICK) )	printf("\t%s\n", "I2C_FUNC_SMBUS_QUICK");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_READ_BYTE) )	printf("\t%s\n", "I2C_FUNC_SMBUS_READ_BYTE");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_WRITE_BYTE) )	printf("\t%s\n", "I2C_FUNC_SMBUS_WRITE_BYTE");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_READ_BYTE_DATA) )	printf("\t%s\n", "I2C_FUNC_SMBUS_READ_BYTE_DATA");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_WRITE_BYTE_DATA) )	printf("\t%s\n", "I2C_FUNC_SMBUS_WRITE_BYTE_DATA");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_READ_WORD_DATA) )	printf("\t%s\n", "I2C_FUNC_SMBUS_READ_WORD_DATA");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_WRITE_WORD_DATA) )	printf("\t%s\n", "I2C_FUNC_SMBUS_WRITE_WORD_DATA");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_PROC_CALL) )	printf("\t%s\n", "I2C_FUNC_SMBUS_PROC_CALL");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_READ_BLOCK_DATA) )	printf("\t%s\n", "I2C_FUNC_SMBUS_READ_BLOCK_DATA");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_WRITE_BLOCK_DATA) )	printf("\t%s\n", "I2C_FUNC_SMBUS_WRITE_BLOCK_DATA");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_READ_I2C_BLOCK) )	printf("\t%s\n", "I2C_FUNC_SMBUS_READ_I2C_BLOCK");
			if( 0 != (this->i2cfuncs & I2C_FUNC_SMBUS_WRITE_I2C_BLOCK) )	printf("\t%s\n", "I2C_FUNC_SMBUS_WRITE_I2C_BLOCK");
			*/
		}
#		endif
		return(this);
	}

	I2Cdevice* I2Cdevice::I2Cwrite(char address, const int value)
	{
		//	write buffer to device
		if(-1 == i2c_smbus_write_byte_data(this->fdbus, address, (unsigned char)value))
		{
			char message[100];
			sprintf(message, "i2c_smbus_write_byte_data failed [I2Cwrite %02X=%02X]", address, value);
			perror(message);
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cwrite", "", "");
#		endif
		// return
		return(this);
	}
	I2Cdevice* I2Cdevice::I2Cwrite(char address, const unsigned char* value)
	{
		//	prepare data buffer
		unsigned char buffer[2];
		buffer[0] = address;
		buffer[1] = *value;
		//	write buffer to device
		if(-1 == i2c_smbus_write_byte_data(this->fdbus, buffer[0], buffer[1]))
		//if(1 != i2c_smbus_write_byte_data(this->fdbus, address, *value))
		{
			char message[100];
			sprintf(message, "i2c_smbus_write_byte_data failed [I2Cwrite %02X=%02X]", buffer[0], buffer[1]);
			perror(message);
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cwrite", "", "");
#		endif
		// return
		return(this);
	}

	I2Cdevice* I2Cdevice::I2Cread(char address, unsigned char* value)
	{
		int buffer = i2c_smbus_read_byte_data(this->fdbus, address);
		if(0 > buffer)
		{
			perror("i2c_smbus_read_byte_data (I2Cread) failed");
		}
		else
		{
			*value = buffer &0xFF;
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cread", "byte", "");
#		endif
		return(this);
	}
	I2Cdevice* I2Cdevice::I2Cread(char address, unsigned char* value, int length)
	{
		//i2c_smbus_read_byte_data
		unsigned char* pos = value;
		int togo = length;
		while(0 < togo)
		{
			int rbytes = 0;
			//	limit read to 32 Bytes, to comply with SMBus
			if(0 > (rbytes = i2c_smbus_read_i2c_block_data(this->fdbus, address, (32<togo ?32 :togo), pos)))
			{
				perror("i2c_smbus_read_i2c_block_data failed");
				break;
			}
			pos += rbytes;
			togo -= rbytes;
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cread", "block", "");
#		endif
		// return anyway
		return(this);
	}

	bool I2Cdevice::I2Cready(void)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cready", "", "");
#		endif
		return (0 < this->fdbus);
	}

	I2Csensor::I2Csensor(I2Csensortype i2csensor, const int i2cdeviceaddress, const char* i2cbusdevice) : I2Cdevice(i2cdeviceaddress, i2cbusdevice)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Csensor", "constructor begin", "");
#		endif
		this->i2caddress_gyro = this->i2caddress_acc = this->i2caddress_mag = 0;
		memset(&this->DataBuffer[0], 0x00, sizeof(this->DataBuffer));
		//	prepare pthread
		this->pthread_stopping = true;
		//	prepare and test I2C
		if(-1 != i2cdeviceaddress)
		{
			this->i2caddress = i2cdeviceaddress;
		}
		if(-1 == this->fdbus)
		{
			this->I2Copen(i2cbusdevice);
		}
		//	auto identify sensor
		if(I2C_AutoIdentify == i2csensor)
		{
			if(this->Identify_BNO055())
			{
				this->sensortype = I2C_BNO055;
			}
			else if(this->Identify_LSM9DS1())
			{
				this->sensortype = I2C_LSM9DS1;
				this->I2Cinitialize();
			}
			else
			{
				this->sensortype = I2C_NoSensor;
				perror("No I2C sensor identified");
				this->I2Cclose();
			}
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Csensor", "constructor", "");
#		endif
	}
	I2Csensor::~I2Csensor()
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Csensor", "destructor", "");
#		endif
		//	deinit
		if(I2C_LSM9DS1 == this->sensortype)
		{
			//	gyroscope, accelerometer
			this->I2Cselect(this->i2caddress_acc);
			this->I2Cwrite(0x22, 0b10000000);	//	REBOOT memory content
			usleep(1000);
			this->I2Cwrite(0x10, 0b00000000);	//	POWER DOWN gyro
			this->I2Cwrite(0x20, 0b00000000);	//	POWER DOWN acc
			//	magnetometer
			this->I2Cselect(this->i2caddress_mag);
			this->I2Cwrite(0x21, 0b00001000);	//	REBOOT memory content
			usleep(1000);
			this->I2Cwrite(0x22, 0b00000011);	//	POWER DOWN mag
		}
		//	stop and clean threads
		this->pthread_stopp();
	}

	void I2Csensor::I2Cread2buffer(void)
	{
		this->I2Copen();
		if(I2C_LSM9DS1 == this->sensortype)
		{
			this->I2Cselect(this->i2caddress_acc);
			BUFFER_I2CREAD_BLOCK(0,0x04,0x0D);
			BUFFER_I2CREAD_BLOCK(0,0x18,0x1D);	// gyro, should restart at 0x18 afterwards
			BUFFER_I2CREAD_BLOCK(0,0x1E,0x24);
			BUFFER_I2CREAD_BLOCK(0,0x26,0x27);
			BUFFER_I2CREAD_BLOCK(0,0x28,0x2D);	// acc, should restart at 0x28 afterwards
			BUFFER_I2CREAD_BLOCK(0,0x2E,0x37);
			this->I2Cselect(this->i2caddress_mag);
			BUFFER_I2CREAD_BLOCK(1,0x05,0x0A);
			BUFFER_I2CREAD_BLOCK(1,0x0F,0x0F);
			BUFFER_I2CREAD_BLOCK(1,0x20,0x24);
			BUFFER_I2CREAD_BLOCK(1,0x27,0x27);
			BUFFER_I2CREAD_BLOCK(1,0x28,0x2D);	// mag, should restart at 0x28 afterwards
			BUFFER_I2CREAD_BLOCK(1,0x30,0x33);
			//	set full scale
			float gyro = 1.0;	// dps/LSB
			if(0b00000000 == (BUFFER_REGISTER(0,0x10) &0b00011000))
				gyro = 0.00875;//245.0/32768;
			else if(0b00001000 == (BUFFER_REGISTER(0,0x10) &0b00011000))
				gyro = 0.01750;//500.0/32768;
			else if(0b00011000 == (BUFFER_REGISTER(0,0x10) &0b00011000))
				gyro = 0.07;//2000.0/32768;
			float acc = 1.0;	// G/LSB
			if(0b00000000 == (BUFFER_REGISTER(0,0x20) &0b00011000))
				acc = 0.000061;//2.0/32768;
			else if(0b00001000 == (BUFFER_REGISTER(0,0x20) &0b00011000))
				acc = 0.000732;//16.0/32768;
			else if(0b00010000 == (BUFFER_REGISTER(0,0x20) &0b00011000))
				acc = 0.000122;//4.0/32768;
			else if(0b00011000 == (BUFFER_REGISTER(0,0x20) &0b00011000))
				acc = 0.000244;//8.0/32768;
			float mag = 1.0;	// gauss/LSB
			if(0b00000000 == (BUFFER_REGISTER(1,0x21) &0b01100000))
				mag = 0.00014;//4.0/32768;
			else if(0b00100000 == (BUFFER_REGISTER(1,0x21) &0b01100000))
				mag = 0.00029;//8.0/32768;
			else if(0b01000000 == (BUFFER_REGISTER(1,0x21) &0b01100000))
				mag = 0.00043;//12.0/32768;
			else if(0b01100000 == (BUFFER_REGISTER(1,0x21) &0b01100000))
				mag = 0.00058;//16.0/32768;
			this->IMUvalue.SetFullScale(gyro, acc, mag);
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			unsigned char pagebuffer[I2C_BUFFER_PAGESIZE];
			memset(&pagebuffer[0], 0x00, sizeof(pagebuffer));
			this->I2Cselect(this->i2caddress_acc);
			this->I2Cread(0x00, &pagebuffer[0], 0x7F-0x00 +1);
			memcpy(&(this->DataBuffer[(pagebuffer[0x07] *I2C_BUFFER_PAGESIZE)]), &pagebuffer[0], 0x7F-0x00 +1);
			pagebuffer[0x07] ^= 0x01;
			this->I2Cwrite(0x07, &pagebuffer[0x07]);
			this->I2Cread(0x00, &(this->DataBuffer[(pagebuffer[0x07] *I2C_BUFFER_PAGESIZE)]), 0x7F-0x00 +1);
		}
		else
		{
			perror("I2Cread2buffer needs a known sensor type");
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cread2buffer", "done", "");
#		endif
	}

	void I2Csensor::I2Creadimu(void)
	{
		int16_t X = 0;
		int16_t Y = 0;
		int16_t Z = 0;
		this->I2Copen();
		if(I2C_LSM9DS1 == this->sensortype)
		{
			this->I2Cselect(this->i2caddress_acc);
			BUFFER_I2CREAD_BLOCK(0,0x18,0x1D);	// should restart at 0x18 afterwards
			X = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x19)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x18)]);
			Y = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x1B)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x1A)]);
			Z = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x1D)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x1C)]);
			this->IMUvalue.PushGyroscope(X,Y,Z);
			BUFFER_I2CREAD_BLOCK(0,0x28,0x2D);	// should restart at 0x28 afterwards
			X = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x29)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x28)]);
			Y = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x2B)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x2A)]);
			Z = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x2D)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x2C)]);
			this->IMUvalue.PushAcceleration(X,Y,Z);
			this->I2Cselect(this->i2caddress_mag);
			BUFFER_I2CREAD_BLOCK(1,0x28,0x2D);	// should restart at 0x28 afterwards
			X = (this->DataBuffer[((1*I2C_BUFFER_PAGESIZE) +0x29)] <<8) | (this->DataBuffer[((1*I2C_BUFFER_PAGESIZE) +0x28)]);
			Y = (this->DataBuffer[((1*I2C_BUFFER_PAGESIZE) +0x2B)] <<8) | (this->DataBuffer[((1*I2C_BUFFER_PAGESIZE) +0x2A)]);
			Z = (this->DataBuffer[((1*I2C_BUFFER_PAGESIZE) +0x2D)] <<8) | (this->DataBuffer[((1*I2C_BUFFER_PAGESIZE) +0x2C)]);
			this->IMUvalue.PushMagnetometer(X,Y,Z);
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			this->I2Cread2buffer();
		}
		else
		{
			this->I2Cread2buffer();
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Creadimu", "done", "");
#		endif
	}

	void I2Csensor::I2Cinitialize(void)
	{
		this->I2Cclose();
		this->I2Copen();
		//	resize LPF filter stack
		this->IMUvalue.LPF_resize(32);
		//	initialize sensor configuration
		if(I2C_LSM9DS1 == this->sensortype)
		{
			unsigned char valNow = 0;
			unsigned char valNew = 0;
			//	configure acc,gyro
			this->I2Cselect(this->i2caddress_acc);
			this->I2Cwrite(0x22, 0b10000000);	//	REBOOT memory content
			sleep(1);
			this->I2Cread(0x22, &valNow);
			assert(0 == (valNow &0x80));	//	REBOOT bit cleared
			valNew = 0b00100000;	//	gyro 14.9Hz, full scale 245dps
			//this->I2Cread(0x10, &valNow); valNew |= (valNow &0b00011011);
			this->I2Cwrite(0x10, valNew);
			valNew = 0b00000000;	//	no interrupt, default output selection
			this->I2Cwrite(0x11, valNew);
			valNew = 0b01000011;	//	gyro HPF enabled, cutoff 0.1Hz
			this->I2Cwrite(0x12, valNew);
			valNew = 0b00000000;	//	gyro X,Y,Z sign positive, directional user orientation =000
			this->I2Cwrite(0x13, valNew);
			valNew = 0b00111000;	//	gyro X,Y,Z enabled
			this->I2Cwrite(0x1E, valNew);
			valNew = 0b10111000;	//	acc update every 4th sample, X,Y,Z enabled
			this->I2Cwrite(0x1F, valNew);
			valNew = 0b01011000;	//	acc 50Hz, full scale 8G
			this->I2Cwrite(0x20, valNew);
			valNew = 0b01100000;	//	HR acc disabled, ODR/400Hz cutoff
			this->I2Cwrite(0x21, valNew);
			valNew = 0b00000100;	//	BDU disabled, auto increment register address, LITTLE ENDIAN
			this->I2Cwrite(0x22, valNew);
			valNew = 0b00000000;
			this->I2Cwrite(0x23, valNew);
			valNew = 0b00000000;	//	gyro and acc self test disabled
			this->I2Cwrite(0x24, valNew);
			valNew = 0b00000000;	//	clear interrupt flags
			this->I2Cwrite(0x26, valNew);
			valNew = 0b00000000;	//	FIFO disabled
			this->I2Cwrite(0x2E, valNew);
			valNew = 0b00000000;	//	disable interrupts
			this->I2Cwrite(0x30, valNew);
			//	configure compass
			this->I2Cselect(this->i2caddress_mag);
			this->I2Cwrite(0x21, 0b00001000);	//	REBOOT memory content
			sleep(1);
			this->I2Cread(0x21, &valNow);
			assert(0 == (valNow &0x08));	//	REBOOT bit cleared
			valNew = 0b11010000;	//	temperature compensate, high performance X,Y, 10Hz, self test disabled
			this->I2Cwrite(0x20, valNew);
			valNew = 0b00000000;	//	full scale 4gauss
			this->I2Cwrite(0x21, valNew);
			valNew = 0b00000000;	//	disable low power, select continuous conversion
			this->I2Cwrite(0x22, valNew);
			valNew = 0b00001000;	//	high performance Z, LITTLE ENDIAN
			this->I2Cwrite(0x23, valNew);
			valNew = 0b00000000;	//	disable fast read, disable BDU
			this->I2Cwrite(0x24, valNew);
			valNew = 0b00000000;	//	disable interrupts
			this->I2Cwrite(0x30, valNew);
		}
		else if(I2C_BNO055 == this->sensortype)
		{
		}
		else
		{
			perror("I2Cinitialize needs a known sensor type");
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "I2Cread2buffer", "done", "");
#		endif
	}

	bool I2Csensor::Identify_LSM9DS1(void)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "Identify_LSM9DS1", "begin", "");
#		endif
		//	magnetometer
		int address_gyro_acc[] = {0x6A,0x6B};
		int WHOAMI_gyro_acc[] = {0x0F,0b01101000};
		int address_mag[] = {0x1C,0x1E};
		int WHOAMI_mag[] = {0x0F,0b00111101};
		unsigned char buffer[256];	memset(&buffer[0], 0x00, sizeof(buffer));
		//	select and check WHO-AM-I (first device address)
		if(NULL != this->I2Cselect(address_gyro_acc[0]) && NULL != this->I2Cread(WHOAMI_gyro_acc[0],&buffer[0]) && WHOAMI_gyro_acc[1] == buffer[0])
		{
			this->i2caddress_gyro = this->i2caddress_acc = address_gyro_acc[0];
		}
		else if(NULL != this->I2Cselect(address_gyro_acc[1]) && NULL != this->I2Cread(WHOAMI_gyro_acc[0],&buffer[0]) && WHOAMI_gyro_acc[1] == buffer[0])
		{
			this->i2caddress_gyro = this->i2caddress_acc = address_gyro_acc[1];
		}
		else
		{
			this->i2caddress_gyro = this->i2caddress_acc = 0;
			perror("LSM9DS1 (acc,gyro) not identified");
		}
		//	select and check WHO-AM-I (first device address)
		if(NULL != this->I2Cselect(address_mag[0]) && NULL != this->I2Cread(WHOAMI_mag[0],&buffer[0]) && WHOAMI_mag[1] == buffer[0])
		{
			this->i2caddress_mag = address_mag[0];
		}
		else if(NULL != this->I2Cselect(address_mag[1]) && NULL != this->I2Cread(WHOAMI_mag[0],&buffer[0]) && WHOAMI_mag[1] == buffer[0])
		{
			this->i2caddress_mag = address_mag[1];
		}
		else
		{
			this->i2caddress_mag = 0;
			perror("LSM9DS1 (mag) not identified");
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "Identify_LSM9DS1", "done", "");
#		endif
		return(0 != this->i2caddress_gyro && 0 != this->i2caddress_acc && 0 != this->i2caddress_mag);
	}

	bool I2Csensor::Identify_BNO055(void)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "Identify_BNO055", "begin", "");
#		endif
		//	magnetometer
		unsigned char address[] = {0x29,0x28};
		unsigned char PAGEID = 0x07;
		unsigned char CHIPID_BNO055[] = {0,0x00,0xA0};	//	page 0
		unsigned char CHIPID_ACC[] = {0,0x01,0xFB};	//	page 0
		unsigned char CHIPID_MAG[] = {0,0x02,0x32};	//	page 0
		unsigned char CHIPID_GYRO[] = {0,0x03,0x0F};	//	page 0
		unsigned char buffer[256];	memset(&buffer[0], 0x00, sizeof(buffer));
		//	select and check chip IDs
		if(NULL != this->I2Cselect(address[0]) && NULL != this->I2Cwrite(PAGEID,&CHIPID_BNO055[0]) && NULL != this->I2Cread(CHIPID_BNO055[1],&buffer[0],4)
			&& CHIPID_BNO055[2] == buffer[0] && CHIPID_ACC[2] == buffer[1] && CHIPID_MAG[2] == buffer[2] && CHIPID_GYRO[2] == buffer[3])
		{
			this->i2caddress_gyro = this->i2caddress_acc = this->i2caddress_mag = address[0];
		}
		else if(NULL != this->I2Cselect(address[1]) && NULL != this->I2Cwrite(PAGEID,&CHIPID_BNO055[0]) && NULL != this->I2Cread(CHIPID_BNO055[1],&buffer[0],4)
			&& CHIPID_BNO055[2] == buffer[0] && CHIPID_ACC[2] == buffer[1] && CHIPID_MAG[2] == buffer[2] && CHIPID_GYRO[2] == buffer[3])
		{
			this->i2caddress_gyro = this->i2caddress_acc = this->i2caddress_mag = address[1];
		}
		else
		{
			this->i2caddress_gyro = this->i2caddress_acc = this->i2caddress_mag = 0;
			perror("BNO055 not identified");
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "Identify_BNO055", "done", "");
#		endif
		return(0 != this->i2caddress_gyro && 0 != this->i2caddress_acc && 0 != this->i2caddress_mag);
	}

	/*
	int16_t I2Csensor::Acceleration_X(void)
	{
		int16_t value = 0;
		if(I2C_LSM9DS1 == this->sensortype)
		{
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			value = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x09)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x08)]);
		}
		return(value);
	}
	int16_t I2Csensor::Acceleration_Y(void)
	{
		int16_t value = 0;
		if(I2C_LSM9DS1 == this->sensortype)
		{
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			value = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x0B)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x0A)]);
		}
		return(value);
	}
	int16_t I2Csensor::Acceleration_Z(void)
	{
		int16_t value = 0;
		if(I2C_LSM9DS1 == this->sensortype)
		{
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			value = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x0D)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x0C)]);
		}
		return(value);
	}
	int16_t I2Csensor::Gyroscope_X(void)
	{
		int16_t value = 0;
		if(I2C_LSM9DS1 == this->sensortype)
		{
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			value = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x15)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x14)]);
		}
		return(value);
	}
	int16_t I2Csensor::Gyroscope_Y(void)
	{
		int16_t value = 0;
		if(I2C_LSM9DS1 == this->sensortype)
		{
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			value = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x17)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x16)]);
		}
		return(value);
	}
	int16_t I2Csensor::Gyroscope_Z(void)
	{
		int16_t value = 0;
		if(I2C_LSM9DS1 == this->sensortype)
		{
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			value = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x19)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x18)]);
		}
		return(value);
	}
	int16_t I2Csensor::Geomagnetic_X(void)
	{
		int16_t value = 0;
		if(I2C_LSM9DS1 == this->sensortype)
		{
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			value = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x0F)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x0E)]);
		}
		return(value);
	}
	int16_t I2Csensor::Geomagnetic_Y(void)
	{
		int16_t value = 0;
		if(I2C_LSM9DS1 == this->sensortype)
		{
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			value = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x11)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x10)]);
		}
		return(value);
	}
	int16_t I2Csensor::Geomagnetic_Z(void)
	{
		int16_t value = 0;
		if(I2C_LSM9DS1 == this->sensortype)
		{
		}
		else if(I2C_BNO055 == this->sensortype)
		{
			value = (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x13)] <<8) | (this->DataBuffer[((0*I2C_BUFFER_PAGESIZE) +0x12)]);
		}
		return(value);
	}
	*/

	void I2Csensor::pthread_I2Creading(void)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "pthread_I2Creading", "starting", "");
#		endif
		//	clear stop flag
		this->pthread_stopping = false;
		//	prepare thread attributes
		pthread_attr_init(&this->pthread_attributes);
		pthread_attr_setdetachstate(&this->pthread_attributes, PTHREAD_CREATE_JOINABLE);
		//	start thread
		int rc = pthread_create(&this->pthread_read, &this->pthread_attributes, pthread_DataReading, (void *)this);
		if(0 > rc)
		{
			perror("pthread_create failed (pthread_DataReading)");
		}
		else
		{
			sleep(1);	//	give time to start the thread
		}
	}
	void I2Csensor::pthread_stopp(void)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "pthread_stopp", "starting", "");
#		endif
		//	set stopp flags
		this->pthread_stopping = true;
		//	destroy attribute
		pthread_attr_destroy(&this->pthread_attributes);
		//	wait for thread completion
		pthread_join(this->pthread_read, NULL);
	}

	void *pthread_DataReading(void *data)
	{
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "pthread_DataReading", "starting", "");
#		endif
		I2Csensor* mother = (I2Csensor*)data;
		printf("starting sensor reading (G=%02X, A=%02X, M=%02X)\n", mother->i2caddress_gyro,mother->i2caddress_acc,mother->i2caddress_mag);
		//	start preparation
		int read_counter = 0;
		while(!mother->pthread_stopping)
		{
			if(rpiScope::I2C_NoSensor == mother->sensortype)
			{
				read_counter = 0;
				sleep(30);	//	give some time, for connection or break up
				//mother->I2Cinitialize();
				continue;
			}
			//	count reading (1Hz interval for complete buffer)
			if(0 == (read_counter++ &0x3F))
			{
				mother->I2Cread2buffer();
#				if defined(DEBUG5)
				mother->DebugDataBuffer();
#				endif
			}
			else
			{
				mother->I2Creadimu();
			}
			usleep(1000000 >>5);	//	32Hz reading
		}
#		if defined(DEBUG4)
		//	function, step, extra
		printf("\t%s\t%s\t%s\n", "pthread_DataReading", "stopping", "");
#		endif
		pthread_exit(NULL);
	}

	void I2Csensor::DebugDataBuffer(void)
	{
		for(int page=0; 2>page; ++page)
		{
			for(int line=0x00; 0x80>line; line+=0x10)
			{
				fprintf(stderr, "  %d %02X\t%02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X\n", page, line
					,BUFFER_REGISTER(page,line+0x00),BUFFER_REGISTER(page,line+0x01),BUFFER_REGISTER(page,line+0x02),BUFFER_REGISTER(page,line+0x03)
					,BUFFER_REGISTER(page,line+0x04),BUFFER_REGISTER(page,line+0x05),BUFFER_REGISTER(page,line+0x06),BUFFER_REGISTER(page,line+0x07)
					,BUFFER_REGISTER(page,line+0x08),BUFFER_REGISTER(page,line+0x09),BUFFER_REGISTER(page,line+0x0A),BUFFER_REGISTER(page,line+0x0B)
					,BUFFER_REGISTER(page,line+0x0C),BUFFER_REGISTER(page,line+0x0D),BUFFER_REGISTER(page,line+0x0E),BUFFER_REGISTER(page,line+0x0F));
			}
		}
	}

};
