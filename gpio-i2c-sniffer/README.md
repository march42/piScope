I2C sniffer

Bei meiner Bresser EXOS-2 ist mir aufgefallen, dass die Motoren über i2c angesteuert werden und darüber auch
die Bewegungen zurück melden. Mit etwas Aufwand dürfte es machbar sein, die Motorsteuerung und Positions-
bestimmung hierüber komplett nach zu bilden. Als erstes werde ich mir also mal einen I2C sniffer bauen, der
das Protokoll aufzeichnet und die Logs auf Regelmässigkeiten analysieren.

Credits:
	Das GPIO handling erledigt die PIGPIO library http://abyz.co.uk/rpi/pigpio/ damit ich hier nicht das Rad
	neu erfinden muss.
