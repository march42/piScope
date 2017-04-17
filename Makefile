# Makefile

SUBDIRS = source stellarium--telescope_server gpio-i2c-sniffer
source_CCEXTRA = -DUSE_OWN_I2CDEV
stellarium--telescope_server_CCEXTRA = 
gpio-i2c-sniffer_CCEXTRA = 

CCFLAGS = -O4 -Wall -Wextra -Wno-unused-parameter -Werror -pthread -DDEBUG
LDFLAGS = -O4 -s -lstdc++ -pthread -lm

.PHONY: all clean $(SUBDIRS)

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

/usr/lib/libpigpio.so /usr/include/pigpio.h:
	git clone https://github.com/joan2937/pigpio
	$(MAKE) -C $@ all
	sudo prefix=/usr $(MAKE) -C $@ install

$(SUBDIRS): /usr/include/pigpio.h
	$(MAKE) -C $@
