#include <ESP8266WiFi.h> 		  //Wlan-Client
#include <PubSubClient.h>		  //MQTT
#include <OneWire.h>			    //Für Dallas
#include <DallasTemperature.h>//Temperatur
#include "Y:/Accounts.h"			//Accounts

//---PWM---//
#define PWM_FREQ 8000 //100 ... 40000 (default: 1000)
#define PWM_RANGE 255  //15 ... 65535 (default: 255)
#define PWM_MIN_DUTY 90 //Mindest PWM Signal
#define PWM_MAX_DUTY 255 //PWM Off
#define PWM_STEP_DUTY 10 //Schrittweite Erhöhung des PWMs
#define PWM_CYCLUS_UP 3 //Loop Anzahl für Schrittweite Erhöhung
#define PWM_CYCLUS_DOWN -3 //Loop Anzahl für Schrittweite Verringerung

//---GPIO-PINs---//
#define RPM_PIN 14
#define FAN1_PIN 10
#define FAN2_PIN 9
#define TEMP_PIN 4  //DS18B20

//
#define LOOP_TIME 750 //ms
#define MAJOR_TEMP 22.0

//________________________________________________________
//RPM
int rpm;                    //U/min
int RPM_Mode = 0;           //0 = AUS, 1 = Messung, 2 = Finish
int RPM_DelayCounter = 0;   //Für die Messpausen
int RPM_Half_turn = 0;      //Anzahl der Halb-Umdrehung
unsigned long RPM_startTime = 0; //Startzeit des ersten Interrupts
unsigned long RPM_diffTime = 0; //Zeit zwischen den ersten und letzten Interrupts

//________________________________________________________
//Var
int dutyCycle, DutyCounter;
int MajorTemp = MAJOR_TEMP;
unsigned long previousMillis = 0;  
const char* MQTT_Clint_Name = "ESP-Server";

//________________________________________________________
//Init
WiFiClient espClient;           		//WLan
PubSubClient client(espClient); 		//MQTT
OneWire oneWire(TEMP_PIN);			  //Aktivieren der OneWire Instanz
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

void suscribe_mqtt()
{

}

//________________________________________________________
//Interrupt Funktion für die RPM Messung
void ICACHE_RAM_ATTR isr() {

  //Erster Interrupt - Erste Zeitmessung
  if (RPM_Half_turn == 0){
    RPM_startTime = millis();
    Serial.printf("Start:");
    Serial.println(RPM_startTime);
  }
  //Letzter Interrupt
  if (RPM_Half_turn == 2){
    RPM_diffTime =  millis() - RPM_startTime;//Differenzzeit Messen
    analogWrite(FAN1_PIN, dutyCycle);   //PWM Ausgang wieder aktivieren
    RPM_Half_turn = 0;
    RPM_Mode = 2; //Messung abgeschlossen
    detachInterrupt(digitalPinToInterrupt(RPM_PIN)); //Interrupt deaktivieren
  }else{
    RPM_Half_turn++; //Nächste halbe Umdrehung
  }      
}

//________________________________________________________
void setup() {
	Serial.begin(115200); //Baudrate
	pinMode (RPM_PIN, INPUT);   //RPM Pin
  pinMode (FAN1_PIN, OUTPUT); //Fan1 Pin
  //pinMode (FAN2_PIN, OUTPUT);
	setup_wifi();	//Wlan verbinden
	client.setServer(MQTT_BROKER, 1883); //MQTT Broker
  sensors.begin(); //DS18B20 Sensor
  analogWriteFreq(PWM_FREQ);   //PWM Frequenz (100...40000 default: 1000)
  analogWriteRange(PWM_RANGE); //PWM Range (15...65535)
}

//________________________________________________________
void loop() {

	//Verbinden zum MQTT Server
	if (!client.connected()) {
	  reconnect();
	}
	client.loop();

  unsigned long currentMillis = millis();

  //Loop Zeit
  if (currentMillis - previousMillis >= LOOP_TIME) {
    previousMillis = currentMillis;

    //Temperatur Messung
    sensors.requestTemperatures(); 
    float temperature = sensors.getTempCByIndex(0);
    if (temperature >= 1){ //Auslesefehler werden nicht gesendet
      client.publish("/server/temp",String(temperature).c_str(),true); //MQTT
    }
    
    //RPM Signal vom FAN
    if (dutyCycle >= PWM_MIN_DUTY){

      //Messung wird gestartet
      if (RPM_DelayCounter == 2 && RPM_Mode == 0){
        analogWrite(FAN1_PIN, PWM_MAX_DUTY); //PWM Signal zum Messen abschalten.
        delay(10);
        RPM_DelayCounter = 0;
        RPM_Mode = 1; //Messmodus aktivieren
        attachInterrupt(digitalPinToInterrupt(RPM_PIN), isr, RISING); //Interrupt aktivieren
      }
      if (RPM_Mode == 0){
        RPM_DelayCounter++;
      }   
    }else{ //Deaktiviert eine laufende Messung, wenn der Fan zu langsam ist.
      detachInterrupt(digitalPinToInterrupt(RPM_PIN));
      analogWrite(FAN1_PIN, dutyCycle);
      RPM_Half_turn = 0;
      RPM_startTime = 0;
      RPM_Mode = 0;
      rpm = 0;
      client.publish("/server/rpm",String(rpm).c_str(),true); //MQTT
    }
    //Messung ist abgeschlossen, RPM wird berechnet.
    if (RPM_Mode == 2) {
      if (RPM_diffTime > 10) { //Fehlerhafte Messung ausschließen        
        client.publish("/server/diff",String(RPM_diffTime).c_str(),true); //MQTT
        rpm = (1000.0 / (RPM_diffTime )) * 60.0; //RPM Berechnung
        RPM_startTime = 0;
        RPM_diffTime = 0;
      }    
      client.publish("/server/rpm",String(rpm).c_str(),true); //MQTT
      RPM_Mode = 0; //Messung abstellen
    }

    //Temperaturregelung
    if (temperature >= MajorTemp) 
    {
      DutyCounter++;
    }else{
      DutyCounter--;
    }

    //PWM Lüftersteuerung
    if (DutyCounter >= PWM_CYCLUS_UP){
      if (dutyCycle <= PWM_MIN_DUTY){
        dutyCycle = PWM_MIN_DUTY + 5;
      }else{
        dutyCycle = dutyCycle + PWM_STEP_DUTY;
        if (dutyCycle > PWM_MAX_DUTY){
        dutyCycle = PWM_MAX_DUTY;
        }
      }
      DutyCounter = 0;
    }
    if (DutyCounter <= PWM_CYCLUS_DOWN){
      if (dutyCycle <= PWM_MIN_DUTY){
        dutyCycle = 0;
      }else{
        dutyCycle = dutyCycle - PWM_STEP_DUTY;
      }    
      DutyCounter = 0;
    }
    
    //Wenn keine RPM Messung aktiv ist, wird PWM ausgegeben
    if (RPM_Mode == 0){
      analogWrite(FAN1_PIN, dutyCycle);
    }
    client.publish("/server/pwm",String(dutyCycle).c_str(),true); //MQTT
  }
}







