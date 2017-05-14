/*
**	config (.h)
**	configuration of project and build process
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

/*!	\brief	configuration macros
**
**	Declaration of configuration macros.
**/

#ifndef _CONFIG_H_

	/*	DEBUG,TRACE,...
	**	DEBUG	debug build
	**	NDEBUG	release build
	**	TRACE	additional debug outputs for tracing
	**	DEBUG1,DEBUG2,DEBUG3,DEBUG4,DEBUG5,DEBUG6,DEBUG7,DEBUG8,DEBUG9	additional debugging outputs
	**	DEBUG2	test-programs.cpp (test_i2csensor) used to output read sensor values
	**	DEBUG3	test-programs.cpp (test_i2csensor) used to output read sensor values
	**	DEBUG4	IMU class tracing output
	**	DEBUG5	IMU class debug buffer output
	*/
#	if !defined(NDEBUG)
//#		define TRACE true
//#		define DEBUG1 true
//#		define DEBUG2 true
//#		define DEBUG3 true
//#		define DEBUG4 true
//#		define DEBUG5 true
//#		define DEBUG6 true
//#		define DEBUG7 true
//#		define DEBUG8 true
//#		define DEBUG9 true
//#		define DEBUG0 true
#	endif

	/*	USE_LINUX_I2CDEV
	**	if I2C tools package and corresponding i2c-dev.h is installed
	**	otherwise the local copy of i2c-dev.h is used.
	*/
#	define USE_LINUX_I2CDEV true

	/*	USE_MADGWICK_AHRS
	**	use Madgwick AHRS code for sensor filtering and fusion
	*/
#	define USE_MADGWICK_AHRS true

	/*	USE_RTIMULIB
	**	use RTIMULib to interface IMU sensor
	*/
#	define USE_RTIMULIB true
	/*	CALCULATE_ORIENTATION
	**	use own calculation routine and maths, instead of relying on RTIMULib
	*/
#	define CALCULATE_ORIENTATION true
	/*	DONT_OPTIMIZE_TIMESTAMP
	**	rely on RTIMULib time stamp in micro seconds and just divide by 1000000
	*/
//#	define DONT_OPTIMIZE_TIMESTAMP true
	/*	USE_TIMESTAMP_IMUNOTMOVING
	**	check time stamp of last orientation, to be no older than x seconds
	**	when checking for not moving
	*/
//#	define USE_TIMESTAMP_IMUNOTMOVING true

#endif
