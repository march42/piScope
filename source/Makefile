# Makefile

LIBRARIES_CPP = I2Csensor.cpp IMU.cpp
LIBRARIES_O = $(LIBRARIES_CPP:.cpp=.o)
LIBRARIES_OBJ = $(LIBRARIES_CPP:.cpp=.obj)

test_i2c_CPP = test-i2c.cpp
test_i2c_O = $(test_i2c_CPP:.cpp=.o)
test_i2c_OBJ = $(test_i2c_CPP:.cpp=.obj)

CCFLAGS = -O4 -Wall -Wextra -Wno-unused-parameter -Werror -pthread -DDEBUG -DUSE_OWN_I2CDEV
LDFLAGS = -O4 -s -lstdc++ -pthread

.SUFFIXES: .cpp .o .obj

.cpp.o:
	$(CC) $(CCFLAGS) $< -c -o $@

.cpp.obj:
	$(CROSS_GPP) $(CCFLAGS) $< -c -o $@

all: $(LIBRARIES_O)

test: test-i2c

clean:
	rm *.o
	del *.obj


test-i2c: $(test_i2c_O) $(LIBRARIES_O)
	$(CC) $(LDFLAGS) $(test_i2c_O) $(LIBRARIES_O) -o $@
