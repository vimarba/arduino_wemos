
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include "settingsPZEM.h"
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <SNTPtime.h>
#include <SoftwareSerial.h>  
#include <ModbusMaster.h>

SNTPtime NTPch("ch.pool.ntp.org");


SoftwareSerial pzem(D5,D6);  // (RX,TX) connect to TX,RX of PZEM for NodeMCU
ModbusMaster node;
WiFiServer TelnetServer(23);
WiFiClient Telnet;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
BlynkTimer timer;
HTTPClient http;  //Declare an object of class HTTPClient


 

///-------VARIABLES------///
int httpCode;
double U_PR, I_PR,  P_PR,  PPR, PR_F, PR_PF, PR_alarm;
double PPRlastmonth=0,PPRmonth=0;
uint8_t result;  uint16_t data[6];
String myData,shour,sminute,ssecond,myDataReset;
boolean breset=false,bInicializacion=true;



// ================================================================
// ===          PARA CARGAR UN PROGRAMA VIA HTTP                ===
// ================================================================
//......1. COMPILAR GENERAR EL ARCHIVO .BIN
//......2. PONER EN EL NAVEGADOR -------> http://IP/update<--------
//......3. SELECCIONAR EL ARCHIVO Y CARGAR



// ================================================================
// ===               INICIALIZACION                             ===
// ================================================================
 
void setup(){
  
  Serial.begin(115200);
  Serial.println("Start serial"); 
  
//-----COMUNICACION SERIE CON PZEM-------//  

  pzem.begin(9600); 
  node.begin(1, pzem);  

//-------CONEXION DE WIFI Y BLYNK-------//

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Blynk.config(AUTH);  // in place of Blynk.begin(auth, ssid, pass);
  Blynk.connect(3333);  // timeout set to 10 seconds and then continue without Blynk
            
            while (Blynk.connect() == false){
              Blynk.disconnect();
              WiFi.mode(WIFI_OFF);}    
                    
//------FUNCIONES PARA OBTENER HORA-------////

        while (!NTPch.setSNTPtime()) Serial.print("."); // set internal clock
        Serial.println();
        Serial.println("Time set");  
        
//------FUNCIONES PARA HTTP UPDATER-------////

  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);

//------------HABILITAR TELNET------------////  
  
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);

//------------INICIALIZAR VARIABLES REMANENTES------//
  leervariablesremanentes();  
//------------TIMER BLYNK------------////        
  timer.setInterval(1000, principal);

}

// ================================================================
// ===                CARGAR VARIABLES REMANENTES               ===
// ================================================================

void leervariablesremanentes(){
 
         String  string_url = String("http://blynk-cloud.com/") + String(AUTH) 
                            +String("/get/")+String(vPIN_ENERGILASTAMONTH);
         http.begin(string_url);
         httpCode = http.GET();      
                   if (httpCode > 0) {
                            String payload = http.getString();
                            payload = payload.substring(2,payload.length()-2); //ELIMINO COMILLAS                           
                            PPRlastmonth=payload.toDouble();                                                       
                            }      
  
  }
 

void principal(){ 
  
  resetEnergy();
  getSerialdata();
  sendtoblynk();
  plotTelnet();
  
  }

// ================================================================
// ===   CHEQUEAR HORA PARA RESETEAR CONTADOR ENERGIA           ===
// ================================================================

void resetEnergy(){
  
 strDateTime dateTimeReset;
 strDateTime dateTime = NTPch.getTime(1.0, 1);
  dateTimeReset.hour=RESET_HOUR;
  dateTimeReset.minute=RESET_MINUTE;
  dateTimeReset.second=RESET_SECOND;
  dateTimeReset.day=RESET_DAY;


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
  (dateTime.second<dateTimeReset.second)&& (dateTime.day==dateTimeReset.day))
  { breset = true;}
  else 
  {breset=false;} 
   
}

// ================================================================
// ===               LEER VALORES DE PZEM004                    ===
// ================================================================

void getSerialdata () {
  
  result = node.readInputRegisters(0x0000, 10);
  if (result == node.ku8MBSuccess)  {
    U_PR      = (node.getResponseBuffer(0x00)/10.0f);
    I_PR      = (node.getResponseBuffer(0x01)/1000.000f);
    P_PR      = (node.getResponseBuffer(0x03)/10.0f);
    PPR       = (node.getResponseBuffer(0x05)/1000.0f);
        if (breset==true)
        { PPRlastmonth=PPR;
          Blynk.virtualWrite(vPIN_ENERGILASTAMONTH,PPRlastmonth);
        }
        PPRmonth=PPR-PPRlastmonth;
    PR_F      = (node.getResponseBuffer(0x07)/10.0f);
    PR_PF     = (node.getResponseBuffer(0x08)/100.0f);
    PR_alarm  = (node.getResponseBuffer(0x09));
      
 } 
}

// ================================================================
// ===               ENVIAR DATOS A BLYNK                       ===
// ================================================================

void sendtoblynk(){
  
    Blynk.virtualWrite(vPIN_VOLTAGE, U_PR);
    Blynk.virtualWrite(vPIN_CURRENT_USAGE, I_PR);
    Blynk.virtualWrite(vPIN_ACTIVE_POWER, P_PR);
    Blynk.virtualWrite(vPIN_ACTIVE_ENERGY, PPRmonth);
    Blynk.virtualWrite(vPIN_ENERGITOTAL, PPR);
    Blynk.virtualWrite(vPIN_ENERGILASTAMONTH, PPRlastmonth);
    Blynk.virtualWrite(vPIN_FREQUENCY, PR_F);
    Blynk.virtualWrite(vPIN_POWER_FACTOR, PR_PF);
    Blynk.virtualWrite(vPIN_OVER_POWER_ALARM, PR_alarm);
    
  }

// ================================================================
// ===               HACER PLOT DE VALORES POR TELNET           ===
//                 PONER IP DE DISPOSITIVO Y PUERTO 23
// ================================================================

void plotTelnet(){   
  
        Telnet.print("HORA ACTUAL:     ");Telnet.println(myData);
        Telnet.print("HORA RESET:     ");Telnet.println(myDataReset);
        Telnet.print("U_PR:     ");Telnet.println(U_PR); 
        Telnet.print("I_PR:     ");Telnet.println(I_PR,3);
        Telnet.print("P_PR:     ");Telnet.println(P_PR);   
        Telnet.print("PPR last month:     ");Telnet.println(PPRlastmonth);         
        Telnet.print("PPR actual month:     ");Telnet.println(PPRmonth,3);
        Telnet.print("PR_F:     ");Telnet.println(PR_F);   
        Telnet.print("PR_PF:     ");Telnet.println(PR_PF);
        
  }

// ================================================================
// ===                     HABILITAR TELNET                     ===
// ================================================================


void handleTelnet(){
  
  if (TelnetServer.hasClient()){
        if(!Telnet || !Telnet.connected()){
          Telnet=TelnetServer.available();}
         else {
          TelnetServer.available().stop();}
          
  }}

// ================================================================
// ===                      BUCLE PRINCIPAL                     ===
// ================================================================

void loop(){
  
  handleTelnet();
  Blynk.run();
  timer.run();
  httpServer.handleClient();
  MDNS.update();
  
}
