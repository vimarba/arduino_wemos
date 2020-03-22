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

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "d4cf8e6adc9a40b9a2f68cb2163ccbd3";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "DRDALASL";
char pass[] = "doradapaellamanitas";

const int pinRele = D1;

int modocalefaccion = MODO_OFF;
float temperaturadeseada = 22;
int estadocalefaccion = CALEFACCION_OFF;
int contador_sinconexion = 0;
int fallolectura = 0;

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


//LEER DATO MODOCALEFACCION
BLYNK_WRITE(V0)
{
  modocalefaccion = param.asInt(); 
  terminal.print("Leido modo: ");
  terminal.println(modocalefaccion);
  terminal.flush();
  BucleTermostato();
}
//LEER DATO TEMPERATURADESEADA
BLYNK_WRITE(V1)
{
  temperaturadeseada = param.asFloat(); 
  terminal.print("Leida temperatura deseada: ");
  terminal.println(temperaturadeseada);
  terminal.flush(); 
  BucleTermostato();
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

  LedCalefaccion.setColor(BLYNK_RED);
  LedCalefaccion.off();

  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V0);
 
  timer.setInterval(300000L, BucleTermostato);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}

