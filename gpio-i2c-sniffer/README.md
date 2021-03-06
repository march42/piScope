# I2C sniffer

~~Bei meiner Montierung ist mir aufgefallen, dass die Motoren über i2c angesteuert werden und darüber auch
die Bewegungen zurück melden.~~ Mit etwas Aufwand dürfte es machbar sein, die Motorsteuerung und Positions-
bestimmung hierüber komplett nach zu bilden. Als erstes werde ich mir also mal einen I2C sniffer bauen, der
das Protokoll aufzeichnet und die Logs auf Regelmässigkeiten analysieren.

Auf den zweiten Blick hat es sich als RS232 herausgestellt, aber der Sniffer ist soweit fast einsatzbereit.
Vielleicht schiebe ich den in ein anderes Projekt und mache ihn noch fertig.
Nun muss ich nochmal ganz genau hinschauen, wie sich die Steuerung am besten realisieren lässt.

## Credits:
- Das GPIO handling erledigt die PIGPIO library http://abyz.co.uk/rpi/pigpio/ damit ich hier nicht das Rad
 neu erfinden muss.

## WIP/ToDo
- [x] basic class layout
- [x] simple wrappers for PIGPIO
- [x] GPIO reading
- [x] I2C bit and timing interpretation
- [ ] I2C data and frequency interpretation
- [x] pthread handling for work loop
- [x] preparation for multiple interface handling
- [x] multiple interfaces and work loops
- [x] logging functions
- [x] parameter handling for logging
- [x] data buffering

## I2C Grundlagen und Informationen
### I2C protocol electrical specifications
- SCL clock line (always generated by the master)
- SDA data line (pulled by sending party)
- both lines (SCL,SDA) held high with pull-up-resistors
- both lines (SCL,SDA) high (after t_SCL = 1/f_SCL) signal bus free
- missing or nonresponsive slave detected by NACK after sending slave address - SDA line stays high, because no slave to pull it down

### I2C protocol logical specifications
- START condition: HIGH-LOW transition on SDA, while SCL is HIGH
- STOP condition: LOW-HIGH transition on SDA, while SCL is HIGH
- repeated START: generating START instead of STOP

#### Byte/Bit
- Every byte put on the SDA line must be eight bits long.
- The number of bytes that can be transmitted per transfer is unrestricted.
- Each byte must be followed by an Acknowledge bit.
- Data is transferred with the Most Significant Bit (MSB) first.
- If a slave cannot receive or transmit another complete byte of data until it has performed some other function, for example
 servicing an internal interrupt, it can hold the clock line SCL LOW to force the master into a wait state.
 Data transfer then continues when the slave is ready for another byte of data and releases clock line SCL.

#### Acknowledge (ACK) and Not Acknowledge (NACK)
- The acknowledge takes place after every byte.
- The master generates all clock pulses, including the acknowledge ninth clock pulse.
- The receiver pulls the SDA line LOW and it remains stable LOW during the HIGH period of this clock pulse.
- When SDA remains HIGH during this ninth clock pulse, this is defined as the Not Acknowledge signal.
- The master can then generate either a STOP condition to abort the transfer, or a repeated START condition to start a new transfer.

#### The slave address and R/-W bit
- After the START condition (S), a slave address is sent.
- This address is seven bits long
- followed by an eighth bit which is a data direction bit (R/W)
  , a zero 0bXXXXXXX0 indicates a transmission (WRITE)
  , a one 0bXXXXXXX1 indicates a request for data (READ)
- A data transfer is always terminated by a STOP condition (P) generated by the master.

#### timings (100kHz,400kHz)
- f_SCL clock frequency 0-100kHz, 0-400kHz
- t_SCL bit length 10&micro;s, 2.5&micro;s
- t_HD_STA hold time START condition &gt;4&micro;s, &gt;0.6&micro;s
- t_LOW_SCL low period of SCL clock &gt;4.7&micro;s, &gt;1.3&micro;s
- t_HIGH_SCL high period of SCL clock &gt;4&micro;s, &gt;0.6&micro;s
- t_SU_SCL setup time for repeated START condition &gt;4.7&micro;s, &gt;0.6&micro;s
- t_HD_SDA hold time for DATA &gt;0
- t_SU_SDA setup time for DATA &gt;250ns, &gt;100ns
- t_SU_STO setup time for STOP condition &gt;4&micro;s, &gt;0.6&micro;s
- t_BUF bus free time between STOP and START condition &gt;4.7&micro;s, &gt;1.3&micro;s
- The maximum tHD;DAT could be 3.45 &micro;s and 0.9 &micro;s for Standard-mode and Fast-mode, but must be less than the maximum of t_VD_DAT or t_VD_ACK by a transition time.
- This maximum must only be met if the device does not stretch the LOW period (tLOW) of the SCL signal.
- If the clock stretches the SCL, the data must be valid by the set-up time before it releases the clock.
- t_VD_DAT data valid time &lt;3.45&micro;s, &lt;0.9&micro;s
- t_VD_ACK data valid time for ACK/NACK bit &lt;3.45&micro;s, &lt;0.9&micro;s
