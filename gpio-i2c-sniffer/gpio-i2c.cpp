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
#	include <deque>
#	include <pthread.h>

/*	I2C protcol sniffer
**	main routine
**		sniffer [SDA,SCL[,NAME]] ...
*/
void *pthread_main(void *data);	//	prototype
class I2CSNIFFER //: protected GPIO_PIN
{
private:	/* private members are accessible only from within the same class or "friends" */
	//	GPIO access
	int SDA;
	GPIO_PIN* datapin;
	int SCL;
	GPIO_PIN* clockpin;
	//	logging
	char NAME[128];
	FILE* LOGFILE;
	int LOGLEVEL;
	//	threading
	pthread_t pthread_sniffing;
	pthread_attr_t pthread_attributes;
	bool pthread_stopping;
	void pthread_stopp(void)
	{
		this->printLog(9,"pthread_stopp started\n");
		//	set stopping flag
		this->pthread_stopping = true;
		//	destroy attribute
		pthread_attr_destroy(&this->pthread_attributes);
		//	wait for thread completion
		pthread_join(this->pthread_sniffing, NULL);
		this->printLog(9,"pthread_stopp done\n");
	}
	friend void *pthread_main(void *data);
protected:	/* protected members are accessible from the same class or "friends" and derived classes */
public:	/* public members are accessible from anywhere */
	I2CSNIFFER(const char* arg1)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "I2CSNIFFER constructor");
#endif
		//	clear/init values
		this->SDA = -1;	this->datapin = NULL;
		this->SCL = -1;	this->clockpin = NULL;
		memset(&this->NAME[0], '\0', sizeof(this->NAME));
		this->pthread_sniffing = 0;
		this->pthread_stopping = true;
		//	check argument
		size_t val = sscanf(arg1, "%d,%d,%s", &this->SDA, &this->SCL, &this->NAME[0]);
		if(3 > val)
		{
			//	no name specified
			sprintf(&this->NAME[0], "I2C-%d,%d", this->SDA, this->SCL);
		}
		//	init, if pins given
		if(-1 != this->SDA && -1 != this->SCL)
		{
			this->datapin = new GPIO_PIN(this->SDA);
			this->clockpin = new GPIO_PIN(this->SCL);
		}
		//	prepare log
		this->LOGFILE = NULL;
		this->LOGLEVEL = 3;
		//	done
#		if defined(DEBUG)
		std::fprintf(stdout, "sniffer:\t%s\tSDA=%d,SCL=%d\n", this->NAME,this->SDA,this->SCL);
#		endif
	}
	~I2CSNIFFER()
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "I2CSNIFFER destructor");
#endif		
		if(0 != this->pthread_sniffing)
		{
			this->pthread_stopp();
		}
		if(NULL != this->datapin)
		{
			delete(this->datapin);
		}
		if(NULL != this->clockpin)
		{
			delete(this->clockpin);
		}
		if(NULL != this->LOGFILE)
		{
			std::fclose(this->LOGFILE);
		}
	}
	bool valid(void)
	{
		return(-1 != this->SDA && NULL != this->datapin && -1 != this->SCL && NULL != this->clockpin);
	}
	bool good(void)
	{
		return(this->valid() && this->datapin->gpioGood() && this->clockpin->gpioGood());
	}

	//	start reading thread
	int pthread_start(void)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "I2CSNIFFER pthread_start");
#endif
		//	prepare thread attributes
		pthread_attr_init(&this->pthread_attributes);
		pthread_attr_setdetachstate(&this->pthread_attributes, PTHREAD_CREATE_JOINABLE);
		//	start thread
		this->pthread_stopping = false;
		int rc = pthread_create(&this->pthread_sniffing, &this->pthread_attributes, pthread_main, (void*)this);
		if(0 > rc)
		{
			this->printLog(0,"pthread_create failed (%d==%s)\n", rc,"pthread_main");
		}
		else
		{
			this->printLog(9,"pthread_create started (%s)\n", "pthread_main");
			sleep(1);	//	give time to start the thread
		}
		return(rc);
	}

	const char* GetName(void) const
	{
		return(&this->NAME[0]);
	}

	int prepareGPIO(void)
	{
		this->printLog(6,"prepareGPIO (SDA=%d,SCL=%d)\n", this->SDA,this->SCL);
		int value = 0;	//	0==OK
		if(!this->valid())
		{
			return(PI_BAD_USER_GPIO);
		}
		else if(0 != (value=this->datapin->gpioSetPullUpDown(PI_PUD_OFF)))
		{
			this->printLog(0,"prepareGPIO (%d==%s)\n", value,"datapin->gpioSetPullUpDown(PI_PUD_OFF)");
		}
		else if(0 != (value=this->datapin->gpioSetMode(PI_INPUT)))
		{
			this->printLog(0,"prepareGPIO (%d==%s)\n", value,"datapin->gpioSetMode(PI_PUD_OFF)");
		}
		else if(0 != (value=this->clockpin->gpioSetPullUpDown(PI_PUD_OFF)))
		{
			this->printLog(0,"prepareGPIO (%d==%s)\n", value,"clockpin->gpioSetPullUpDown(PI_PUD_OFF)");
		}
		else if(0 != (value=this->clockpin->gpioSetMode(PI_INPUT)))
		{
			this->printLog(0,"prepareGPIO (%d==%s)\n", value,"clockpin->gpioSetMode(PI_INPUT)");
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
		::gpioTime(PI_TIME_RELATIVE, &sec,&ysec);	//	direct calling, if clockpin,datapin not initialized before
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
	**	8,9	INFO
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
		std::fprintf(stderr, "TRACE:\t%s\t(%d<=%d)\n", "I2CSNIFFER printLog", level, this->LOGLEVEL);
#endif
		int written = 0;
		if(level <= this->LOGLEVEL)
		{
			va_list args;
			va_start(args, format);
			char message[200] = {0};
			written = snprintf(&message[0],sizeof(message), "%s:\t%s\t", this->TimeStamp(), this->GetName());
			written += vsnprintf(&message[written],sizeof(message)-written, format, args);
			va_end(args);
			std::fprintf(stdout, "%s", message);
			if(NULL != this->LOGFILE)	std::fprintf(this->LOGFILE, "%s", message);
		}
		return(written);
	}

	int bbI2COpen(unsigned value)
	{
		this->printLog(6,"open GPIO for I2C communication (SDA=%d,SCL=%d,fSCL=%d)\n", this->SDA,this->SCL,value);
		//	Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_I2C_BAUD, or PI_GPIO_IN_USE.
		if(0 > this->SDA || 0 > this->SCL)
		{
			return(PI_BAD_USER_GPIO);
		}
		return(::bbI2COpen(this->SDA,this->SCL,value));
	}
	int bbI2CClose(void)
	{
		this->printLog(6,"close GPIO from I2C communication\n");
		//	Returns 0 if OK, otherwise PI_BAD_USER_GPIO, PI_BAD_I2C_BAUD, or PI_GPIO_IN_USE.
		if(0 > this->SDA || 0 > this->SCL)
		{
			return(PI_BAD_USER_GPIO);
		}
		return(::bbI2CClose(this->SDA));
	}
	int bbI2CZip(char *inBuf, unsigned inLen, char *outBuf, unsigned outLen)
	{
		this->printLog(6,"GPIO bitbanging I2C communication\n");
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

void main_usage(const char* error = NULL, const char* arg0 = __FILE__, const char* argX = NULL)
{
	std::fprintf(stderr, "%s (%s build %s %s)\n", arg0, __FILE__, __DATE__, __TIME__);
	std::fprintf(stderr, "usage:\t%s %s %s\n", arg0, "[-l<file>]", "SDA,SCL[,NAME]" );
	std::fprintf(stderr, "\t%s %s\n", "-l<file>","use <file> as log (DEFAULT=sniffer.log)" );
	std::fprintf(stderr, "\t%s %s\n", "-L<level>","maximum level to log (DEFAULT=3, DEBUG=9)" );
	std::fprintf(stderr, "\t%s %s\n", "SDA","is the GPIOx pin for data" );
	std::fprintf(stderr, "\t%s %s\n", "SCL","is the GPIOx pin for clock" );
	std::fprintf(stderr, "\t%s %s\n", "NAME","is the name to appear in output instead of GPIOx" );
	std::fprintf(stderr, "\t%s\n", "For some functions, this program needs to be run as root." );
	if(NULL != error && NULL != argX)
	{
		std::fprintf(stderr, "\nerror:\t%s (%s)\n\n", error, argX);
	}
	else if(NULL != error)
	{
		std::fprintf(stderr, "\nerror:\t%s\n\n", error);
	}
}

static volatile bool keep_running = true;
#include <signal.h>
static void signal_handler(int signum)
{
#if defined(TRACE)
	std::fprintf(stderr, "TRACE:\t%s(%d)\n", "signal_handler", signum);
#endif
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

void *pthread_main(void *data)
{
	I2CSNIFFER* sniffer = (I2CSNIFFER*)data;	//	mother
	sniffer->printLog(9,"pthread_main started\n");
	//	running main work loop
	while(keep_running && !sniffer->pthread_stopping)
	{
		sniffer->printLog(8,"pthread_main running\n");
		usleep(1);
	}
	//	cleaning up
	sniffer->pthread_sniffing = 0;
	sniffer->printLog(9,"pthread_main stopped\n");
	pthread_exit(NULL);
}

int main (int argc, char* argv[], char* envp[])
{
	std::fprintf(stdout, "%s (%s build %s)\n", argv[0], __FILE__, __DATE__);
	(void)envp;	//	unused variable envp
	//	check parameters
	int argp = 1;	//	first parameter
	if(2 > argc)
	{
		main_usage("no argument passed", argv[0]);
	}
	else
	{
		//	parameters specified, so check them out
		std::deque<I2CSNIFFER*> snifferline;	//	queue of sniffers
		const char* logfile = NULL;
		int loglevel = 3;	//	DEFAULT log level
#		if defined(DEBUG)
		loglevel = 9;	//	this will set maximum logging
#		endif
		for(argp=1; argp<argc; ++argp)
		{
			if(0 == std::strncmp(argv[argp], "-l", 2))
			{
				//	-l<filename>
				logfile = argv[argp] +2;	//	remember logfile name
			}
			else if(0 == std::strncmp(argv[argp], "-L", 2))
			{
				//	-L<level>
				if(1 != sscanf(argv[argp] +2, "%d", &loglevel))
				{
					main_usage("invalid loglevel passed", argv[0], argv[argp]);
				}
			}
			else if(NULL != std::strchr(argv[argp], ','))
			{
				//	SDA,SCL[,NAME]
				snifferline.push_back(new I2CSNIFFER(argv[argp]));
				if(!snifferline.back()->valid())
				{
					main_usage("invalid arguments passed", argv[0], snifferline.back()->GetName());
					break;
				}
				else if(!snifferline.back()->good())
				{
					main_usage("could not initialize", argv[0], snifferline.back()->GetName());
					break;
				}
			}
			else //if(argv[argp])
			{
				main_usage("invalid argument passed", argv[0], argv[argp]);
				break;
			}
		}
		//	break, if error
		if(!keep_running || argp != argc)
		{
			snifferline.clear();
		}
		//	now prepare the sniffers
		for(size_t pos=0; pos<snifferline.size(); ++pos)
		{
			I2CSNIFFER* sniffer = snifferline[pos];
			//	prepare log file
			sniffer->SetLogFile(logfile);	//	this will open stdout and sniffer.log
			sniffer->SetLogLevel(loglevel);	//	this will set highest logging
			sniffer->printLog(2,"(%s) start sniffing for I2C communication\n", sniffer->TimeStampUTC());
			//	prepare GPIO for sniffing
			sniffer->prepareGPIO();
		}
		//	break, if error
		if(!keep_running)
		{
			snifferline.clear();
		}
		else
		{
			//	prepare signal handler
			sighandler_t shnd = SIG_ERR;
			if(SIG_ERR == (shnd = signal(SIGPIPE, SIG_IGN)))	//	ignore SIGPIPE
			{
				std::fprintf(stderr, "signal failed (%s)\n", "SIGPIPE");
			}
			if(SIG_ERR == (shnd = signal(SIGINT, signal_handler)))
			{
				std::fprintf(stderr, "signal failed (%s)\n", "SIGINT");
			}
			if(SIG_ERR == (shnd = signal(SIGTERM, signal_handler)))
			{
				std::fprintf(stderr, "signal failed (%s)\n", "SIGTERM");
			}
			if(SIG_ERR == (shnd = signal(SIGQUIT, signal_handler)))
			{
				std::fprintf(stderr, "signal failed (%s)\n", "SIGQUIT");
			}
			if(SIG_ERR == (shnd = signal(SIGHUP, signal_handler)))	//	maybe i should ignore SIGHUP???
			{
				std::fprintf(stderr, "signal failed (%s)\n", "SIGHUP");
			}
		}
		//	now start the sniffers
		for(size_t pos=0; pos<snifferline.size(); ++pos)
		{
			I2CSNIFFER* sniffer = snifferline[pos];
			//	running loop
			sniffer->pthread_start();
		}
		//	wait for termination
		while(keep_running && !snifferline.empty())
		{
			sleep(1);
		}
		while(!snifferline.empty())
		{
			I2CSNIFFER* sniffer = snifferline.front();
			sniffer->printLog(2,"(%s) stopp sniffing\n", sniffer->TimeStampUTC());
			//	cleanup and exit now
			snifferline.pop_front();
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
		std::fprintf(stderr, "TRACE:\t%s\n", "GPIO_PIN constructor");
#endif
		this->gpiopin = -1;
		this->gpioInitialise();
		this->gpiopin = CheckGPIOPIN(pinnr);
		this->notifyHandle = PI_NO_HANDLE;
	};
	GPIO_PIN::~GPIO_PIN()
	{
		this->gpioTerminate();
	};

	int GPIO_PIN::gpioGetMode(void)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "GPIO_PIN gpioGetMode");
#endif
		//	0==OK, PI_BAD_GPIO==error
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioGetMode(this->gpiopin));
	}
	int GPIO_PIN::gpioSetMode(unsigned value)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioSetMode", value);
#endif
		//	Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_MODE.
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioSetMode(this->gpiopin,value));
	}
	int GPIO_PIN::gpioSetPullUpDown(unsigned value)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioSetPullUpDown", value);
#endif
		//	0==OK, PI_BAD_GPIO==pin, PI_BAD_PUD==error
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioSetPullUpDown(this->gpiopin,value));
	}
	int GPIO_PIN::gpioRead(void)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "GPIO_PIN gpioRead");
#endif
		//	0==OK, PI_BAD_GPIO==pin
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioRead(this->gpiopin));
	}
	int GPIO_PIN::gpioWrite(unsigned value)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioWrite", value);
#endif
		//	0==OK, PI_BAD_GPIO==pin, PI_BAD_LEVEL==error
		if(0 > this->gpiopin)
		{
			return(PI_BAD_GPIO);
		}
		return(::gpioWrite(this->gpiopin,value));
	}

	int GPIO_PIN::gpioTrigger(int user_gpio, unsigned pulseLen, unsigned level)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioTrigger", user_gpio);
#endif
		return(::gpioTrigger( (-1==user_gpio ?this->gpiopin :user_gpio) ,pulseLen,level));
	}

	uint32_t GPIO_PIN::gpioRead_Bits_0_31(void)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "gpioRead_Bits_0_31");
#endif
		return(::gpioRead_Bits_0_31());
	}
	uint32_t GPIO_PIN::gpioRead_Bits_32_53(void)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "gpioRead_Bits_32_53");
#endif
		return(::gpioRead_Bits_32_53());
	}
	int GPIO_PIN::gpioWrite_Bits_0_31_Clear(uint32_t value)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %08X\n", "gpioWrite_Bits_0_31_Clear", value);
#endif
		return(::gpioWrite_Bits_0_31_Clear(value));
	}
	int GPIO_PIN::gpioWrite_Bits_32_53_Clear(uint32_t value)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %08X\n", "gpioWrite_Bits_32_53_Clear", value);
#endif
		return(::gpioWrite_Bits_32_53_Clear(value));
	}
	int GPIO_PIN::gpioWrite_Bits_0_31_Set(uint32_t value)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %08X\n", "gpioWrite_Bits_0_31_Set", value);
#endif
		return(::gpioWrite_Bits_0_31_Set(value));
	}
	int GPIO_PIN::gpioWrite_Bits_32_53_Set(uint32_t value)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %08X\n", "gpioWrite_Bits_32_53_Set", value);
#endif
		return(::gpioWrite_Bits_32_53_Set(value));
	}

	int GPIO_PIN::gpioTime(unsigned timetype, int *seconds, int *micros)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "gpioTime");
#endif
		return(::gpioTime(timetype,seconds,micros));
	}
	int GPIO_PIN::gpioSleep(unsigned timetype, int seconds, int micros)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s (%d.%06d)\n", "gpioSleep" ,seconds,micros);
#endif
		return(::gpioSleep(timetype,seconds,micros));
	}
	uint32_t GPIO_PIN::gpioDelay(uint32_t micros)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s (%d)\n", "gpioDelay" ,micros);
#endif
		return(::gpioDelay(micros));
	}
	uint32_t GPIO_PIN::gpioTick(void)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "gpioTick");
#endif
		return(::gpioTick());
	}

	int GPIO_PIN::gpioNotifyOpen(void)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s\n", "gpioNotifyOpen");
#endif
		if(PI_NO_HANDLE == this->notifyHandle)
		{
			this->notifyHandle = ::gpioNotifyOpen();
		}
		return(this->notifyHandle);
	}
	int GPIO_PIN::gpioNotifyClose(int handle)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s (%d)\n", "gpioNotifyClose" ,handle);
#endif
		int rc = PI_NO_HANDLE;
		if(PI_NO_HANDLE != handle)
		{
			rc == ::gpioNotifyClose(handle);
		}
		else if(PI_NO_HANDLE != this->notifyHandle)
		{
			rc == ::gpioNotifyClose(this->notifyHandle);
			this->notifyHandle = PI_NO_HANDLE;
		}
		return(rc);
	}
	int GPIO_PIN::gpioNotifyBegin(int handle, uint32_t bits)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s (%d,%b)\n", "gpioNotifyBegin" ,handle,bits);
#endif
		return(::gpioNotifyBegin( (PI_NO_HANDLE==handle ?this->notifyHandle :handle) ,bits));
	}
	int GPIO_PIN::gpioNotifyPause(int handle)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s (%d)\n", "gpioNotifyPause" ,handle);
#endif
		return(::gpioNotifyPause( (PI_NO_HANDLE==handle ?this->notifyHandle :handle) ));
	}

	int GPIO_PIN::gpioSetAlertFunc(int user_gpio, gpioAlertFunc_t fnc)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s (%d)\n", "gpioSetAlertFunc" ,user_gpio);
#endif
		return(::gpioSetAlertFunc( (-1==user_gpio ?this->gpiopin :user_gpio) ,fnc));
	}
	int GPIO_PIN::gpioSetAlertFuncEx(int user_gpio, gpioAlertFuncEx_t fnc, void *userdata)
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s (%d)\n", "gpioSetAlertFuncEx" ,user_gpio);
#endif
		return(::gpioSetAlertFuncEx( (-1==user_gpio ?this->gpiopin :user_gpio) ,fnc, (NULL==userdata ?this :userdata) ));
	}

	int GPIO_PIN::gpioSetISRFunc(int user_gpio, unsigned edge, int timeout, gpioISRFunc_t fnc);
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s (%d)\n", "gpioSetISRFunc" ,user_gpio);
#endif
		return(::gpioSetISRFunc( (-1==user_gpio ?this->gpiopin :user_gpio) ,edge,timeout,fnc));
	}
	int GPIO_PIN::gpioSetISRFuncEx(int user_gpio, unsigned edge, int timeout, gpioISRFuncEx_t fnc, void *userdata);
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s (%d)\n", "gpioSetISRFuncEx" ,user_gpio);
#endif
		return(::gpioSetISRFuncEx( (-1==user_gpio ?this->gpiopin :user_gpio) ,edge,timeout,fnc, (NULL==userdata ?this :userdata) ));
	}

	bool GPIO_PIN::gpioGood(void) const
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioGood", this->gpiopin);
#endif
		assert(0 < PIGPIO_UseCount);	//	init/terminate within constructor/destructor
		return(PI_INIT_FAILED != PIGPIO_Version && 0 <= this->gpiopin);
	}

	int GPIO_PIN::CheckGPIOPIN(int pinnr) const
	{
#if defined(TRACE)
		std::fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN CheckGPIOPIN", pinnr);
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
		std::fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioInitialise", PIGPIO_UseCount);
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
		std::fprintf(stderr, "TRACE:\t%s %d\n", "GPIO_PIN gpioTerminate", PIGPIO_UseCount);
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

