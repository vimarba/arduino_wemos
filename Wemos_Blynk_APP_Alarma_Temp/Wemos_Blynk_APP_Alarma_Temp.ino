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

  Simple push notification example

  App project setup:
    Push widget

  Connect a button to pin 2 and GND...
  Pressing this button will also push a message! ;)
 *************************************************************/

/* Comment this out to disable prints and save space */

/*DEFINICIONES*/
#define BLYNK_PRINT Serial
#define BLYNK_GREEN     "#23C48E"
#define BLYNK_BLUE      "#04C0F8"
#define BLYNK_YELLOW    "#ED9D00"
#define BLYNK_RED       "#D3435C"
#define BLYNK_DARK_BLUE "#5F7CD8"
#define DHTPIN 2  
#define DHTTYPE DHT22 

/*LIBRERIAS*/
#include <DHT.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

/*TIPOS*/
DHT dht(DHTPIN, DHTTYPE);
RCSwitch mySwitch = RCSwitch();
WidgetLED LedAlarma(V1); //ASIGNADO AL VIRTUAL PIN V1
BlynkTimer timer1,timer2;

/*BLYNK APP VARIABLES*/
char auth[] = "adfbd99f68f345df8be28f73854888f0";
char ssid[] = "DRDALASL";
char pass[] = "doradapaellamanitas";

/*VARIABLES PROYECTO*/

unsigned long AntTime500;
int ReciveValue,cnt=0,cnt2=0,bstatus,bEnableAlarma,bEnableAlarmaAnterior,bAlarmaOn,bstatusAnt,iOffsetTemp=0,iOffsetHumedad=0;
const int RF_ALARM=14253502;
char MSG_NOTIFY[]="ATENCION PUERTA ABIERTA"; 

void setup()
{
/*  pinMode(D2,OUTPUT);//14
  pinMode(D6,OUTPUT);//12
  pinMode(D7,INPUT);//13
  pinMode(D8,INPUT);//15
  digitalWrite(D2, HIGH);
  digitalWrite(D6, LOW);*/
  pinMode(D2,OUTPUT);
  digitalWrite(D2, HIGH);
  mySwitch.enableReceive(0); //GPIO12 D6 SEÑAL RECEPCION DATOS RF
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  LedAlarma.on();
 // Setup a function to be called every second
  timer2.setInterval(15L*60000L, ReadSendTemperature);  //ENVIO TEMPERATURA APP CADA 1min//(1L*60000L, ReadSendTemperature)
  timer1.setInterval(300L, blinkLedWidget);          //ENVIO STADO LED APP CADA 500ms
}


void loop()
{

     if (( millis()-AntTime500>500))
          {ReadRadio();
           AntTime500=millis();}
    
      Blynk.run();
      timer1.run();
      timer2.run();
  
      
}


//LEER DATO HABILITACION ALARMA
BLYNK_WRITE(V0)
{
  bEnableAlarma = param.asInt(); 
}
//LEER DATO HABILITACION ALARMA
BLYNK_WRITE(V4)
{
  iOffsetTemp = param.asInt(); 
}
BLYNK_WRITE(V5)
{
  iOffsetHumedad = param.asInt(); 
}



//LEER VALOR RADIOFRECUENCIA
void ReadRadio()

  {
      ReciveValue = mySwitch.getReceivedValue(); 
      if (mySwitch.available()) {
         ReciveValue=( mySwitch.getReceivedValue() );
            mySwitch.resetAvailable();
          }
      else {
          ReciveValue=0;
          }
      CheckValue();
      ResetStatusAlarma();
  }



//CHECKEO SI SE ABRE O CIERRA PUERTA
void CheckValue() 
{
      if (ReciveValue==RF_ALARM)  //VALOR DEL SENSOR RF
        {bstatus=1;
          }
      else   {bstatus=0;}

     
     if ((bstatus!=bstatusAnt)){             
       cnt=cnt+1;
           if ( cnt % 2 == 0)  
                {  
            cnt2=cnt2+1;
                      if ( cnt2 % 2 != 0) 
                        {
                            bAlarmaOn=1;    
                          Serial.println( "Puerta Abierta ");
                            if (bEnableAlarma==1)
                              {Blynk.notify(MSG_NOTIFY);}
                        } 
                      if ( cnt2 % 2 == 0) 
                          {
                          Serial.println( "Puerta Cerrada ");
                          bAlarmaOn=0;
                        } 
                 }
  
      bstatusAnt=bstatus;
  }
  }
//FUNCION RESETEA VALOR DE ALARMA AL PULSAR APP (PULSAR SIEMPRE CON PUERTA CERRADA)
void ResetStatusAlarma()
{
  if(bEnableAlarma!=bEnableAlarmaAnterior)
  { cnt=0;
    cnt2=0;
    bAlarmaOn=0;
    bEnableAlarmaAnterior=bEnableAlarma;
    }
  Serial.println(bEnableAlarma);
  }
//FUNCION READ TEMPERATURE
void ReadSendTemperature()
{
 int vcc = ESP.getVcc();
 float  h = dht.readHumidity()+iOffsetHumedad;
 float  t = dht.readTemperature()+iOffsetTemp; // or dht.readTemperature(true) for Fahrenheit

//  if (isnan(h) || isnan(t)) {
//    Serial.println("Failed to read from DHT sensor!");
//    return;
//  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V3, h);
  Serial.println(iOffsetTemp);
  Serial.println(t);
 // if (t<17){Blynk.notify("Temperatura casa menor que 17ºC");}
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V0,vcc);
 // Serial.print("Datos enviados ");

}


//FUNCION PLOT LED
void blinkLedWidget()
{
  if (bAlarmaOn==1) {
    LedAlarma.setColor(BLYNK_RED);
   // Serial.println("LED on V1: red");
  } else {
    LedAlarma.setColor(BLYNK_GREEN);
   // Serial.println("LED on V1: green");
  }
}
