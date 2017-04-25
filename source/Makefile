# Makefile

LIBRARIES_CPP += Vector3D.cpp Location.cpp TimeStamp.cpp AstroTime.cpp AstroVector.cpp
LIBRARIES_CPP += LogFile.cpp Telescope.cpp
LIBRARIES_CPP += I2Csensor.cpp IMU.cpp
LIBRARIES_O = $(LIBRARIES_CPP:.cpp=.o)
LIBRARIES_OBJ = $(LIBRARIES_CPP:.cpp=.obj)

test-vector_CPP = test-vector.cpp
test-vector_O = $(test-vector_CPP:.cpp=.o)
test-vector_OBJ = $(test-vector_CPP:.cpp=.obj)

test-i2csensor_CPP = test-i2csensor.cpp
test-i2csensor_O = $(test-i2csensor_CPP:.cpp=.o)
test-i2csensor_OBJ = $(test-i2csensor_CPP:.cpp=.obj)

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

test: test-i2csensor test-vector

clean:
	-$(RM) $(LIBRARIES_O) $(test-i2csensor_O) $(test-vector_O)
	-$(RM) $(LIBRARIES_OBJ) $(test-i2csensor_OBJ) $(test-vector_OBJ)
	-$(RM) test-i2csensor test-vector
	-$(RM) test-i2csensor.exe test-vector.exe


test-i2csensor: $(test-i2csensor_O) $(LIBRARIES_O)
	$(CC) $(LDFLAGS) $(LDEXTRA) $(test-i2csensor_O) $(LIBRARIES_O) -o $@

test-i2csensor.exe: $(test-i2csensor_OBJ) $(LIBRARIES_OBJ)
	$(CC) $(LDFLAGS) $(LDEXTRA) -static $(test-i2csensor_OBJ) $(LIBRARIES_OBJ) -o $@

test-vector: $(test-vector_O) $(LIBRARIES_O)
	$(CC) $(LDFLAGS) $(LDEXTRA) $(test-vector_O) $(LIBRARIES_O) -o $@

test-vector.exe: $(test-vector_OBJ) $(LIBRARIES_OBJ)
	$(CC) $(LDFLAGS) $(LDEXTRA) -static $(test-vector_OBJ) $(LIBRARIES_OBJ) -o $@
