


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
HTTPClient http;  //Declare an object of class 


#define NumValvulas  2
#define NumProgramas 4
#define BLYNK_PRINT Serial
#define WIFI_LED 13
#define BLYNK_RED       "#D3435C"

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "MtCoohNdIxmsHXsmTjGOl_zT-wbamLo4";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "DRDALASL";
char pass[] = "doradapaellamanitas";
  
const char* host = "Riego";


/////////////////////////////////////////////////////////////////////
// this code can control up to 4 switches                          //
// for each switch up to 4 schedule start times can be configured  //
// for each switch one duration is used (when schedule time is     //
//    reached the switch turns on for the duration)                //
/////////////////////////////////////////////////////////////////////
int start_time_sec[NumValvulas][NumProgramas];   // array of 4 start times (in seconds) for 4 switches [switch number][schedule timer number]
bool start_valid[NumValvulas][NumProgramas];      // is the start time valid ?
int stop_time_sec[NumValvulas][NumProgramas];    // array of 4 start times (in seconds) for 4 switches [switch number][schedule timer number]
bool stop_valid[NumValvulas][NumProgramas];      // is the start time valid ?
bool weekdays[NumValvulas][NumProgramas][8];     // array of 8 days (day 0 not used) for each schedule time
bool wifi_led_status = false;
int  relayAuto[NumValvulas][NumProgramas],relayBlynk[NumValvulas];// relayPuls[NumValvulas];
byte Salidas_rele[] = {12 , 5 , 4 , 15}; 
byte Pulsadores[] = {0,9,10,14};
int Output[4];

int AutoON=1;

int Porcentaje=100;
int diasOFF=0;

void ICACHE_RAM_ATTR ch_s0();
void ICACHE_RAM_ATTR ch_s1();

// timer object declaration
BlynkTimer timer,timeleds;

// this code use Real Time Clock widget in the blynk app to keep the clock updated from net
WidgetRTC rtc;
WidgetLED LedV0(V1);
WidgetLED LedV1(V2);

BLYNK_CONNECTED() {
  rtc.begin();
  Blynk.syncAll();
}

/////////////////////////////////////////////////////////////////
//              FUNCION CONVERSION TRATAMIENTO PROGRAMA        //
/////////////////////////////////////////////////////////////////
void set_time(BlynkParam param, byte valvula, byte programa){

     TimeInputParam t(param);
     
  // VERFICAR START
  
  if (t.hasStartTime())
  {start_time_sec[valvula][programa]=t.getStartHour()*60+ t.getStartMinute();   
    start_valid[valvula][programa] = true;}
  else
  { start_valid[valvula][programa] = false;}
  
  // VERFICAR STOP 
    
  if (t.hasStopTime())
  { stop_time_sec[valvula][programa]=t.getStopHour()*60+ t.getStopMinute();   
    stop_valid[valvula][programa] = true;}
  else
  {stop_valid[valvula][programa] = false;}

  // VERFICAR DIAS

  for (int dia = 1; dia <= 7; dia++) {
    if (t.isWeekdaySelected(dia)) 
    {weekdays[valvula][programa][dia] = true;}
    else 
    {weekdays[valvula][programa][dia] = false;}
    
  }}


/////////////////////////////////////////////////////////////////
//                  PROGRAMAS DE BLYNK                         //
/////////////////////////////////////////////////////////////////
BLYNK_WRITE(V3)  { set_time(param, 0,0);check();   }
BLYNK_WRITE(V4)  { set_time(param, 0,1);check();   }
BLYNK_WRITE(V5)  { set_time(param, 0,2);check();   }

BLYNK_WRITE(V6)  { set_time(param, 1,0);check();   }
BLYNK_WRITE(V7)  { set_time(param, 1,1);check();   }
BLYNK_WRITE(V8)  { set_time(param, 1,2);check();   }

/////////////////////////////////////////////////////////////////
//                  PULSADORES EVS Y MAN/AUTO DE BLYNK         //
/////////////////////////////////////////////////////////////////
BLYNK_WRITE(V12) { 
  AutoON=param.asInt();
  check();  
  }
BLYNK_WRITE(V10) { 
  relayBlynk[0]=param.asInt();
  check();   
  }
BLYNK_WRITE(V11) { 
  relayBlynk[1]=param.asInt();
  check();   
  }  


/////////////////////////////////////////////////////////////////
//                  Porcentaje Riego y Apagado dias riego      //
/////////////////////////////////////////////////////////////////
BLYNK_WRITE(V13) { 
  Porcentaje=param.asInt(); 
  check();  
  }
BLYNK_WRITE(V14) { 
  diasOFF=param.asInt();  
  check();  
  }

  
// schedual events handling
void check(){         // check if schedule #1 should run today

  if(year() != 1970){ //Verfico que la hora es correcta
    unsigned int nowminutes = ( hour()*60+minute()); //Genero el minuto actual
    int dayadjustment = -1;  
    if(weekday() == 1){
      dayadjustment = 6; //Genero el dia actual
    }

  //Si 0:00 rebajar en uno los diasOFF
  if (nowminutes==0){diasOFF--;}

  //Hago la comparar para ver si toca activar la salida
     
     for (int valvula = 0;  valvula< NumValvulas; valvula++) {
         for (int programa = 0;  programa< NumProgramas; programa++) {

//// aqui se puede incluir el procentaje de riego    <-----------------------

              if((start_valid[valvula][programa] == true)
              &(stop_valid[valvula][programa] == true)
              &(weekdays[valvula][programa][weekday() + dayadjustment]==true)
              &(nowminutes >= start_time_sec[valvula][programa])
              &(nowminutes < ( stop_time_sec[valvula][programa]-((stop_time_sec[valvula][programa]-start_time_sec[valvula][programa])*(100-Porcentaje)/100) ) ) )
                  { relayAuto[valvula][programa]=1;
                    }
              else
                  { relayAuto[valvula][programa]=0; }
              }}
  
//inicializo la suma a 0

      int relayAutoSum[4]={0,0,0,0};
      
//Hago suma de reles de programas para luego comparar para pasar de [X][X]->[X]

        for(int valvula=0;valvula<NumValvulas;valvula++) //VALVULA
          { for(int programa=0;programa<NumProgramas;programa++) //PROGRAMA
            {relayAutoSum[valvula]=relayAutoSum[valvula]+relayAuto[valvula][programa];} } 

//Pongo a 1 o 0 las salidas para el modo auto y blynk             

        for(int valvula=0;valvula<NumValvulas;valvula++) { 

                  if ((relayAutoSum[valvula]>0 & AutoON==1 & diasOFF==0 ) | relayBlynk[valvula]==1)
                   {  digitalWrite(Salidas_rele[valvula],HIGH);
                      Output[valvula]=1;
                   }
                   else
                   {   digitalWrite(Salidas_rele[valvula],LOW);
                       Output[valvula]=0;
                   }
        }
 }}



  
 
void setup()
{
        // Debug console
        Serial.begin(9600);
      
      
      //------INICIALIZAR ENTRADAS Y SALIDAS------////  
        for (int i = 0; i<NumValvulas ; i++){
          pinMode(Salidas_rele[i], OUTPUT);
          pinMode(Pulsadores[i], INPUT_PULLUP);
        }
        pinMode(WIFI_LED, OUTPUT);
        digitalWrite(WIFI_LED,HIGH);
      
      //------INICIALIZAR BLYNK------////  
        Blynk.begin(auth, ssid, pass);
        timer.setInterval(60000L, check);       // ejecuto el programa principal de AUTOMATICO cada minuto
        timeleds.setInterval(2000L, status_leds);     // verfico el estado de las salidad cada seg para refrescarlo en blynk
        setSyncInterval(10 * 60);                     // Sync interval in seconds (10 minutes)
        timeleds.setInterval(5000L, showRSSI);     // verfico el estado de las salidad cada seg para refrescarlo en blynk
      //------INICIALIZAR LEDS DE ESTADOS  DE BLYNK------////
        LedV0.setColor(BLYNK_RED);
        LedV0.off();
        LedV1.setColor(BLYNK_RED);
        LedV1.off();
        
      //------FUNCIONES PARA HTTP UPDATER-------////
      
        MDNS.begin(host);
        httpUpdater.setup(&httpServer);
        httpServer.begin();
        MDNS.addService("http", "tcp", 80);
        Serial.println(WiFi.localIP());
        Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
      
      //------INTERRUPCIONES PARA LOS PILSADORES FISICOS-------////
        attachInterrupt(digitalPinToInterrupt(Pulsadores[0]), ch_s0, FALLING);
        attachInterrupt(digitalPinToInterrupt(Pulsadores[1]), ch_s1, FALLING);

}


void status_leds ()
{       if (Output[0]==1) { LedV0.on();} else if (Output[0]==0){LedV0.off();}
        if (Output[1]==1) { LedV1.on();} else if (Output[1]==0){LedV1.off();} 
  }


void loop()
{ 
        Blynk.run();
        timer.run();
        timeleds.run();
    
        
      
      // LANZAR HTTP UPDATE //
        httpServer.handleClient();
        MDNS.update();
      // LED AZUL CUANDO HAY CONEXION  
        if (Blynk.connected()){digitalWrite(WIFI_LED,LOW);}
    
}


/////////////////////////////////////////////////////////////////
// INTERRUPCIONES QUE ENTRAN AL PULSAR FISICAMENTE EL BOTON    //
/////////////////////////////////////////////////////////////////

void ch_s0()
{
  if (relayBlynk[0]==1){
    Blynk.virtualWrite(V10,0);
    relayBlynk[0]=0;
    }
   else
   {
    Blynk.virtualWrite(V10,1);
    relayBlynk[0]=1;
   } 
   check();
  }
void ch_s1()
{
  if (relayBlynk[1]==1){
    Blynk.virtualWrite(V11,0);
    relayBlynk[1]=0;
   }
   else
   {
    Blynk.virtualWrite(V11,1);
    relayBlynk[1]=1;
   } 
   check();
  }

/////////////////////////////////////////////////////////////////
//                            PLOTS                            //
/////////////////////////////////////////////////////////////////

void PlotPrograma()
{
  for(int valvula=0;valvula<NumValvulas;valvula++) //VALVULA
    { for(int programa=0;programa<NumProgramas;programa++) //PROGRAMA
  {  
        Serial.print("VALVULA: ");
        Serial.print (valvula); 
        Serial.print("\t");
        Serial.print("Programa: ");
        Serial.print (programa); 
        Serial.print("\t");
        Serial.print("Start: ");
        Serial.print(start_time_sec[valvula][programa]); 
        Serial.print("\t");
        Serial.print("Start Valid: ");
        Serial.print(start_valid[valvula][programa]); 
        Serial.print("\t");
        Serial.print("Stop: ");
        Serial.print(stop_time_sec[valvula][programa]); 
        Serial.print("\t");
        Serial.print("Stop Valid: ");
        Serial.print(stop_valid[valvula][programa]); 
        Serial.println(" ");
        ///FALTA PLOT DIASSS
      }}
  
  
  
  }

void PlotSalidas_Modo(){

       Serial.print("Modo: ");
       if (AutoON) {Serial.println ("AUTO");}
       else {Serial.println ("MANUAL");}
       
for(int valvula=0;valvula<NumValvulas;valvula++) //VALVULA
    { 
        Serial.print("VALVULA: ");
        Serial.print (valvula); 
        Serial.print("Valor: ");
        Serial.println (Output[valvula]); 
    } }

void PlotrelaysAuto()
{
  for(int valvula=0;valvula<NumValvulas;valvula++) //VALVULA
    { for(int programa=0;programa<NumProgramas;programa++) //PROGRAMA
  {  
        Serial.print("VALVULA: ");
        Serial.print (valvula); 
        Serial.print("\t");
        Serial.print("Programa: ");
        Serial.print (programa); 
        Serial.print("\t");
        Serial.print("Valor: ");
        Serial.println(relayAuto[valvula][programa]); 
        }
        }
  
  }


void showRSSI(){
  String sRRSI=(map(WiFi.RSSI(), -110, -30, 30, 100) + String('%'));
//  
//  if(WiFi.RSSI() < -83){
//    Blynk.setProperty(V0, "color", BLYNK_RED);   // red gaug
//  }
//  else if(WiFi.RSSI() > -57){
//    Blynk.setProperty(V0, "color", BLYNK_GREEN);   // green gauge
//  }
//  else{
//    Blynk.setProperty(V0, "color", BLYNK_YELLOW);   // yellow gauge    
//  }
//  
//  delay(30);
//  Blynk.virtualWrite(V0, map(WiFi.RSSI(), -110, -30, 45, 90));
//  float cycle = millis() / (1000.0);
  Blynk.virtualWrite(V0, sRRSI);


  
}  
