#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include "settingsPZEM.h"
#include "Costes.h"
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <SNTPtime.h>
#include <SoftwareSerial.h>  
#include <ModbusMaster.h>
#include <ESP8266Ping.h>
SNTPtime NTPch("ch.pool.ntp.org");


SoftwareSerial pzem(D5,D6);  // (RX,TX) connect to TX,RX of PZEM for NodeMCU
ModbusMaster node;
WiFiServer TelnetServer(23);
WiFiClient Telnet;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
BlynkTimer timer1,timer2,timer3,timer4;
HTTPClient http;  //Declare an object of class HTTPClient

WidgetTable table;
BLYNK_ATTACH_WIDGET(table, vPIN_Tabla);

 

///-------VARIABLES------///
int httpCode;
double U_PR, I_PR,  P_PR,  PPR, PR_F, PR_PF, PR_alarm;
double PPRlastmonth,PPRmonth,PPR_ant,CosteMes;
uint8_t result;  
String myData,shour,sminute,ssecond,myDataReset;
boolean bInicializacion=true,bAviso=false,bEnableNotificacion=false;
int minutoactual,diaactual,rowIndex=0;
double UmbralAvisoPotencia=500;
unsigned long previousMillis,currentMillis,TiempoEntreNotificaciones=36000000;


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
  pinMode(4, OUTPUT);
  digitalWrite(4,HIGH);
  Serial.begin(115200);
  Serial.println("Start serial");
 

//-------CONEXION DE WIFI Y BLYNK-------//

  Blynk.begin(AUTH, ssid, pass);
  
//------FUNCIONES PARA HTTP UPDATER-------////

  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);

//------------HABILITAR TELNET------------////  
 
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);

//-----COMUNICACION SERIE CON PZEM-------//  

  pzem.begin(9600);
  node.begin(1, pzem);  


//-----SINCRONIZAR HORA-------// 
while (!NTPch.setSNTPtime()) Serial.print("."); // set internal clock
        Serial.println();
        Serial.println("Time set");
  

//------------INICIALIZAR VARIABLES REMANENTES------//
//  leervariablesremanentes();  
//------------TIMER BLYNK------------////        
  timer1.setInterval(1000, principal);
//  timer3.setInterval(30000, Costes);
  timer2.setInterval(60000, resetEnergy);
//  timer3.setInterval(10000, plotTelnet);
  timer4.setInterval(62000, notificacion);
}
//http://blynk-cloud.com/BUi2VYHuPd_4a68bUEUvTE-kDDYQPB4V/get/V11
//http://blynk-cloud.com/BUi2VYHuPd_4a68bUEUvTE-kDDYQPB4V/get/V3

// ================================================================
// ===                CARGAR VARIABLES REMANENTES               ===
// ================================================================



BLYNK_CONNECTED()
{
   if (bInicializacion) {    //CARGAR EL ULTIMO VALOR DEL SERVIDOR BLYNK EN
            Serial.println("Sincronizando....");
            Blynk.syncAll();
          bInicializacion = false;
  }}

BLYNK_WRITE(vPIN_EnergiaMesActual)
{ PPRmonth=param.asFloat();
  Serial.print("Valor mes:  ");
  Serial.println(PPRmonth);
 Blynk.virtualWrite(V25, PPRmonth);}

BLYNK_WRITE(vPIN_EnergiaIteracionAnterior)
{ PPR_ant=param.asFloat();
  Serial.print("Valor mes ant:  ");
  Serial.println(PPR_ant);
 Blynk.virtualWrite(V26, PPR_ant);}

BLYNK_WRITE(vPIN_TablaRow)
{rowIndex=param.asFloat();  
  }

void principal(){
  getSerialdata();
  sendtoblynk();
  }
// ================================================================
// ===          PULSADOR  RESETEAR CONTADOR ENERGIA             ===
// ================================================================

BLYNK_WRITE(vPIN_ResetManual) {
  if (param.asInt()==1){
    Blynk.virtualWrite(vPIN_EnergiaMesAnterior, PPRmonth);
    PPRmonth=0;}
}

// ================================================================
// ===        PULSADOR  NOTIFICACIONES FUERA DE CASA            ===
// ================================================================

BLYNK_WRITE(vPIN_EnableNotificacion) {
  if (param.asInt()==1){
    bEnableNotificacion=1;}
  else
    bEnableNotificacion=0;
}


// ================================================================
// ===   CHEQUEAR HORA PARA RESETEAR CONTADOR ENERGIA           ===
// ================================================================

void resetEnergy(){
 
 strDateTime dateTime = NTPch.getTime(1.0, 1);

  minutoactual=dateTime.hour*60+dateTime.minute;
  diaactual=dateTime.day;

String dia_minuto=String(diaactual)+String("    ")+String(minutoactual);

  Blynk.virtualWrite(vPIN_FECHA, dia_minuto);

  CosteMes=CalculoCoste(PPRmonth,diaactual);

  Blynk.virtualWrite(vPIN_COSTE,CosteMes); 
  Blynk.virtualWrite(vPIN_StrCOSTE,CosteMes+ String(" €")); 

  
  if ((minutoactual==1)&&(diaactual==1))
  {Blynk.virtualWrite(vPIN_EnergiaMesAnterior, PPRmonth);
   Blynk.virtualWrite(vPIN_StrEnergiaMesAnterior, String((int)PPRmonth) + " KWH");
   Blynk.virtualWrite(vPIN_CosteMesAnterior, CosteMes);
   Blynk.virtualWrite(vPIN_StrCosteMesAnterior, String((int)CosteMes) + " €");
   WriteTable(dateTime.month, dateTime.year, String((int)CosteMes), String((int)PPRmonth));
   PPRmonth=0;}
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
   
   
   
        if (((PPR-PPR_ant)>0)&(PPR_ant>0))      //&((PPRmonth>0)||(breset==true))){
           { PPRmonth=PPRmonth+(PPR-PPR_ant);    
            Blynk.virtualWrite(vPIN_EnergiaMesActual, PPRmonth);}
           
        else if   ((PPR-PPR_ant)<0)
            {PPRmonth=PPRmonth;    
            Blynk.virtualWrite(vPIN_EnergiaMesActual, PPRmonth);}
            
            PPR_ant=PPR;
            Blynk.virtualWrite(vPIN_EnergiaIteracionAnterior, PPR_ant);        
           
     Serial.print("Energia:  ");
     Serial.println(PPRmonth);
    
 //   CosteMes=CalculoCoste(PPRmonth,diaactual);
       
    PR_F      = (node.getResponseBuffer(0x07)/10.0f);
    PR_PF     = (node.getResponseBuffer(0x08)/100.0f);
    PR_alarm  = (node.getResponseBuffer(0x09));
     
 }
}

// ================================================================
// ===               CALCULAR COSTES DE MES                     ===
// ================================================================
//
//void Costes()
//{ 
//  strDateTime dateTime = NTPch.getTime(1.0, 1);
//  diaactual=dateTime.day;
//  CosteMes=CalculoCoste(PPRmonth,diaactual);
//  Blynk.virtualWrite(vPIN_COSTE, CosteMes + String(" €")); 
//  Blynk.virtualWrite(vPIN_CosteMesAnterior, CosteMes);
//  
//  if ((minutoactual==1)&&(diaactual==1)&&(auxReset==1))
//  {
//  Blynk.virtualWrite(vPIN_CosteMesAnterior, CosteMes);
//  auxReset=0;   
//  }    
// }






// ================================================================
// ===               ENVIAR DATOS A BLYNK                       ===
// ================================================================



void sendtoblynk(){
    String sRRSI=(map(WiFi.RSSI(), -110, -30, 30, 100) + String('%'));
    Blynk.virtualWrite(vPIN_VOLTAGE, U_PR);
    Blynk.virtualWrite(vPIN_CURRENT_USAGE, I_PR);
    Blynk.virtualWrite(vPIN_ACTIVE_POWER, P_PR);
   
    Blynk.virtualWrite(vPIN_ENERGITOTAL, PPR);
     
    Blynk.virtualWrite(vPIN_FREQUENCY, PR_F);
    Blynk.virtualWrite(vPIN_POWER_FACTOR, PR_PF);
    Blynk.virtualWrite(vPIN_OVER_POWER_ALARM, PR_alarm);
    Blynk.virtualWrite(vPIN_RRSI, sRRSI);

   
  }

// ================================================================
// ===               HACER PLOT DE VALORES POR TELNET           ===
//                 PONER IP DE DISPOSITIVO Y PUERTO 23
// ================================================================

void plotTelnet(){  

        Serial.print("HORA ACTUAL:     ");Serial.println(myData);
        Serial.print("HORA RESET:     ");Serial.println(myDataReset);
        Serial.print("U_PR:     ");Serial.println(U_PR);
        Serial.print("I_PR:     ");Serial.println(I_PR,3);
        Serial.print("P_PR:     ");Serial.println(P_PR);
        Serial.print("PPR:     ");Serial.println(PPR);    
        Serial.print("PPR anterior:     ");Serial.println(PPR_ant);        
        Serial.print("PPR actual month:     ");Serial.println(PPRmonth,3);
        Serial.print("PR_F:     ");Serial.println(PR_F);  
        Serial.print("PR_PF:     ");Serial.println(PR_PF);
        Serial.print("Umbral de aviso:     ");Serial.println(UmbralAvisoPotencia);
 
        Telnet.print("HORA ACTUAL:     ");Telnet.println(myData);
        Telnet.print("HORA RESET:     ");Telnet.println(myDataReset);
        Telnet.print("U_PR:     ");Telnet.println(U_PR);
        Telnet.print("I_PR:     ");Telnet.println(I_PR,3);
        Telnet.print("P_PR:     ");Telnet.println(P_PR);
        Telnet.print("PPR:     ");Telnet.println(PPR);  
        Telnet.print("PPR anterior:     ");Telnet.println(PPR_ant);        
        Telnet.print("PPR actual month:     ");Telnet.println(PPRmonth,3);
        Telnet.print("PR_F:     ");Telnet.println(PR_F);  
        Telnet.print("PR_PF:     ");Telnet.println(PR_PF);
        Telnet.print("Umbral de aviso:     ");Telnet.println(UmbralAvisoPotencia);
       
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
// ==  NOTIFICACION CONSUMO EN VIVIENDA SI HABITANTES          ===
// ================================================================



void notificacion(){
 

 if (bEnableNotificacion==true){
         if ((Ping.ping(ipOscar)==0)&(Ping.ping(ipOlga)==0)&(P_PR>UmbralAvisoPotencia)&(bAviso==false))
         {
          Blynk.virtualWrite(V100, Ping.ping(ipOscar));  
         Blynk.notify(String("El consumo de la vivienda es de ") + P_PR + " Watts");
         bAviso=true;  }
         else
         {  Blynk.virtualWrite(V100, Ping.ping(ipOscar));
            bAviso=false;
          }
         
         currentMillis = millis();
        
            if(currentMillis - previousMillis > TiempoEntreNotificaciones) {
                    previousMillis = currentMillis;  
                    bAviso=false;
            }
 }
}

// ================================================================
// ===                      BUCLE PRINCIPAL                     ===
// ================================================================

void loop(){
 
  Blynk.run();
  timer1.run();
  timer2.run();
  timer3.run();
  timer4.run();
  
  //handleTelnet();
  httpServer.handleClient();
  MDNS.update();

if (!Blynk.connected()){
  Serial.println("Reiniciando....");
  digitalWrite(4,LOW);} //RESET
 
}


// ================================================================
// ===                      ESCRIBIR TABLA                      ===
// ================================================================

void WriteTable(int mes, int anyo, String Coste, String Energia)
{
  
 String Smes;
 switch (mes) {
    case 2:    
      Smes="ENERO ";
      break;
    case 3:  
      Smes="FEBRERO ";
      break;
    case 4:    
      Smes="MARZO ";
      break;
    case 5:    
      Smes="ABRIL ";
      break;
     case 6:    
      Smes="MAYO ";
      break;
     case 7:    
      Smes="JUNIO ";
      break;
     case 8:    
      Smes="JULIO ";
      break;
     case 9:    
      Smes="AGOSTO ";
      break;
     case 10:    
      Smes="SEPTIEMBRE ";
      break;
     case 11:    
      Smes="OCTUBRE ";
      break; 
     case 12:    
      Smes="NOVIEMBRE ";
      break;
     case 1:  
      Smes="DICIEMBRE ";
      anyo=anyo-1;
      break; }
      
    table.addRow(rowIndex, Smes + String(anyo) + " " + String(minutoactual), Coste + " €" + " | " + Energia + " KWH");
    table.pickRow(rowIndex);
    rowIndex++;
    Blynk.virtualWrite(vPIN_TablaRow, rowIndex);
  
}

BLYNK_WRITE(vPIN_TablaClean) {
  if (param.asInt()) {
    table.clear();
    rowIndex = 0;
  }
}
