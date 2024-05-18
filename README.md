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

  ![grafik](https://github.com/Raychan87/ESP8266-TempFanControl-MQTT/assets/18511462/7b1a5c8b-3612-4b7d-93e3-93347f434481)

- Pololu TB6612FNG Dual Motor Driver Carrier
  
  ![0J12386 600](https://github.com/Raychan87/ESP8266-TempFanControl-MQTT/assets/18511462/e00ae69a-c851-47f4-9960-972b69f98844)
 
- SparkFun AP63203 3,3V StepDown Regler

  ![grafik](https://github.com/Raychan87/ESP8266-TempFanControl-MQTT/assets/18511462/4370400d-bd46-4e2f-a3be-0dc437de522f)
  
- 3,3V Logic Level Converter
  
  ![grafik](https://github.com/Raychan87/ESP8266-TempFanControl-MQTT/assets/18511462/3b016632-582c-4b65-b124-5af503afa766)

- DS18B20
  
  ![beelogger_DS18B20_Schaltung](https://github.com/Raychan87/ESP8266-TempFanControl-MQTT/assets/18511462/067c72ee-9093-459f-b875-c8e6a0d09c78)

- 2x 3-Pol PC Lüfter

 
