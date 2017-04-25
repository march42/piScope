# Makefile

SUBDIRS = source stellarium--telescope_server gpio-i2c-sniffer
source_CCEXTRA = -DUSE_OWN_I2CDEV
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

linux-i2c-dev-h.installed: FORCE
	-[ -e /usr/include/linux/i2c-dev.h ] || sudo apt-get --yes install linux-libc-dev
	[ -e /usr/include/linux/i2c-dev.h ] && touch $@

/usr/include/linux/i2c-dev.h: linux-i2c-dev-h.installed

pigpio.installed: FORCE
	-[ -e /usr/include/pigpio.h -a -e /usr/lib/libpigpio.so ] || sudo apt-get --yes install pigpio
	[ -e /usr/include/pigpio.h -a -e /usr/lib/libpigpio.so ] && touch $@

/usr/lib/libpigpio.so /usr/include/pigpio.h: pigpio.installed

#main_SRCS := main.c try.c test.c
#lib_SRCS := lib.c api.c
#.SECONDEXPANSION:
#main lib: $$(patsubst %.c,%.o,$$($$@_SRCS))

clean:
	-for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done

$(SUBDIRS): /usr/include/linux/i2c-dev.h /usr/include/pigpio.h
	$(MAKE) -C $@
