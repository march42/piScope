# Makefile

LIBRARIES_CPP = I2Csensor.cpp IMU.cpp
LIBRARIES_CPP += TimeStamp.cpp LogFile.cpp Location.cpp Vector4D.cpp Orientation.cpp Telescope.cpp
LIBRARIES_O = $(LIBRARIES_CPP:.cpp=.o)
LIBRARIES_OBJ = $(LIBRARIES_CPP:.cpp=.obj)

test-vector_CPP = test-vector.cpp
test-vector_O = $(test-vector_CPP:.cpp=.o)
test-vector_OBJ = $(test-vector_CPP:.cpp=.obj)

test-i2c_CPP = test-i2c.cpp
test-i2c_O = $(test-i2c_CPP:.cpp=.o)
test-i2c_OBJ = $(test-i2c_CPP:.cpp=.obj)

CCFLAGS = -O4 -Wall -Wextra -Wno-unused-parameter -Werror -pthread -DDEBUG
#CCFLAGS += -DUSE_LINUX_I2CDEV
LDFLAGS = -O4 -s -lstdc++ -pthread -lm

ifeq ($(OS),Windows_NT)
	RM = del /Q /F
	RRM = rmdir /Q /S
else
	RM = rm -f
	RRM = rm -f -r
endif

.SUFFIXES: .cpp .o .obj

.cpp.o .cpp.obj:
	$(CC) $(CCFLAGS) $(CCEXTRA) $< -c -o $@

all: $(LIBRARIES_O) test

test: test-i2c test-vector

clean:
	-$(RM) $(LIBRARIES_O) $(test-i2c_O) $(test-vector_O)
	-$(RM) $(LIBRARIES_OBJ) $(test-i2c_OBJ) $(test-vector_OBJ)
	-$(RM) test-i2c test-vector
	-$(RM) test-i2c.exe test-vector.exe


test-i2c: $(test-i2c_O) $(LIBRARIES_O)
	$(CC) $(LDFLAGS) $(LDEXTRA) $(test-i2c_O) $(LIBRARIES_O) -o $@

test-vector: $(test-vector_O) $(LIBRARIES_O)
	$(CC) $(LDFLAGS) $(LDEXTRA) $(test-vector_O) $(LIBRARIES_O) -o $@

test-vector.exe: $(test-vector_OBJ) $(LIBRARIES_OBJ)
	$(CC) $(LDFLAGS) $(LDEXTRA) -static $(test-vector_OBJ) $(LIBRARIES_OBJ) -o $@
