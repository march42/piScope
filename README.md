# piScope
**WIP**: Raspberry Pi controlled telescope with (orientation) 9DOF sensor and ST-4 guider port

## Building, Installation, Running
**Work In Progress** vermutlich noch zu nichts zu gebrauchen und unter Umständen auch nicht zum Laufen
zu kriegen. *;)

## Motivation
Das Problem entstand, als ich mir eine neue Montierung fürs Telekop kaufte und feststellen musste, dass
die im Handbuch erwähnte RS232 Schnittstelle ohne Funktion ist. Der Laptop ist also weder in der Lage
das Teleskop zu bewegen, noch die aktuelle Ausrichtung des Tubus auszulesen. Die rudimentäre Steuerung
über den ST-4 port ist kein Problem, hier müssen lediglich 4 Transistoren oder Relais die entsprechenden
Steuereingänge gegen Masse schalten. Blieb nur das Problem: "Woher soll ich wissen, wo der Tubus hinschaut".
Die Idee war einen 9DOF orientation sensor zur exakten Positionsbestimmung am Teleskoptubus anzubringen
und das Teleskop dann per ST-4 Port zu Steuern.

Der Raspberry Pi als Plattform bot sich an, da ich davon den Einen oder Anderen im Arsenal habe. Auf einem
meiner PIs steckt der PiHat (Astro Pi) der einen LSM9DS1 9DOF Sensor an Bord hat. Für den Anfang dürfte das
also tun, für die Endversion würde ich dann eventuell eher den Bosch BNO055 wählen.

## Credits
* Stellarium findet man unter http://stellarium.org/ den Source unter https://launchpad.net/stellarium
* GPIO handling erledigt die PIGPIO library http://abyz.co.uk/rpi/pigpio/
* Infos zum BNO055 von Bosch gibt es dort https://www.bosch-sensortec.com/bst/products/all_products/bno055
* Von Bosch gibt es eine library https://github.com/BoschSensortec/BNO055_driver
* Unter https://github.com/ChrisDick/StarPi/ gibt es ein vergleichbares Projekt
* i2c-dev.h from i2c-tools required https://github.com/groeck/i2c-tools/tree/master/include/linux
or install package libi2c-dev
* AHRS algorithm by Sebastian Madgwick http://x-io.co.uk/open-source-imu-and-ahrs-algorithms/
* richards-tech RTIMULib found at https://github.com/RPi-Distro/RTIMULib
* RTIMULib2 found at https://github.com/RTIMULib/RTIMULib2
* SOFA library created by IAU http://www.iausofa.org/tandc.html
* Embedded Tool Kit ETK from https://github.com/supercamel/EmbeddedToolKit

## Status und ToDo
- Der 9DOF liefert immer etwas sinnfreie Werte, aber schon grob mit richtiger Richtung.
- ~~Eine Einnordung und Kalibrierung des 9DOF habe ich noch nicht geschafft.~~
- ~~Nachdem ich mich mit der Arithmethik und Sensorfusion beschäftigte graut es mir dolle~~
- Check out the Explore-Scientific PMC-Eight controller and mount
- with RTIMULib the IMU sensor values are getting close, to usable
- automated installing/building of libi2c-dev, pigpio, RTIMULib
- after using RTIMULib for the IMU sensor the data is getting closer to useable
- need to check out SOFA library, to ease work
