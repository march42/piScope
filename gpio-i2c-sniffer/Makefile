# Makefile

LIBRARIES_CPP = gpio-i2c.cpp
LIBRARIES_O = $(LIBRARIES_CPP:.cpp=.o)
LIBRARIES_OBJ = $(LIBRARIES_CPP:.cpp=.obj)

CCFLAGS = -O4 -Wall -Wextra -Wno-unused-parameter -Werror -pthread -DDEBUG
# -DNDEBUG	to disable assert
CCFLAGS += $(CCEXTRA)
LDFLAGS = -O4 -s -lstdc++ -pthread

#	(DEFAULT) use operating systems /usr/include/pigpio.h
CCFLAGS += -D_GPIO_PIGPIO_
LDFLAGS += -lpigpio
#	use sysfs functionality
#CCFLAGS += -D_GPIO_SYSFS_

ifeq ($(OS),Windows_NT)
	RM = cmd //C del //Q //F
	RRM = cmd //C rmdir //Q //S
else
	RM = rm -f
	RRM = rm -f -r
endif

.SUFFIXES: .cpp .o .obj

.cpp.o:
	$(CC) $(CCFLAGS) $< -c -o $@

.cpp.obj:
	$(CROSS_GPP) $(CCFLAGS) $< -c -o $@

all: $(LIBRARIES_O) gpio-i2c-sniffer

clean:
	-$(RM) $(LIBRARIES_O) gpio-i2c-sniffer.o gpio-i2c-sniffer

gpio-i2c-sniffer: gpio-i2c.cpp
	$(CC) $(CCFLAGS) -D_WITH_MAIN_ gpio-i2c.cpp -c -o gpio-i2c-sniffer.o
	$(CC) $(LDFLAGS) gpio-i2c-sniffer.o -o $@
