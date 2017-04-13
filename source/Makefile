# Makefile

LIBRARIES_CPP = I2Csensor.cpp IMU.cpp
LIBRARIES_O = $(LIBRARIES_CPP:.cpp=.o)
LIBRARIES_OBJ = $(LIBRARIES_CPP:.cpp=.obj)

test-i2c_CPP = test-i2c.cpp
test-i2c_O = $(test-i2c_CPP:.cpp=.o)
test-i2c_OBJ = $(test-i2c_CPP:.cpp=.obj)

CCFLAGS = -O4 -Wall -Wextra -Wno-unused-parameter -Werror -pthread -DDEBUG -DUSE_OWN_I2CDEV
LDFLAGS = -O4 -s -lstdc++ -pthread -lm

ifeq ($(OS),Windows_NT)
	RM = cmd //C del //Q //F
	RRM = cmd //C rmdir //Q //S
else
	RM = rm -f
	RRM = rm -f -r
endif

.SUFFIXES: .cpp .o .obj

.cpp.o:
	$(CC) $(CCFLAGS) $(CCEXTRA) $< -c -o $@

.cpp.obj:
	$(CROSS_GPP) $(CCFLAGS) $(CCEXTRA) $< -c -o $@

all: $(LIBRARIES_O) test

test: test-i2c

clean:
	-$(RM) $(LIBRARIES_O) $(test-i2c_O) test-i2c


test-i2c: $(test-i2c_O) $(LIBRARIES_O)
	$(CC) $(LDFLAGS) $(LDEXTRA) $(test-i2c_O) $(LIBRARIES_O) -o $@
