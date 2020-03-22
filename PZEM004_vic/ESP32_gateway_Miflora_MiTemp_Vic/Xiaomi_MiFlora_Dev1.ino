


  
void getSensorDev1(BLEAddress pAddress){

  BLEDevice::init("");
  pClient1 = BLEDevice::createClient();
 
  btStart();
  Serial.println("CONECTANDO A DEVICE1 (MiFlora).....");
  if (!pClient1->connect(pAddress)){
  pClient1->disconnect();
  Serial.println(" -NOT Connected to Flora");
  return; 
  } 
  BLERemoteService* pRemoteService = pClient1->getService(serviceUUID1);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID1.toString().c_str());
  } 
  pRemoteCharacteristic1 = pRemoteService->getCharacteristic(uuid_write_mode1);
  uint8_t buf[2] = {0xA0, 0x1F};
  pRemoteCharacteristic1->writeValue(buf, 2, true);
  pRemoteCharacteristic1 = pRemoteService->getCharacteristic(uuid_sensor_data1);
  if (pRemoteCharacteristic1 == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(uuid_sensor_data1.toString().c_str());
  }  
  std::string value = pRemoteCharacteristic1->readValue();
  const char *val = value.c_str();

  temp1 = (val[0] + val[1] * 256) / ((float)10.0);
  moisture = val[7];
  light = val[3] + val[4] * 256;
  conductivity = val[8] + val[9] * 256;
  
  char buffer[64];
  
  pClient1->disconnect();
  delay(500);
  btStop();
  PlotSerialValuesDev1();
  EnviarDatoPorBlynkDev1();

}

void PlotSerialValuesDev1(){
  Serial.println("Valores leidos del Sensor: ");
  Serial.print("  Temperatura:");
  Serial.println(temp1); 
  Serial.print("  Humedad: "); 
  Serial.println(moisture);
  Serial.print("  Luminosidad "); 
  Serial.println(light);
  Serial.print("  Conductividad "); 
  Serial.println(conductivity);
  }

void EnviarDatoPorBlynkDev1(){

  int ConexionStatus=ConexionBlynk();

    if (ConexionStatus==20){
        Serial.println("Connected to Blynk server");
        Blynk.virtualWrite(V5, temp1);
        Blynk.virtualWrite(V6, String((temp1), 2) + String(" ºC")); 
        Blynk.virtualWrite(V7, moisture);
        Blynk.virtualWrite(V8, String((moisture), 2) + String(" %")); 
        Blynk.virtualWrite(V9, light);
        Blynk.virtualWrite(V10, String((light), 2) + String(" lum")); 
        Blynk.virtualWrite(V11, conductivity);
        Blynk.virtualWrite(V12, String((conductivity), 2) + String(" uS")); 
      
        Serial.println("Valores registrados en blynk correctamente");
        ConfimacionEscritura();
        Blynk.disconnect();
        WiFi.mode(WIFI_OFF);
      }
      else if (ConexionStatus==30){
        Serial.println("WiFi Not connected");
        Serial.println("Valores No registrados");
        Blynk.disconnect();
        WiFi.mode(WIFI_OFF);
      return; 
        }
      else if (ConexionStatus==10){
        Serial.println("Not connected to Blynk server");
        Serial.println("Valores No registrados");
        Blynk.disconnect();
        WiFi.mode(WIFI_OFF);
      return; 
        }
        }
     
