#include <ESP8266WiFi.h> 		  //Wlan-Client
#include <PubSubClient.h>		  //MQTT
#include <OneWire.h>			    //Für Dallas
#include <DallasTemperature.h>//Temperatur
#include "Y:/Accounts.h"			//Accounts

#define pinDATA SDA
#define TEMP_MAJOR 23.5
#define MESSPUNKT 255

int rpm, dutyCycle, DutyCounter;
int InterruptCounter = 0;
int RPM_Trig = 0;
int Counter = 0;
unsigned long currentTime1;
unsigned long diffTime;
const int RPM_Pin = 14; 		//GPIO für den RPM (D3)
const int FAN1_Pin = 2;     //GPIO für Fan 1 (SD3)
//const int FAN2_Pin = 9 ;   //GPIO für Fan 2 (SD2)
const int oneWirePin = 4;  	//GPIO für den DS18B20 (D2)
const char* MQTT_Clint_Name = "ESP-Server";

WiFiClient espClient;           		//WLan
PubSubClient client(espClient); 		//MQTT
OneWire oneWire(oneWirePin);			  //Aktivieren der OneWire Instanz
DallasTemperature sensors(&oneWire);//OneWire Referenz an den DallasSensor

//________________________________________________________
//Aufbau der WLan Verbindung
void setup_wifi() {
	delay(10);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(SSID);

  //Anmelden ins WLan
	WiFi.begin(SSID, PSK);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

//________________________________________________________
//Beim Verbindungsabruch der Reconnect
void reconnect() {
	while (!client.connected()) {
		Serial.print("Reconnecting...");
		if (!client.connect(MQTT_Clint_Name)) {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" retrying in 5 seconds");
			delay(5000);
		}
	}
}

//________________________________________________________
//Interrupt Funktion für die RPM Messung
void ICACHE_RAM_ATTR isr() {
  //Erste Zeitmessung
  if (InterruptCounter == 0){
    currentTime1 = millis();
  }
  //Nach 3 Umdrehungen
  if (InterruptCounter == 6){
    diffTime =  millis() - currentTime1;//Differenzzeit Messen
    analogWrite(FAN1_Pin, dutyCycle);   //PWM Ausgang wieder aktivieren
    InterruptCounter = 0;
    //currentTime1 = 0; 
    RPM_Trig = 2; //Messung abgeschlossen
    detachInterrupt(digitalPinToInterrupt(RPM_Pin)); //Interrupt deaktivieren
  }else{
    InterruptCounter++;
  }      
}

//________________________________________________________
void setup() {
	Serial.begin(115200); //Baudrate
	pinMode (RPM_Pin, INPUT);   //RPM Pin
  pinMode (FAN1_Pin, OUTPUT); //Fan1 Pin
  //pinMode (FAN2_Pin, OUTPUT);
	setup_wifi();	//Wlan verbinden
	client.setServer(MQTT_BROKER, 1883); //MQTT
  sensors.begin(); //DS18B20 Sensor
}

//________________________________________________________
void loop() {

	//Verbinden zum MQTT Server
	if (!client.connected()) {
	  reconnect();
	}
	client.loop();

	//Temperatur Messung
	sensors.requestTemperatures(); 
	float temperature = sensors.getTempCByIndex(0);
	client.publish("/server/temp",String(temperature).c_str(),true); //MQTT

	//RPM Signal vom FAN
  if (dutyCycle >= 30){

    //Messung wird gestartet
    if (Counter == 2 && RPM_Trig == 0){
      Counter = 0;
      RPM_Trig = 1; //Messmodus aktivieren
      analogWrite(FAN1_Pin, MESSPUNKT); //PWM Signal zum Messen abschalten.
      attachInterrupt(digitalPinToInterrupt(RPM_Pin), isr, RISING); //Interrupt aktivieren
    }
    if (RPM_Trig == 0){
      Counter++;
    }   
  }else{ //Deaktiviert eine laufende Messung, wenn der Fan zu langsam ist.
    detachInterrupt(digitalPinToInterrupt(RPM_Pin));
    analogWrite(FAN1_Pin, dutyCycle);
    InterruptCounter = 0;
    currentTime1 = 0;
    RPM_Trig = 0;
    rpm = 0;
    client.publish("/server/rpm",String(rpm).c_str(),true); //MQTT
  }
  //Messung ist abgeschlossen, RPM wird berechnet.
  if (RPM_Trig == 2) {
    if (diffTime > 30) { //Fehlerhafte Messung ausschließen
      rpm = (1000.0 / (diffTime / 3.0)) * 60.0; //RPM Berechnung
    }    
    client.publish("/server/rpm",String(rpm).c_str(),true); //MQTT
    RPM_Trig = 0; //Messung abstellen
  }

  //Temperaturregelung
  if (temperature >= TEMP_MAJOR) 
  {
    DutyCounter++;
  }else{
    DutyCounter--;
  }

  //PWM Lüftersteuerung
  if (DutyCounter >= 3){
    dutyCycle = dutyCycle + 10;
    DutyCounter = 0;
  }
  if (DutyCounter <= -3){
    dutyCycle = dutyCycle - 10;
    DutyCounter = 0;
  }
  if (dutyCycle > 255){
    dutyCycle = 255;
  }
  if (dutyCycle < 0){
    dutyCycle = 0;
  }

  //Wenn keine RPM Messung aktiv ist, wird PWM ausgegeben
  if (RPM_Trig == 0){
    analogWrite(FAN1_Pin, dutyCycle);
  }
  client.publish("/server/pwm",String(dutyCycle).c_str(),true); //MQTT

	delay(100);
}







