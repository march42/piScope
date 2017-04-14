/*	GPIO and I2C implementations
**
**	(C) Copyright 2017 by Marc Hefter <marchefter@march42.net>
**
**	Pin numbering is always hardware based on chip not on connector
**
**	This program is free software; you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation; either version 2 of the License, or
**	(at your option) any later version.
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**	You should have received a copy of the GNU General Public License
**	along with this program; if not, write to the Free Software
**	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
**	MA 02110-1301 USA.
*/

#if !defined(_GPIO_I2C_H_)
#	define _GPIO_I2C_H_

#	if defined(_GPIO_PIGPIO_)
#		include <pigpio.h>
#	elif defined(_GPIO_SYSFS_)
#		define _GPIO_SYSFS_PATH "/sys/class/gpio/"
#		define _GPIO_SYSFS_PINFMT (_GPIO_SYSFS_PATH "gpio" "%d")
#	endif

class GPIO_PIN
{
public:	/* public members are accessible from anywhere */
	//	constructor, destructor
	GPIO_PIN(int pinnr=-1);
	~GPIO_PIN();
	//	PIN preparation and setup
	int gpioGetMode(void);
	int gpioSetMode(unsigned value);
	int gpioSetPullUpDown(unsigned value);
	//	PIN manipulation
	int gpioRead(void);
	int gpioWrite(unsigned value);
	int gpioTrigger(int user_gpio = -1, unsigned pulseLen, unsigned level = 1);
	//	bank manipulation
	uint32_t gpioRead_Bits_0_31(void);
	uint32_t gpioRead_Bits_32_53(void);
	int gpioWrite_Bits_0_31_Clear(uint32_t value);
	int gpioWrite_Bits_32_53_Clear(uint32_t value);
	int gpioWrite_Bits_0_31_Set(uint32_t value);
	int gpioWrite_Bits_32_53_Set(uint32_t value);
	//	timing
	int gpioTime(unsigned timetype, int *seconds, int *micros);
	int gpioSleep(unsigned timetype, int seconds, int micros);
	uint32_t gpioDelay(uint32_t micros);
	uint32_t gpioTick(void);
	//	notfication
	int gpioNotifyOpen(void);	//	buffers handle to this->notifyHandle
	int gpioNotifyClose(int handle = PI_BAD_HANDLE);
	int gpioNotifyBegin(int handle = PI_BAD_HANDLE, uint32_t bits);
	int gpioNotifyPause(int handle = PI_BAD_HANDLE);
	//	events
	int eventMonitor(int handle = PI_BAD_HANDLE, uint32_t bits);
	int eventSetFunc(unsigned event, eventFunc_t fnc);
	int eventSetFuncEx(unsigned event, eventFuncEx_t fnc, void *userdata = NULL);
	int eventTrigger(unsigned event);
	//	alert
	int gpioSetAlertFunc(int user_gpio = -1, gpioAlertFunc_t fnc);
	int gpioSetAlertFuncEx(int user_gpio = -1, gpioAlertFuncEx_t fnc, void *userdata = NULL);
	//	interrupt
	int gpioSetISRFunc(int user_gpio = -1, unsigned edge, int timeout, gpioISRFunc_t fnc);
	int gpioSetISRFuncEx(int user_gpio = -1, unsigned edge, int timeout, gpioISRFuncEx_t fnc, void *userdata = NULL);
	//	status checking
	bool gpioGood(void) const;

protected:	/* protected members are accessible from the same class or "friends" and derived classes */
	//	GPIO PIN number handling
	int gpiopin;	//	-1=INVALID
	int CheckGPIOPIN(int pinnr=-1) const;
	//	internal notification
	int notifyHandle;

private:	/* private members are accessible only from within the same class or "friends" */
	//	PIGPIO init/terminate
	void gpioInitialise(void) const;
	void gpioTerminate(void) const;
};

#endif
