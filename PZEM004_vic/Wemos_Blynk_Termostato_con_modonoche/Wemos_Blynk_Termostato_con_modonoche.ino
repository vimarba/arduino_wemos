#define BLYNK_PRINT     Serial
#define BLYNK_GREEN     "#23C48E"
#define BLYNK_BLUE      "#04C0F8"
#define BLYNK_YELLOW    "#ED9D00"
#define BLYNK_RED       "#D3435C"
#define BLYNK_DARK_BLUE "#5F7CD8"

#define CALEFACCION_OFF   0
#define CALEFACCION_ON    1

#define MODO_OFF  1
#define MODO_ON   2
#define MODO_AUTO 3
#define MODO_NOCHE 4

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <SNTPtime.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266HTTPUpdateServer.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "d4cf8e6adc9a40b9a2f68cb2163ccbd3";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "DRDALASL";
char pass[] = "doradapaellamanitas";

SNTPtime NTPch("ch.pool.ntp.org");

const long tiempobucle = 300;  //segundos
const int pinRele = D1;

int modocalefaccion = MODO_OFF;
float temperaturadeseada = 20;
float temperaturadeseada_noche = 14;
int estadocalefaccion = CALEFACCION_OFF;
int contador_sinconexion = 0;
int fallolectura = 0;
long stopTimeinSeconds = 81000;  //22:30
int HorastopTime, MinutosstopTime;
boolean arranque=true;

//ESP8266WebServer httpServer(80);
//ESP8266HTTPUpdateServer httpUpdater;

WidgetLED LedCalefaccion(V2);
WidgetTerminal terminal(V3);
BlynkTimer timer;

void BucleTermostato() {
  bool isconnected = Blynk.connected();
  if (isconnected == true){

    int seleccion = modocalefaccion;
    contador_sinconexion = 0;

    terminal.print("-> BucleTermostato. Estado actual calefaccion: ");
    terminal.println(estadocalefaccion);

    switch (seleccion) {
      case MODO_OFF:
        terminal.println("Modo: OFF");
        if (estadocalefaccion == CALEFACCION_ON) {
          terminal.println("Apagando");
          digitalWrite(pinRele, LOW); //ABRIR RELE
          estadocalefaccion = CALEFACCION_OFF;
          LedCalefaccion.off();
          Blynk.notify("Apagada Calefaccion");
        }
      break;
      case MODO_ON:
        terminal.println("Modo: ON");
        if (estadocalefaccion == CALEFACCION_OFF) {
          terminal.println("Encendiendo");
          digitalWrite(pinRele, HIGH); //CERRAR RELE
          estadocalefaccion = CALEFACCION_ON;
          LedCalefaccion.on();
          Blynk.notify("Encendida Calefaccion modo MANUAL");
        }
      break;
      case MODO_AUTO: 
        {
        HTTPClient http;  //Declare an object of class HTTPClient
        float temp_salon, temp_hab, temp_media, temp_deseada_high, temp_deseada_low;

        terminal.println("Modo: AUTO");
        
        http.begin("http://blynk-cloud.com/adfbd99f68f345df8be28f73854888ff/get/V1");  
        int httpCode = http.GET(); 
 
        if (httpCode > 0) {
          String payload = http.getString();
          payload = payload.substring(2,8);
          temp_salon = payload.toFloat();
          terminal.print("Temperatura real salon: ");
          terminal.println(temp_salon);
          if ((temp_salon < 2) or (temp_salon > 50)) {
            fallolectura++;
            terminal.print("Fallo lectura de datos: ");
            terminal.println(fallolectura);
            http.end();
            break;
          }
        } else {
          fallolectura++;
          terminal.print("Fallo lectura de datos: ");
          terminal.println(fallolectura);
          http.end();
          break;
        }

        http.begin("http://blynk-cloud.com/d264e4ccf3814039a6081a612ef08571/get/V1");  
        httpCode = http.GET(); 
 
        if (httpCode > 0) {
          String payload = http.getString();
          payload = payload.substring(2,8);
          temp_hab = payload.toFloat();
          terminal.print("Temperatura real habitacion: ");
          terminal.println(temp_hab);
          if ((temp_hab < 2) or (temp_hab > 50)) {
            fallolectura++;
            terminal.print("Fallo lectura de datos: ");
            terminal.println(fallolectura);
            http.end();
            break;
          }
        } else {
          fallolectura++;
          terminal.print("Fallo lectura de datos: ");
          terminal.println(fallolectura);
          http.end();
          break;
        }

        http.end();

        fallolectura = 0;

        temp_media = (temp_salon + temp_hab)/2;
        temp_deseada_high = temperaturadeseada*1.005;
        temp_deseada_low = temperaturadeseada*0.995;
        terminal.print("Temperatura real media: ");
        terminal.println(temp_media);
        terminal.print("Temperatura low/deseada/high: ");
        terminal.print(temp_deseada_low);
        terminal.print("/");
        terminal.print(temperaturadeseada);
        terminal.print("/");
        terminal.println(temp_deseada_high);

        if ((temp_media < temp_deseada_low) and (estadocalefaccion == CALEFACCION_OFF)) {
            estadocalefaccion = CALEFACCION_ON;
            digitalWrite(pinRele, HIGH); //CERRAR RELE
            LedCalefaccion.on();
            terminal.println("Encendiendo"); 
            Blynk.notify("Encendida Calefaccion modo AUTO");
        }
        
        if ((temp_media > temp_deseada_high) and (estadocalefaccion == CALEFACCION_ON)) {        
            estadocalefaccion = CALEFACCION_OFF;
            digitalWrite(pinRele, LOW); //ABRIR RELE
            LedCalefaccion.off();
            terminal.println("Apagando");
            Blynk.notify("Apagada Calefaccion modo AUTO"); 
        }
        }
      break;
      case MODO_NOCHE: 
        {
        HTTPClient http;  //Declare an object of class HTTPClient
        float temp_salon, temp_hab, temp_media, temp_deseada_high, temp_deseada_low;

        terminal.println("Modo: NOCHE");
        
        http.begin("http://blynk-cloud.com/adfbd99f68f345df8be28f73854888ff/get/V1");  
        int httpCode = http.GET(); 
 
        if (httpCode > 0) {
          String payload = http.getString();
          payload = payload.substring(2,8);
          temp_salon = payload.toFloat();
          terminal.print("Temperatura real salon: ");
          terminal.println(temp_salon);
          if ((temp_salon < 2) or (temp_salon > 50)) {
            fallolectura++;
            terminal.print("Fallo lectura de datos: ");
            terminal.println(fallolectura);
            http.end();
            break;
          }
        } else {
          fallolectura++;
          terminal.print("Fallo lectura de datos: ");
          terminal.println(fallolectura);
          http.end();
          break;
        }

        http.begin("http://blynk-cloud.com/d264e4ccf3814039a6081a612ef08571/get/V1");  
        httpCode = http.GET(); 
 
        if (httpCode > 0) {
          String payload = http.getString();
          payload = payload.substring(2,8);
          temp_hab = payload.toFloat();
          terminal.print("Temperatura real habitacion: ");
          terminal.println(temp_hab);
          if ((temp_hab < 2) or (temp_hab > 50)) {
            fallolectura++;
            terminal.print("Fallo lectura de datos: ");
            terminal.println(fallolectura);
            http.end();
            break;
          }
        } else {
          fallolectura++;
          terminal.print("Fallo lectura de datos: ");
          terminal.println(fallolectura);
          http.end();
          break;
        }

        http.end();

        fallolectura = 0;

        temp_media = (temp_salon + temp_hab)/2;
        temp_deseada_high = temperaturadeseada_noche*1.005;
        temp_deseada_low = temperaturadeseada_noche*0.995;
        terminal.print("Temperatura real media: ");
        terminal.println(temp_media);
        terminal.print("Temperatura low/deseada/high: ");
        terminal.print(temp_deseada_low);
        terminal.print("/");
        terminal.print(temperaturadeseada_noche);
        terminal.print("/");
        terminal.println(temp_deseada_high);

        if ((temp_media < temp_deseada_low) and (estadocalefaccion == CALEFACCION_OFF)) {
            estadocalefaccion = CALEFACCION_ON;
            digitalWrite(pinRele, HIGH); //CERRAR RELE
            LedCalefaccion.on();
            terminal.println("Encendiendo"); 
            Blynk.notify("Encendida Calefaccion modo AUTO");
        }
        
        if ((temp_media > temp_deseada_high) and (estadocalefaccion == CALEFACCION_ON)) {        
            estadocalefaccion = CALEFACCION_OFF;
            digitalWrite(pinRele, LOW); //ABRIR RELE
            LedCalefaccion.off();
            terminal.println("Apagando");
            Blynk.notify("Apagada Calefaccion modo AUTO"); 
        }
        }
      break;          
    }
    if (fallolectura >= 4) {
      estadocalefaccion = CALEFACCION_OFF;
      digitalWrite(pinRele, LOW); //ABRIR RELE
      LedCalefaccion.off();
      terminal.println("Forzando apagado por fallo en lectura de datos de temperatura");
      if (fallolectura == 4) {
        Blynk.notify("Apagada Calefaccion por fallo de lectura");
      }
    }

// Comprobar si hay que pasar a modo noche //
  if (conmutarnoche()) {
    if (seleccion == MODO_ON) {
      Blynk.virtualWrite(V0, MODO_OFF);
      modocalefaccion = MODO_OFF;
      terminal.println("----- Apagado Automatico Calefaccion ----- ");
    }
    if (seleccion == MODO_AUTO) {
      Blynk.virtualWrite(V0, MODO_NOCHE);
      modocalefaccion = MODO_NOCHE;
      terminal.println("----- Conmutacion Automatica Calefaccion a nodo NOCHE ----- ");
    }
  }
  
  } else {
    contador_sinconexion++;
    terminal.println("Sin conexion con la nube: ");
    terminal.println(contador_sinconexion);
    if (contador_sinconexion >= 4) {
      digitalWrite(pinRele, LOW); //ABRIR RELE
      terminal.println("Reiniciando ESP");
      ESP.restart();
    }
  }
  terminal.flush();
}

boolean conmutarnoche(){
  long TimeinSeconds;
  boolean retorno;
  
  strDateTime dateTime = NTPch.getTime(1.0, 1);

  TimeinSeconds = dateTime.hour*3600 + dateTime.minute*60;

  terminal.println("Hora Actual: " + String(dateTime.hour) + ":" + String(dateTime.minute));

  terminal.println("Hora Parada: " + String(HorastopTime) + ":" + String(MinutosstopTime));

  if ((stopTimeinSeconds <= TimeinSeconds) && ((stopTimeinSeconds + tiempobucle) > TimeinSeconds)) {
    retorno = true;
    terminal.println("Conmutar a off o noche");
  } else {
    retorno = false;
  }
  return retorno;   
}

//LEER DATO MODOCALEFACCION
BLYNK_WRITE(V0)
{
  modocalefaccion = param.asInt(); 
  terminal.print("Leido modo: ");
  terminal.println(modocalefaccion);
  terminal.flush();
  if (!arranque) BucleTermostato();
}
//LEER DATO TEMPERATURADESEADA
BLYNK_WRITE(V1)
{
  temperaturadeseada = param.asFloat(); 
  terminal.print("Leida temperatura deseada: ");
  terminal.println(temperaturadeseada);
  terminal.flush(); 
  if (!arranque) BucleTermostato();
}
//LEER DATO TEMPERATURADESEADA NOCHE
BLYNK_WRITE(V4)
{
  temperaturadeseada_noche = param.asFloat(); 
  terminal.print("Leida temperatura deseada noche: ");
  terminal.println(temperaturadeseada_noche);
  terminal.flush(); 
  if (!arranque) BucleTermostato();
}
//LEER DATO HORA MODO NOCHE
BLYNK_WRITE(V5)
{
  stopTimeinSeconds = param[0].asLong(); 
  terminal.print("Hora de modo noche automatico: ");
  terminal.println(stopTimeinSeconds);
  terminal.flush();
  div_t t1 = div(stopTimeinSeconds,3600);
  div_t t2 = div(t1.rem,60);
  HorastopTime = t1.quot;
  MinutosstopTime = t2.quot;
  terminal.println("Hora Parada: " + String(HorastopTime) + ":" + String(MinutosstopTime));
   
  if (!arranque) BucleTermostato();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(pinRele, OUTPUT);
  digitalWrite(pinRele, LOW); //ABRIR RELE
  
  Blynk.begin(auth, ssid, pass);
  terminal.clear();
  terminal.println("Booting");
  terminal.flush();

  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //  terminal.print(".");
  //}
  //terminal.flush();

  //------FUNCIONES PARA OBTENER HORA-------////
  while (!NTPch.setSNTPtime()) terminal.print("."); // set internal clock
  terminal.println();
  terminal.println("Time set"); 
  terminal.flush();
  //------FUNCIONES PARA OBTENER HORA-------////

  LedCalefaccion.setColor(BLYNK_RED);
  LedCalefaccion.off();

  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V5);

  arranque=false;A

  //------FUNCIONES PARA HTTP UPDATER-------////
  //httpUpdater.setup(&httpServer);
  //httpServer.begin();
  //terminal.println("HTTPUpdateServer ready! Open http://<IP>/update   --> IP:" + WiFi.localIP());
  //------FUNCIONES PARA HTTP UPDATER-------////
 
  timer.setInterval(tiempobucle*1000, BucleTermostato);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}
