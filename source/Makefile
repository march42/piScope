# Makefile

LIBRARIES_CPP += Vector3D.cpp Location.cpp TimeStamp.cpp AstroTime.cpp AstroVector.cpp
LIBRARIES_CPP += LogFile.cpp Telescope.cpp
LIBRARIES_CPP += I2Csensor.cpp IMU.cpp
LIBRARIES_O = $(LIBRARIES_CPP:.cpp=.o)

TESTPROGRAMS = test_i2csensor test_vector test_rtimulib

CCFLAGS = -O3 -Wall -Wextra -Wno-unused-parameter -Werror -pthread -DDEBUG
CCFLAGS += -DUSE_LINUX_I2CDEV
CCFLAGS += -DUSE_MADGWICK_AHRS
LDFLAGS = -O3 -s -lstdc++ -pthread -lm

CCFLAGS += -DUSE_RTIMULIB
LDFLAGS += -lRTIMULib

ifeq ($(OS),Windows_NT)
	RM = del /Q /F
	RRM = rmdir /Q /S
else
	RM = rm -f
	RRM = rm -f -r
endif

ifneq (,$(findstring USE_MADGWICK_AHRS,$(CCFLAGS)))
LIBRARIES_O += MadgwickAHRS.o
endif

.SUFFIXES: .cpp .o .obj

.cpp.o .cpp.obj:
	$(CC) $(CCFLAGS) $(CCEXTRA) $< -c -o $@

.o .obj:
	$(CC) $(LDFLAGS) $(LDEXTRA) $< $(LIBRARIES_O) -o $@

all: $(LIBRARIES_O) test

test: $(TESTPROGRAMS)

clean:
	-$(RM) $(LIBRARIES_O) $(test-i2csensor_O) $(test-vector_O)
	-$(RM) $(LIBRARIES_OBJ) $(test-i2csensor_OBJ) $(test-vector_OBJ)
	-$(RM) test-i2csensor test-vector

MadgwickAHRS.o: ../MadgwickAHRS/MadgwickAHRS.c
	$(CC)  $(CCFLAGS) $(CCEXTRA) $< -c -o $@

$(TESTPROGRAMS): test-programs.cpp $(LIBRARIES_O)
	$(CC) $(CCFLAGS) $(CCEXTRA) -D __$(shell echo $@ | tr '[:lower:]' '[:upper:]')__ $< -c -o $@.o
	$(CC) $(LDFLAGS) $(LDEXTRA) $@.o $(LIBRARIES_O) -o $@
