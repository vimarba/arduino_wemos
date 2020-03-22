/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP8266 chip.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right ESP8266 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/
/***********
OK      PULSADOR INTERRUPCION
OK    LED VERDE APAGADO EN CONEXION/ENCENDIDO NO CONEXION
      SINCRONISMO PULSADOR Y APP
      5 SCHEDULE 
**********/


/* Comment this out to disable prints and save space */
#define vAutoAppRelay       V0
#define vLedWifi            V1
#define vLedRelay           V2
#define vManualAppRelay     V3


#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "1baaf7976fee42548a4226a8ec11e7a0";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "DRDALASL";
char pass[] = "doradapaellamanitas";

const byte pinRelay = 12;
const byte pinLed = 13;
const byte pinPulsador = 0;
int iAutoRelay,iManualRelay;
bool bStatusRelay = LOW;

char startTime1;
char startDay1;
char stopTime1;
char stopDay1;
bool pinValue;

BlynkTimer timer;
void setup()
{
  Serial.begin(9600);
  
  pinMode(pinPulsador, INPUT);
  pinMode(pinLed, OUTPUT);
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinLed, HIGH); //ENCIENDO LED VERDE AL ARRANCAR
  
  Blynk.begin(auth, ssid, pass);
  Serial.println("Booting");

  attachInterrupt(digitalPinToInterrupt(0), CheckPulsador, HIGH);
  //timer.setInterval(1000L, main);
 // timer.setInterval(5*1000L, checkBlynkConnection);
// timer.setInterval(1000L, CheckPulsador);
}

//VERIFICAR CONEXION SERVIDOR ENCENDER LED VERDE SI NO LA HAY
void checkBlynkConnection(){  // called every 3 seconds by SimpleTimer
  bool isconnected = Blynk.connected();
  if (isconnected == false){
   digitalWrite(pinLed, HIGH);}  //ENCIENDO LED VERDE AL DESCONECTAR   
  else
  {digitalWrite(pinLed, LOW);}   //APAGO LED VERDE AL DESCONECTAR   
}

//PULSADOR MANUAL SONOFF
void CheckPulsador(){
//{   digitalWrite(pinRelay, LOW);
//    pinValue = !digitalRead(pinPulsador);
    
    if (bStatusRelay==HIGH){
        bStatusRelay=LOW;
        digitalWrite(pinRelay, LOW);
        Blynk.virtualWrite(vManualAppRelay,0);}
     else
     {bStatusRelay=HIGH;
      digitalWrite(pinRelay, HIGH);
      Blynk.virtualWrite(vManualAppRelay,1);
      }   
      

     
      }


//GET DATA APP MANUAL PULSADOR APP
BLYNK_WRITE(vManualAppRelay)
{iManualRelay = param.asInt();
 if (iManualRelay==1){
     bStatusRelay=HIGH;
     digitalWrite(pinRelay, HIGH);
     Blynk.virtualWrite(vManualAppRelay,1);
  }
  if (iManualRelay==0){
     bStatusRelay=LOW;
     digitalWrite(pinRelay, LOW);
     Blynk.virtualWrite(vManualAppRelay,0);
  }
}

//
/*void main()
{ 
schedule(V1);
schedule(V2);
schedule(V4);
schedule(V5);
}
*/


void loop()
{ 
  Blynk.run();
  timer.run();
}







BLYNK_WRITE(V0) {
  TimeInputParam t(param);

  // Process start time

  if (t.hasStartTime())
  {
    Serial.println(String("Start: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
  }
  else if (t.isStartSunrise())
  {
    Serial.println("Start at sunrise");
  }
  else if (t.isStartSunset())
  {
    Serial.println("Start at sunset");
  }
  else
  {
    // Do nothing
  }

  // Process stop time

  if (t.hasStopTime())
  {
    Serial.println(String("Stop: ") +
                   t.getStopHour() + ":" +
                   t.getStopMinute() + ":" +
                   t.getStopSecond());
  }
  else if (t.isStopSunrise())
  {
    Serial.println("Stop at sunrise");
  }
  else if (t.isStopSunset())
  {
    Serial.println("Stop at sunset");
  }
  else
  {
    // Do nothing: no stop time was set
  }

  // Process timezone
  // Timezone is already added to start/stop time

  Serial.println(String("Time zone: ") + t.getTZ());

  // Get timezone offset (in seconds)
  Serial.println(String("Time zone offset: ") + t.getTZ_Offset());

  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)

  for (int i = 1; i <= 7; i++) {
    if (t.isWeekdaySelected(i)) {
      Serial.println(String("Day ") + i + " is selected");
    }
  }

  Serial.println();
}

