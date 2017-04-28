# Makefile

SUBDIRS = source stellarium--telescope_server gpio-i2c-sniffer
source_CCEXTRA =
stellarium--telescope_server_CCEXTRA =
gpio-i2c-sniffer_CCEXTRA =

CCFLAGS = -O4 -Wall -Wextra -Wno-unused-parameter -Werror -pthread -DDEBUG
LDFLAGS = -O4 -s -lstdc++ -pthread -lm

.PHONY: clean $(SUBDIRS)

.SUFFIXES: .cpp .o .obj .exe

.cpp.o:
	$(CC) $(CCFLAGS) $(CCEXTRA) $< -c -o $@

.cpp.obj:
	$(CROSS_GPP) $(CCFLAGS) $(CCEXTRA) $< -c -o $@

all: $(SUBDIRS)

test:

#main_SRCS := main.c try.c test.c
#lib_SRCS := lib.c api.c
#.SECONDEXPANSION:
#main lib: $$(patsubst %.c,%.o,$$($$@_SRCS))

clean:
	-for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
	-[ -d RTIMULib.build ] && rm -fR RTIMULib.build
	-[ -d i2c-tools ] && rm -fR i2c-tools

FORCE:

$(SUBDIRS): /usr/include/linux/i2c-dev.h /usr/include/pigpio.h /usr/include/RTIMULib.h
	$(MAKE) -C $@


/usr/lib/libpigpio.so /usr/include/pigpio.h: pigpio.installed

/usr/include/linux/i2c-dev.h: linux-i2c-dev-h.installed

/usr/include/RTIMULib.h: RTIMULib.built

linux-i2c-dev-h.installed: FORCE
	-sudo apt-get --yes install libi2c-dev
	grep -i i2c_smbus_read_i2c_block_data /usr/include/linux/i2c-dev.h && touch $@

pigpio.installed: FORCE
	-sudo apt-get --yes install pigpio
	[ -e /usr/include/pigpio.h -a -e /usr/lib/libpigpio.so ] && touch $@

linux-i2c-dev-h.built: FORCE
	[ -d i2c-tools ] || git clone https://github.com/groeck/i2c-tools.git
	prefix=/usr $(MAKE) -C i2c-tools install-lib install-include
	grep -i i2c_smbus_read_i2c_block_data /usr/include/linux/i2c-dev.h && touch $@

RTIMULib.built: FORCE
	[ -d RTIMULib ] || git clone https://github.com/RPi-Distro/RTIMULib.git
	(mkdir RTIMULib.build; cd RTIMULib.build; cmake -D BUILD_DRIVE=OFF -D BUILD_DRIVE10=OFF -D BUILD_DRIVE11=OFF -D CMAKE_INSTALL_PREFIX=/usr ../RTIMULib/Linux/)
	$(MAKE) -C RTIMULib.build RTIMULib RTIMULibCal
	sudo $(MAKE) -C RTIMULib.build install/strip
	sudo ldconfig
	[ -e /usr/include/RTIMULib.h -a -e /usr/lib/libRTIMULib.so ] && touch $@
