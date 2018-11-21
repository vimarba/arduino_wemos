#define BLYNK_PRINT Serial
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

WidgetLED LedCalefaccion(V2);
BlynkTimer timer;

void BucleTermostato() {
  bool isconnected = Blynk.connected();
  if (isconnected == true){

    int seleccion = modocalefaccion;
    contador_sinconexion = 0;

    Serial.print("-> Entrada BucleTermostato. Estado actual calefaccion: ");
    Serial.println(estadocalefaccion);

    switch (seleccion) {
      case MODO_OFF:
        Serial.println("Agagando");
        if (estadocalefaccion == CALEFACCION_ON) {
          digitalWrite(pinRele, LOW); //ABRIR RELE
          estadocalefaccion = CALEFACCION_OFF;
          LedCalefaccion.off();
          Blynk.notify("Apagada Calefaccion");
        }
      break;
      case MODO_ON:
        Serial.println("Encendiendo");
        if (estadocalefaccion == CALEFACCION_OFF) {
          digitalWrite(pinRele, HIGH); //CERRAR RELE
          estadocalefaccion = CALEFACCION_ON;
          LedCalefaccion.on();
          Blynk.notify("Encendida Calefaccion");
        }
      break;
      case MODO_AUTO: 
        HTTPClient http;  //Declare an object of class HTTPClient
        float temp_salon, temp_hab, temp_media, temp_deseada_high, temp_deseada_low;

        Serial.println("AUTO");
        
        http.begin("http://blynk-cloud.com/adfbd99f68f345df8be28f73854888ff/get/V1");  
        int httpCode = http.GET(); 
 
        if (httpCode > 0) {
          String payload = http.getString();
          payload = payload.substring(2,8);
          temp_salon = payload.toFloat();
          Serial.print("Temperatura real salon: ");
          Serial.println(temp_salon);
        }

        http.begin("http://blynk-cloud.com/d264e4ccf3814039a6081a612ef08571/get/V1");  
        httpCode = http.GET(); 
 
        if (httpCode > 0) {
          String payload = http.getString();
          payload = payload.substring(2,8);
          temp_hab = payload.toFloat();
          Serial.print("Temperatura real habitacion: ");
          Serial.println(temp_hab);
        }

        http.end();

        temp_media = (temp_salon + temp_hab)/2;
        temp_deseada_high = temperaturadeseada*1.01;
        temp_deseada_low = temperaturadeseada*0.99;
        Serial.print("Temperatura real media: ");
        Serial.println(temp_media);
        Serial.print("Temperatura deseada high: ");
        Serial.println(temp_deseada_high);
        Serial.print("Temperatura deseada low: ");
        Serial.println(temp_deseada_low);

        if ((temp_media < temp_deseada_low) and (estadocalefaccion == CALEFACCION_OFF)) {
            estadocalefaccion = CALEFACCION_ON;
            digitalWrite(pinRele, HIGH); //CERRAR RELE
            LedCalefaccion.on();
            Serial.println("Encendiendo"); 
            Blynk.notify("Encendida Calefaccion");
        }
        
        if ((temp_media > temp_deseada_high) and (estadocalefaccion == CALEFACCION_ON)) {        
            estadocalefaccion = CALEFACCION_OFF;
            digitalWrite(pinRele, LOW); //ABRIR RELE
            LedCalefaccion.off();
            Serial.println("Apagando");
            Blynk.notify("Apagada Calefaccion"); 
        }
      break;
    }
  } else {
    contador_sinconexion++;
    Serial.println("Sin conexion con la nube: ");
    Serial.println(contador_sinconexion);
    if (contador_sinconexion >= 4) {
      digitalWrite(pinRele, LOW); //ABRIR RELE
      Serial.println("Reiniciando ESP");
      ESP.restart();
    }
  }
}


//LEER DATO MODOCALEFACCION
BLYNK_WRITE(V0)
{
  modocalefaccion = param.asInt(); 
  Serial.print("Leido modo: ");
  Serial.println(modocalefaccion);
  BucleTermostato();  
}
//LEER DATO TEMPERATURADESEADA
BLYNK_WRITE(V1)
{
  temperaturadeseada = param.asFloat(); 
  Serial.print("Leida temperatura deseada: ");
  Serial.println(temperaturadeseada); 
  BucleTermostato();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(pinRele, OUTPUT);
  digitalWrite(pinRele, LOW); //ABRIR RELE
  
  Blynk.begin(auth, ssid, pass);
  Serial.println("Booting");

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
