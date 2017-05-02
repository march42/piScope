## piScope/source


## Status und ToDo
- [ ] fill README.md
- [x] combine test routines and programs to a single source file
- [x] some code cleanup and remove Windows targets
- [x] option USE_LINUX_I2CDEV for I2Csensor.cpp - to use system include file
- [x] option USE_MADGWICK_AHRS for IMU.cpp - to use Madgwick sensor filtering and fusion
- [x] remove own copy of i2c-dev.h - it comes with libi2c-dev
- [ ] add RTIMULib to class Telescope (maybe better to Vector3D, but these lacks TimeStamp)
- [ ] add GPS (or other system specific) position detection to class Location
- [ ] add server code for remote access to class Telescope
- [ ] remove old, now unneeded, code
- [x] rename all classes and prefix with MH not to be mixed up with variable names
- [x] option USE_RTIMULIB for building with RTIMULib
- [ ] get orientation from IMU sensor
- [ ] add vector math to class MHVector3D
- [ ] add statistical locking to orientation (the more data the more precision
