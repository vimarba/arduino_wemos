#define BLYNK_PRINT     Serial
#define BLYNK_GREEN     "#23C48E"
#define BLYNK_BLUE      "#04C0F8"
#define BLYNK_YELLOW    "#ED9D00"
#define BLYNK_RED       "#D3435C"
#define BLYNK_DARK_BLUE "#5F7CD8"

#define RESET_HOUR         0
#define RESET_MINUTE       0
#define RESET_SECOND       10

#define vEstadoDetector     V0
#define vTiempoDetector     V1
#define vLedDetector        V2
#define vpinDetector        D2

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <SNTPtime.h>

SNTPtime NTPch("ch.pool.ntp.org");
String myData,shour,sminute,ssecond,myDataReset;

WidgetLED LedDetector(vLedDetector);
BlynkTimer timer;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "sxFQued1mVcWrAVrXY44Rard9cnTOw-t";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "DRDALASL";
char pass[] = "doradapaellamanitas";

const int pinDetector = vpinDetector;
int estadopinDetector;
long tiempoDetector;
boolean tiempoDetectorreset;

void BucleDetector() {

  int estadopinDetector_leido;

  Serial.println("");

  estadopinDetector_leido = 255*digitalRead(pinDetector);

  Serial.print("Estado pin Detector leido: ");
  Serial.println(estadopinDetector_leido);
  
  if (estadopinDetector_leido == 255){
    tiempoDetector++;
  }
  Serial.print("Tiempo acumulado encendido: ");
  Serial.println(tiempoDetector);

  if (estadopinDetector_leido == 255 && estadopinDetector == 0){
      LedDetector.on();
  } else {
  if (estadopinDetector_leido == 0 && estadopinDetector == 255){
      LedDetector.off();
  }
  }

  estadopinDetector = estadopinDetector_leido;

  resettiempoDetector();
  Serial.print("Hora: ");
  Serial.println(myData);
  Serial.print("Hora Reset: ");
  Serial.println(myDataReset);
  if (tiempoDetectorreset) {
      tiempoDetector = 0;
  }
  
  bool isconnected = Blynk.connected();
  if (isconnected == true){
    Blynk.virtualWrite(vEstadoDetector, estadopinDetector_leido);
    Blynk.virtualWrite(vTiempoDetector, tiempoDetector);
  }

}

void resettiempoDetector(){
  
 strDateTime dateTimeReset;
 strDateTime dateTime = NTPch.getTime(1.0, 1);
  dateTimeReset.hour=RESET_HOUR;
  dateTimeReset.minute=RESET_MINUTE;
  dateTimeReset.second=RESET_SECOND;


  if (dateTime.hour<10) {
       shour= "0" + String(dateTime.hour);}
    else
       shour= String(dateTime.hour);
      
  if (dateTime.minute<10) {
      sminute= "0" + String(dateTime.minute);}
    else
      sminute= String(dateTime.minute);
     
  if (dateTime.second<10) {
      ssecond= "0" + String(dateTime.second);}
    else
      ssecond= String(dateTime.second);
     
  myData=String(dateTime.day)+"/" + String(dateTime.month) + "/" + String(dateTime.year) + " " + shour + ":" + sminute + ":" + ssecond;


  myDataReset=String(dateTimeReset.day)+"/" + String(dateTimeReset.month) + "/" + String(dateTimeReset.year) + " " + String(dateTimeReset.hour) + ":" + String(dateTimeReset.minute) + ":" + String(dateTimeReset.second);

  if ((dateTime.hour==dateTimeReset.hour)&&(dateTime.minute==dateTimeReset.minute)&&
  (dateTime.second<dateTimeReset.second))
  { tiempoDetectorreset = true;}
  else 
  { tiempoDetectorreset = false;} 
   
}


//LEER DATO TiempoDetector para el incio
BLYNK_WRITE(vTiempoDetector)
{
  tiempoDetector = param.asInt(); 
  Serial.print("Tiempo acumulado encendido: ");
  Serial.println(tiempoDetector);

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  pinMode(pinDetector, INPUT);
  
  Blynk.begin(auth, ssid, pass);
  Serial.println("Booting");

  LedDetector.setColor(BLYNK_RED);

  Blynk.syncVirtual(vTiempoDetector);

  //------FUNCIONES PARA OBTENER HORA-------////

  while (!NTPch.setSNTPtime()) Serial.print("."); // set internal clock
  Serial.println();
  Serial.println("Time set"); 
 
  timer.setInterval(1000L, BucleDetector);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}
