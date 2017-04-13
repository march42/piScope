/*	GPIO and I2C implementations, with I2C protcol sniffer
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

#include "gpio-i2c.h"

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <ctime>
#include <cstdarg>

#if defined(_WITH_MAIN_)
/*	I2C protcol sniffer
**	main routine
**		sniffer [SDA,SCL[,NAME]] ...
*/
class I2CSNIFFER //: protected GPIO_PIN
{
private:
	int SCL;
	GPIO_PIN* clockpin;
	int SDA;
	GPIO_PIN* datapin;
	char NAME[128];
	FILE* LOGFILE;
	int LOGLEVEL;
protected:
public:
	I2CSNIFFER(const char* arg1)
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s\n", "I2CSNIFFER constructor");
#endif
		this->SCL = -1;	this->clockpin = NULL;
		this->SDA = -1;	this->datapin = NULL;
		memset(&this->NAME[0], '\0', sizeof(this->NAME));
		size_t val = sscanf(arg1, "%d,%d,%s", &this->SCL, &this->SDA, &this->NAME[0]);
		if(3 > val)
		{
			//	no name specified
			sprintf(&this->NAME[0], "I2C-%d,%d", this->SCL, this->SDA);
		}
		//	init, if pins given
		if(-1 != this->SCL && -1 != this->SDA)
		{
			this->clockpin = new GPIO_PIN(this->SCL);
			this->datapin = new GPIO_PIN(this->SDA);
		}
		//	prepare log
		this->LOGFILE = NULL;
		this->LOGLEVEL = 3;
		//	done
#		if defined(DEBUG)
		fprintf(stdout, "sniffer:\t%s\tSCL=%d,SDA=%d\n", this->NAME,this->SCL,this->SDA);
#		endif
	}
	~I2CSNIFFER()
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "I2CSNIFFER destructor");
#endif		
		if(NULL != this->clockpin)
		{
			delete(this->clockpin);
		}
		if(NULL != this->datapin)
		{
			delete(this->datapin);
		}
		if(NULL != this->LOGFILE)
		{
			std::fclose(this->LOGFILE);
		}
	}
	bool valid(void)
	{
		return(-1 != this->SCL && NULL != this->clockpin && -1 != this->SDA && NULL != this->datapin);
	}
	bool good(void)
	{
		return(this->valid() && this->clockpin->gpioGood() && this->datapin->gpioGood());
	}

	int prepareGPIO(void)
	{
		sniffer->printLog(6,"%s:\tprepareGPIO (SCL=%d,SDA=%d)\n", sniffer->TimeStamp() ,this->SCL,this->SDA);
		int value = 0;	//	0==OK
		if(!this->valid())
		{
			return(PI_BAD_USER_GPIO);
		}
		else if(0 != (value=this->clockpin->gpioSetPullUpDown(PI_PUD_OFF)))
		{
			sniffer->printLog(0,"%s:\tprepareGPIO (%d==%s)\n", sniffer->TimeStamp() ,value,"clockpin->gpioSetPullUpDown(PI_PUD_OFF)");
		}
		else if(0 != (value=this->clockpin->gpioSetMode(PI_INPUT)))
		{
			sniffer->printLog(0,"%s:\tprepareGPIO (%d==%s)\n", sniffer->TimeStamp() ,value,"clockpin->gpioSetMode(PI_INPUT)");
		}
		else if(0 != (value=this->datapin->gpioSetPullUpDown(PI_PUD_OFF)))
		{
			sniffer->printLog(0,"%s:\tprepareGPIO (%d==%s)\n", sniffer->TimeStamp() ,value,"datapin->gpioSetPullUpDown(PI_PUD_OFF)");
		}
		else if(0 != (value=this->datapin->gpioSetMode(PI_INPUT)))
		{
			sniffer->printLog(0,"%s:\tprepareGPIO (%d==%s)\n", sniffer->TimeStamp() ,value,"datapin->gpioSetMode(PI_PUD_OFF)");
		}
		return(value);
	}

	/*	getting a useful timestamp
	**	uint32_t gpioTick(void)
	**		returns microseconds since system boot
	**		As tick is an unsigned 32 bit quantity it wraps around after 2^32 microseconds, which is approximately 1 hour 12 minutes.
	**	int gpioTime(unsigned timetype, int *seconds, int *micros)
	**		return 0=SUCCESS or PI_BAD_TIMETYPE
	**		timetype=PI_TIME_ABSOLUTE seconds since epoche, timetype=PI_TIME_RELATIVE seconds since library initialized
	*/
	const char* TimeStampUTC(void) const
	{
		time_t ts; time(&ts);
		static char value[20];
		strftime(&value[0],sizeof(value), "%04Y%02m%02d.%02H%02M%02S %Z", gmtime(&ts));
		return(&value[0]);
	}
	const char* TimeStamp(void) const
	{
		int sec = 0;
		int ysec = 0;
		::gpioTime(PI_TIME_RELATIVE, &sec,&ysec);
		static char value[20];
		sprintf(&value[0], "%04d.%06d", sec,ysec);
		return(&value[0]);
	}

	FILE* SetLogFile(const char* file=NULL)
	{
		this->LOGFILE = std::fopen((NULL==file ?"sniffer.log" :file),"a");
		return(this->LOGFILE);
	}
	/*	loglevel
	**	0	FEHLER
	**	1	WARNUNGEN
	**	2	START/STOP
	**	3	Daten
	**	4,5	Datenanalyse
	**	6,7	GPIO
	*/
	int SetLogLevel(int level=-1)
	{
		if(-1 != level)
		{
			this->LOGLEVEL = level;
		}
		return(this->LOGLEVEL);
	}
	int printLog(int level, const char * format, ... ) const
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s\t(%d<=%d)\n", "I2CSNIFFER printLog", level, this->LOGLEVEL);
#endif
		int written = 0;
		if(level <= this->LOGLEVEL)
		{
			va_list args;
			va_start(args, format);
			char message[200] = {0};
			written = snprintf(&message[0],sizeof(message), "%s:\t", this->TimeStamp());
			written += vsnprintf(&message[written],sizeof(message)-written, format, args);
			va_end(args);
			fprintf(stdout, "%s", message);
			if(NULL != this->LOGFILE)	fprintf(this->LOGFILE, "%s", message);
		}
		return(written);
	}

	int bbI2COpen(unsigned value)
	{
		sniffer->printLog(6,"%s:\topen GPIO for I2C communication (SCL=%d,SDA=%d,fSCL=%d)\n", sniffer->TimeStamp() ,this->SCL,this->SDA,value);
		//	Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_I2C_BAUD, or PI_GPIO_IN_USE.
		if(0 > this->SDA || 0 > this->SCL)
		{
			return(PI_BAD_USER_GPIO);
		}
		return(::bbI2COpen(this->SDA,this->SCL,value));
	}
	int bbI2CClose(void)
	{
		sniffer->printLog(6,"%s:\tclose GPIO from I2C communication\n", sniffer->TimeStamp());
		//	Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_I2C_BAUD, or PI_GPIO_IN_USE.
		if(0 > this->SDA || 0 > this->SCL)
		{
			return(PI_BAD_USER_GPIO);
		}
		return(::bbI2CClose(this->SDA));
	}
	int bbI2CZip(char *inBuf, unsigned inLen, char *outBuf, unsigned outLen)
	{
		sniffer->printLog(6,"%s:\tGPIO bitbanging I2C communication\n", sniffer->TimeStamp());
		/*	int bbI2CZip(char *inBuf, unsigned inLen, char *outBuf, unsigned outLen)
		**	Returns >= 0 if OK (the number of bytes read)
		**	otherwise PI_BAD_USER_GPIO, PI_NOT_I2C_GPIO, PI_BAD_POINTER, PI_BAD_I2C_CMD, PI_BAD_I2C_RLEN, PI_BAD_I2C_WLEN, PI_I2C_READ_FAILED, or PI_I2C_WRITE_FAILED
		**
		**	Name	Cmd & Data	Meaning
		**	End	0	No more commands
		**	Escape	1	Next P is two bytes
		**	Start	2	Start condition
		**	Stop	3	Stop condition
		**	Address	4 P	Set I2C address to P
		**	Flags	5 lsb msb	Set I2C flags to lsb + (msb << 8)
		**	Read	6 P	Read P bytes of data
		**	Write	7 P ...	Write P bytes of data
		*/
		if(0 > this->SDA || 0 > this->SCL)
		{
			return(PI_BAD_USER_GPIO);
		}
		return(::bbI2CZip(this->SDA, inBuf, inLen, outBuf, outLen));
	}

};
void main_usage(const char* error = NULL, const char* arg0 = __FILE__)
{
	fprintf(stderr, "%s (%s build %s %s)\n", arg0, __FILE__, __DATE__, __TIME__);
	fprintf(stderr, "usage:\t%s %s\n", arg0, "[SCL,SDA[,NAME]]" );
	fprintf(stderr, "\t%s %s\n", "SCL","is the GPIOx pin for clock" );
	fprintf(stderr, "\t%s %s\n", "SDA","is the GPIOx pin for data" );
	fprintf(stderr, "\t%s %s\n", "NAME","is the name to appear in output instead of GPIOx" );
	fprintf(stderr, "\t%s\n", "For some functions, this program needs to be run as root." );
	if(NULL != error)
	{
		fprintf(stderr, "\nerror:\t%s\n\n", error);
	}
}
static volatile bool keep_running = true;
#include <signal.h>
static void signal_handler(int signum)
{
	switch (signum)
	{
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			keep_running = false;
			break;
		default:
			// just ignore
			break;
	}
}
int main (int argc, char* argv[], char* envp[])
{
	fprintf(stdout, "%s (%s build %s)\n", argv[0], __FILE__, __DATE__);
	(void)envp;	//	unused variable envp
	//	check parameters
	if(2 > argc)
	{
		main_usage("no argument passed", argv[0]);
	}
	else
	{
		I2CSNIFFER* sniffer = new I2CSNIFFER(argv[1]);
		if(!sniffer->valid())
		{
			main_usage("invalid arguments passed", argv[0]);
		}
		else if(!sniffer->good())
		{
			main_usage("could not initialize", argv[0]);
		}
		else
		{
			//	prepare signal handler
			signal(SIGPIPE, SIG_IGN);	//	ignore SIGPIPE
			signal(SIGINT, signal_handler);
			signal(SIGTERM, signal_handler);
			signal(SIGQUIT, signal_handler);
			signal(SIGHUP, signal_handler);	//	maybe i should ignore SIGHUP???
			//	prepare log file
			sniffer->SetLogFile();	//	this will open stdout and sniffer.log
			sniffer->printLog(2,"%s:\t(%s) start sniffing for I2C communication\n", sniffer->TimeStamp(),sniffer->TimeStampUTC());
			//	prepare GPIO for sniffing
			sniffer->prepareGPIO();
			//	running loop
			while(keep_running)
			{
			}
			//	cleanup and exit now
			sniffer->printLog(2,"%s:\t(%s) stopp sniffing\n", sniffer->TimeStamp(),sniffer->TimeStampUTC());
		}
	}
	//	done
	return(0);
}
#endif

/*	I2C protocol electrical specifications
**	START condition: HIGH-LOW transition on SDA, while SCL is HIGH
**	STOP condition: LOW-HIGH transition on SDA, while SCL is HIGH
**	repeated START: generating START instead of STOP
**
**	Every byte put on the SDA line must be eight bits long.
**	The number of bytes that can be transmitted per transfer is unrestricted.
**	Each byte must be followed by an Acknowledge bit.
**	Data is transferred with the Most Significant Bit (MSB) first.
**	If a slave cannot receive or transmit another complete byte of data until it has performed some other function, for example
**		servicing an internal interrupt, it can hold the clock line SCL LOW to force the master into a wait state.
**		Data transfer then continues when the slave is ready for another byte of data and releases clock line SCL.
**
**	Acknowledge (ACK) and Not Acknowledge (NACK)
**		The acknowledge takes place after every byte.
**		The master generates all clock pulses, including the acknowledge ninth clock pulse.
**		The receiver pulls the SDA line LOW and it remains stable LOW during the HIGH period of this clock pulse.
**		When SDA remains HIGH during this ninth clock pulse, this is defined as the Not Acknowledge signal.
**		The master can then generate either a STOP condition to abort the transfer, or a repeated START condition to start a new transfer.
**
**	The slave address and R/-W bit
**		After the START condition (S), a slave address is sent.
**		This address is seven bits long followed by an eighth bit which is a data direction bit (R/W)
**			a ‘zero’ indicates a transmission (WRITE), a ‘one’ indicates a request for data (READ)
**		A data transfer is always terminated by a STOP condition (P) generated by the master.
**
**	timings (100kHz,400kHz)
**		f_SCL clock frequency 0-100kHz,0-400kHz
**		t_HD_STA hold time START condition >4ys,>0.6ys
**		t_LOW_SCL low period of SCL clock >4.7ys,>1.3yS
**		t_HIGH_SCL high period of SCL clock >4ys,>0.6ys
**		t_SU_SCL setup time for repeated START condition >4.7ys,>0.6ys
**		t_HD_SDA hold time for DATA >0
**		t_SU_SDA setup time for DATA >250ns,>100ns
**		t_SU_STO setup time for STOP condition >4ys,>0.6ys
**		t_BUF bus free time between STOP and START condition >4.7ys,>1.3ys
**	The maximum tHD;DAT could be 3.45 µs and 0.9 µs for Standard-mode and Fast-mode, but must be less than the maximum of tVD;DAT or tVD;ACK by a transition time.
**	This maximum must only be met if the device does not stretch the LOW period (tLOW) of the SCL signal.
**	If the clock stretches the SCL, the data must be valid by the set-up time before it releases the clock.
**		t_VD_DAT data valid time <3.45ys,<0.9ys
**		t_VD_ACK data valid time for ACK/NACK bit <3.45,<0.9ys
**
*/

	//	PIGPIO library
	static int PIGPIO_UseCount = 0;
	static int PIGPIO_Version = PI_INIT_FAILED;

	//	PIN assignments
#	define GPIO_USAGE_UNAVAIL 0
#	define GPIO_USAGE_SAFE 1
#	define GPIO_USAGE_UART 2
#	define GPIO_USAGE_I2C 4
#	define GPIO_USAGE_SPI 8
#	define GPIO_USAGE_I2C0 16
#	define GPIO_USAGE_PLUS 32
#	define GPIO_USAGE_PWM 64
#	define GPIO_USAGE_CONFIG GPIO_USAGE_UNAVAIL
	static int8_t GPIO_PIN_USAGE[] = {
		GPIO_USAGE_I2C0,GPIO_USAGE_I2C0,	/* GPIO0,GPIO1 are reserved for extension board identification */
		GPIO_USAGE_I2C,GPIO_USAGE_I2C,	/* GPIO2,GPIO3 are used for I2C bus1 */
		GPIO_USAGE_SAFE,	/* GPIO4 =GPCLK0 */
		GPIO_USAGE_SAFE|GPIO_USAGE_PLUS,GPIO_USAGE_SAFE|GPIO_USAGE_PLUS,	/* GPIO5=GPCLK1, GPIO6=GPCLK2 only on Model B boards */
		GPIO_USAGE_SPI,GPIO_USAGE_SPI,GPIO_USAGE_SPI,GPIO_USAGE_SPI,GPIO_USAGE_SPI,	/* GPIO7-GPIO11 designated to SPI bus0 */
		GPIO_USAGE_SAFE|GPIO_USAGE_PWM|GPIO_USAGE_PLUS,GPIO_USAGE_SAFE|GPIO_USAGE_PWM|GPIO_USAGE_PLUS,	/* GPIO12,GPIO13 only on Model B+ boards */
		GPIO_USAGE_UART,GPIO_USAGE_UART,	/* GPIO14=TxD0, GPIO14=RxD0 designated to UART 0 */
		GPIO_USAGE_SAFE|GPIO_USAGE_PLUS,	/* GPIO16 on Model A STATUS (ACT/OK) LED */
		GPIO_USAGE_SAFE,GPIO_USAGE_SAFE,	/* GPIO17,GPIO18 */
		GPIO_USAGE_SAFE|GPIO_USAGE_SPI|GPIO_USAGE_PLUS,	/* GPIO19=SPI-MISO only on Model B+ boards */
		GPIO_USAGE_SAFE|GPIO_USAGE_SPI|GPIO_USAGE_PLUS,	/* GPIO20=SPI-MOSI only on Model B+ boards */
		GPIO_USAGE_SPI|GPIO_USAGE_PLUS,	/* GPIO21=SPI-SCLK only on Model B+ boards, but was CAM_GPIO */
		GPIO_USAGE_SAFE,GPIO_USAGE_SAFE,	/* GPIO22,GPIO23 */
		GPIO_USAGE_SAFE,GPIO_USAGE_SAFE,	/* GPIO24,GPIO25 */
		GPIO_USAGE_SAFE|GPIO_USAGE_PLUS,	/* GPIO26 only on Model B+ boards */
		GPIO_USAGE_SAFE,	/* GPIO27 */
		GPIO_USAGE_CONFIG,GPIO_USAGE_CONFIG,GPIO_USAGE_CONFIG,GPIO_USAGE_CONFIG,	/* GPIO28-31 are CONFIG0-3 only on Model A */
		GPIO_USAGE_UNAVAIL,GPIO_USAGE_UNAVAIL,GPIO_USAGE_UNAVAIL,GPIO_USAGE_UNAVAIL,	/* GPIO32-35 */
		GPIO_USAGE_UNAVAIL,GPIO_USAGE_UNAVAIL,GPIO_USAGE_UNAVAIL,GPIO_USAGE_UNAVAIL,	/* GPIO36-39 */
		GPIO_USAGE_UNAVAIL,	/* GPIO40=PWM0_OUT */
		GPIO_USAGE_UNAVAIL,GPIO_USAGE_UNAVAIL,GPIO_USAGE_UNAVAIL,GPIO_USAGE_UNAVAIL,	/* GPIO41-44 */
		GPIO_USAGE_UNAVAIL,	/* GPIO45=PWM1_OUT */
		GPIO_USAGE_UNAVAIL,	/* GPIO46=HDMI_HPD_P */
		GPIO_USAGE_UNAVAIL,	/* GPIO47=SD_CARD_DET */
		GPIO_USAGE_UNAVAIL,	/* GPIO48=SD_CLK_R */
		GPIO_USAGE_UNAVAIL,	/* GPIO49=SD_CMD_R */
		GPIO_USAGE_UNAVAIL,	/* GPIO50=SD_DATA0_R */
		GPIO_USAGE_UNAVAIL,	/* GPIO51=SD_DATA1_R */
		GPIO_USAGE_UNAVAIL,	/* GPIO52=SD_DATA2_R */
		GPIO_USAGE_UNAVAIL,	/* GPIO53=SD_DATA3_R */
	};

	GPIO_PIN::GPIO_PIN(int pinnr)
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s\n", "GPIO_PIN constructor");
#endif
		this->gpiopin = -1;
		this->gpioInitialise();
		this->gpiopin = CheckGPIOPIN(pinnr);
	};
	GPIO_PIN::~GPIO_PIN()
	{
		this->gpioTerminate();
	};

	int gpioGetMode(void)
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s\n", "GPIO_PIN gpioGetMode");
#endif
		//	0==OK, PI_BAD_GPIO==error
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioGetMode(this->gpiopin));
	}
	int gpioSetMode(unsigned value)
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioSetMode", value);
#endif
		//	Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_MODE.
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioSetMode(this->gpiopin,value));
	}
	int gpioSetPullUpDown(unsigned value)
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioSetPullUpDown", value);
#endif
		//	0==OK, PI_BAD_GPIO==pin, PI_BAD_PUD==error
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioSetPullUpDown(this->gpiopin,value));
	}
	int gpioRead(void)
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s\n", "GPIO_PIN gpioRead");
#endif
		//	0==OK, PI_BAD_GPIO==pin
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioRead(this->gpiopin));
	}
	int gpioWrite(unsigned value)
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioWrite", value);
#endif
		//	0==OK, PI_BAD_GPIO==pin, PI_BAD_LEVEL==error
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioWrite(this->gpiopin,value));
	}

	bool GPIO_PIN::gpioGood(void) const
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioGood", this->gpiopin);
#endif
		assert(0 < PIGPIO_UseCount);	//	init/terminate within constructor/destructor
		return(PI_INIT_FAILED != PIGPIO_Version && 0 <= this->gpiopin);
	}

	int GPIO_PIN::CheckGPIOPIN(int pinnr) const
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN CheckGPIOPIN", pinnr);
#endif
		int value = -1;	//	DEFAULT return value PIN=INVALID
		if( 0 <= pinnr && sizeof(GPIO_PIN_USAGE) > (unsigned int)pinnr && GPIO_USAGE_UNAVAIL != GPIO_PIN_USAGE[pinnr] )
		{
			value = pinnr;
		}
		return(value);
	};

	void GPIO_PIN::gpioInitialise(void) const
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioInitialise", PIGPIO_UseCount);
#endif
		if(0 > PIGPIO_UseCount)
		{
			//	ERROR, this should never happen
			throw std::out_of_range("PIGPIO_UseCount below zero (in gpioInitialise)");
		}
		//	check and increment
		else if(0 == PIGPIO_UseCount++)
		{
			//	set safe parameters for different hardware
			/*	DEFAULTS are set in PIGPIO library
			unsigned hwrev = ::gpioHardwareRevision();
			if(16 <= hwrev)
			{
				::gpioCfgPermissions(PI_DEFAULT_UPDATE_MASK_R3);
			}
			else if(15 == hwrev || 4 == hwrev || 5 == hwrev || 6 == hwrev)
			{
				::gpioCfgPermissions(PI_DEFAULT_UPDATE_MASK_A_B2);
			}
			else if(2 == hwrev || 3 == hwrev)
			{
				::gpioCfgPermissions(PI_DEFAULT_UPDATE_MASK_B1);
			}
			*/
			//	configure access interfaces
			::gpioCfgInterfaces(PI_DISABLE_FIFO_IF | PI_DISABLE_SOCK_IF|PI_LOCALHOST_SOCK_IF);
			//::gpioCfgSocketPort(8888);	//	DEFAULT socket port =8888
			//	call PIGPIO init
			PIGPIO_Version = ::gpioInitialise();
		}
		assert(0 < PIGPIO_UseCount);
	};
	void GPIO_PIN::gpioTerminate(void) const
	{
#if defined(TRACE)
		fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioTerminate", PIGPIO_UseCount);
#endif
		//	decrement and check
		assert(0 < PIGPIO_UseCount);
		if(0 > --PIGPIO_UseCount)
		{
			//	ERROR, this should never happen
			throw std::out_of_range("PIGPIO_UseCount below zero (in gpioTerminate)");
		}
		else if(0 == PIGPIO_UseCount)
		{
			//	no further usage, so terminate
			::gpioTerminate();
			PIGPIO_Version = PI_INIT_FAILED;
		}
		assert(0 <= PIGPIO_UseCount);
	};

