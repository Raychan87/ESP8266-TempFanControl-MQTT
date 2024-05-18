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

### Benötigte Hardware
- NodeMCU v2 ESP8266
- Pololu TB6612FNG Dual Motor Driver Carrier
![0J12386 600](https://github.com/Raychan87/ESP8266-TempFanControl-MQTT/assets/18511462/9f4c5e5e-b9a9-4122-9c2d-c0a4f532d579)

- SparkFun AP63203 3,3V StepDown Regler
- 3,3V Logic Level Converter
- DS18B20
- 2x 3-Pol PC Lüfter

 
