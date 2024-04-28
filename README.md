# ESP8266-TempFanControl-MQTT
 
### Beschreibung
Das Programm ist für ein ESP8288. Dieser steuert zwei 3 polige PC-Lüfter über ein DS18B20 Temperatur Sensor.
Zusätzlich werden die Temperatur und die Umdrehung eines Lüfters per MQTT in ein Smarthome System übertragen.
Und es kann über MQTT die Lüfter ausgeschaltet werden und die Temperaturgrenze eingestellt werden.


- WLAN Betrieb
- MQTT senden
    - RPM des Lüfters
    - Dutycycle des PWM der Lüfter
    - Temperatur
- MQTT empfangen
    - Temperaturgrenze
    - Ein- und Ausschalten der Lüfter
- PWM Regelnung zwei 3-polige PC-Lüfter
- Temperaturgesteuerte Regelung
- Auslesen der RPM eines Lüfters


 
