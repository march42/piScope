# piScope
WIP: Raspberry Pi controlled telescope with (orientation) 9DOF sensor and ST-4 guider port

Das Problem entstand, als ich mir eine neue Montierung fürs Telekop kaufte und feststellen musste, dass
die im Handbuch erwähnte RS232 Schnittstelle ohne Funktion ist. Der Laptop ist also weder in der Lage
das Teleskop zu bewegen, noch die aktuelle Ausrichtung des Tubus auszulesen. Die rudimentäre Steuerung
über den ST-4 port ist kein Problem, hier müssen lediglich 4 Transistoren oder Relais die entsprechenden
Steuereingänge gegen Masse schalten. Blieb nur das Problem: "Woher soll ich wissen, wo der Tubus hinschaut".
Die Idee war einen 9DOF orientation sensor zur exakten Positionsbestimmung am Teleskoptubus anzubringen
und das Teleskop dann per ST-4 Port zu Steuern.

Der Raspberry Pi als Plattform bot sich an, da ich davon den Einen oder Anderen im Arsenal habe. Auf einem
meiner PIs steckt der PiHat (Astro Pi) der einen LSM9DS1 9DOF Sensor an Bord hat. Für den Anfang dürfte das
also tun, für die Endversion würde ich dann eventuell eher den Bosch BSO055 wählen.

Status und ToDo
Der 9DOF liefert immer etwas sinnfreie Werte, aber schon grob mit richtiger Richtung.
Eine Einnordung und Kalibrierung des 9DOF habe ich noch nicht geschafft.
