## piScope/source
piScope project https://github.com/march42/piScope

Source code of C++ classes to use for piScope Telescope project.

## License
(C) Copyright 2017 by Marc Hefter

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301 USA.

## Status und ToDo
- [ ] fill README.md
- [x] combine test routines and programs to a single source file
- [x] some code cleanup and remove Windows targets
- [x] option USE_LINUX_I2CDEV for I2Csensor.cpp - to use system include file
- [x] option USE_MADGWICK_AHRS for IMU.cpp - to use Madgwick sensor filtering and fusion
- [x] remove own copy of i2c-dev.h - it comes with libi2c-dev
- [x] add RTIMULib to class Telescope (maybe better to Vector3D, but these lacks TimeStamp)
- [ ] add GPS (or other system specific) position detection to class Location
- [ ] add server code for remote access to class Telescope
- [ ] remove old, now unneeded, code
- [x] rename all classes and prefix with MH not to be mixed up with variable names
- [x] option USE_RTIMULIB for building with RTIMULib
- [x] get orientation from IMU sensor
- [ ] add vector math to class MHVector3D
- [x] add statistical locking to orientation (the more data the more precision
- [ ] correct angles - most probably misinterpretation of radian and degree
- [ ] dont rely on RTIMULib-calculatedPose use own trigonometric

## general definitions
### angles in MHAstroVector type=LocalRPY
* preferably stored in RADIAN (-PI..0..PI)
* gravitational vector pointing from location to earth's center of mass
* X axis facing front, rotation clock-wise -90..roll..90, 0=horizontal, +90=left straight up, -90=left straight down
* Y axis facing right, rotation clock-wise -180..pitch..180, 0=horizontal, +90=nose straight up, -90=nose straight down
* Yaw/Azimuth - Z axis facing up, rotation clock-wise 0..yaw..360degree, 0=north, 90=east, 180=south, 270=west
### Euler angles
* Yaw, defined as clock wise rotation from X to X' on horizontal plane, viewing down from above, > turned to right
* Pitch, defined as angle between X/X' axis and horizontal plane, clock wise turn along Y axis, > rising X axis
* Roll, defined as angle between Y axis and horizontal plane, clock wise turn along X axis, > falling Y axis
### RTIMULib calculated pose
* X is roll
* Y is pitch
* Z is yaw
