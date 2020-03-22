
#include <BLEDevice.h>
#include <BlynkSimpleEsp32.h>
#include "Settings.h"


//FUNCION DE VERIFICACION DE CONEXION CON BLYNK
int ConexionBlynk()
{
  WiFi.begin(ssid, pass); 
  long previousMillis = 0;   
  previousMillis=millis();
  //HAGO LA CONEXION A LA WIFI DURANTE 5 SEG  
  while ((WiFi.status() != WL_CONNECTED)&(millis()-previousMillis<5000)) {
    delay(500);
    Serial.print(".");}
  if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println(" ");
            Blynk.config(auth);  // in place of Blynk.begin(auth, ssid, pass);
            Blynk.connect(3333);  // timeout set to 10 seconds and then continue without Blynk
            
            while (Blynk.connect() == false){
              Blynk.disconnect();
              WiFi.mode(WIFI_OFF);
              return (10);  //SI LA WIFI SE CONECTA Y EL SERVIDOR DE BLYNK NO ENVIO 10
            }            
            return (20);    // SI LA WIFI SE CONECTA Y EL SERVIDOR TAMBIEN ENVIO 20
          }
   
    else {
          return (30);      //SI LA WIFI NO SE CONECTA ENVIO 30
      }
      }


//CONFIRMACION DE ESCRITURA EN BLYNK//
void ConfimacionEscritura()
{
      digitalWrite(LED_BUILTIN,HIGH);
      delay(1500);
      digitalWrite(LED_BUILTIN,LOW);
      delay(500);
  }

//FUNCION PARA HIBERNAR
void hibernate() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Going to sleep now.");
  //INDICACION LUMINOSA DE QUE ESP32 ESTA COLGADO PORQUE NO HA RECIBIDO DATO BLE Y QUE VA A REINICIAR
      for (int i = 0; i < 10; i++) {
      digitalWrite(LED_BUILTIN,HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN,LOW);
      delay(200);
      }
  esp_deep_sleep_start();
}

void delayedHibernate(void *parameter) {
  delay(EMERGENCY_HIBERNATE*1000); // delay for five minutes
  Serial.println("Hibernacion de emergencia, sin conexion BLE...");
  hibernate();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Gateway Device.....");
  pinMode(LED_BUILTIN, OUTPUT);

 

  // crear tarea de hibernacion para el caso que durante 1 minuto se quede colgado el ESP32
  xTaskCreate(delayedHibernate, "hibernate", 4096, NULL, 1, &hibernateTaskHandle);

  getSensorDev2(AddressDev2); 
  getSensorDev1(AddressDev1);  

 // borrar tarea de hibernacion
  vTaskDelete(hibernateTaskHandle);

  hibernate();
}

void loop() {
}
