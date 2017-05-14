# Makefile

LIBRARIES_CPP += Vector3D.cpp Location.cpp TimeStamp.cpp AstroTime.cpp AstroVector.cpp
LIBRARIES_CPP += LogFile.cpp Telescope.cpp
LIBRARIES_CPP += I2Csensor.cpp IMU.cpp
LIBRARIES_O = $(LIBRARIES_CPP:.cpp=.o)

TESTPROGRAMS = test test_i2csensor test_vector test_rtimulib

CCFLAGS = -O3 -Wall -Wextra -Wno-unused-parameter -Werror -pthread -DDEBUG
LDFLAGS = -O3 -s -lstdc++ -pthread -lm

CONFIG_H = $(shell awk '/^[^\/].*define.*true/ { print $$3 }' ../config.h)
ifneq (,$(findstring USE_RTIMULIB,$(CONFIG_H)))
LDFLAGS += -lRTIMULib
endif
ifneq (,$(findstring USE_MADGWICK_AHRS,$(CONFIG_H)))
LIBRARIES_O += MadgwickAHRS.o
endif

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

.o .obj:
	$(CC) $(LDFLAGS) $(LDEXTRA) $< $(LIBRARIES_O) -o $@

all: $(LIBRARIES_O) testprogs

testprogs: $(TESTPROGRAMS)

clean:
	-$(RM) $(LIBRARIES_O)
	-$(RM) test.o test_i2csensor.o test_vector.o test_rtimulib.o
	-$(RM) test test_i2csensor test_vector test_rtimulib

dist-clean: clean
	-$(RM) *.o *.ini *.log

MadgwickAHRS.o: ../MadgwickAHRS/MadgwickAHRS.c
	$(CC)  $(CCFLAGS) $(CCEXTRA) $< -c -o $@

$(TESTPROGRAMS): test-programs.cpp $(LIBRARIES_O)
	$(CC) $(CCFLAGS) $(CCEXTRA) -D __$(shell echo $@ | tr '[:lower:]' '[:upper:]')__ $< -c -o $@.o
	$(CC) $(LDFLAGS) $(LDEXTRA) $@.o $(LIBRARIES_O) -o $@
