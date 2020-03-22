



static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
//    temp2 = ((pData[2]-48)*10 + pData[3]-48 + (pData[5]-48)*0.1);
//    hum2 = ((pData[9]-48)*10 + pData[10]-48 + (pData[12]-48)*0.1);
//    if (temp2>55) temp2 = 9.9;

  for (int i = 0; i <= 14; i++) {
  Serial.print(pData[i]); Serial.print(" ");
  }
  Serial.println("");

  int finalstring [2];
  int j = 0;
  for (int i = 0; i <= 14; i++) {
      if (pData[i] == 32) {pData[i] = 0; finalstring[j]=i; j++;}
  }

  for (int i = 0; i <= 14; i++) {
  Serial.print(pData[i]); Serial.print(" ");
  }
  Serial.println("");

  String S = String((char *)(pData + 2));
  Serial.println(S);
  temp2 = S.toFloat();

  S = String((char *)(pData + finalstring[0]+3));
  Serial.println(S);
  hum2 = S.toFloat();
} 




class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient2) {
  }

//  void onDisconnect(BLEClient* pClient2) {
//    connected = false;
//    if (n==0){  
//      pClient2->disconnect();
//      btStop();
//      WiFi.mode(WIFI_OFF);
//      Serial.println("SIN CONEXION BLUETOOTH DEVICE 2");
//      return;
//    }
//  }
};



void getSensorDev2(BLEAddress pAddress){

    BLEDevice::init("");
    pClient2  = BLEDevice::createClient();
  Serial.println("CONECTANDO A DEVICE2 (TEMP_HUM).....");
    n=0;
 //   pClient2->setClientCallbacks(new MyClientCallback());
    pClient2->connect(pAddress);
    n=1;
    BLERemoteService* pRemoteService = pClient2->getService(serviceUUID2);
    pRemoteCharacteristic2 = pRemoteService->getCharacteristic(charUUID2);
    pRemoteCharacteristic2->registerForNotify(notifyCallback);
   delay(3000);//RETRASO NECESARIO NO BORRAR
    PlotSerialValuesDev2();
    EnviarDatoPorBlynkDev2();  
    delay(1000);
}

void PlotSerialValuesDev2(){
  Serial.println("Valores leidos de Sensor: ");
  Serial.print("  Temperatura:");
  Serial.print(temp2); 
  Serial.println("ºC  ");
  Serial.print("  Humedad:"); 
  Serial.print(hum2);
  Serial.println("%");
  }

void EnviarDatoPorBlynkDev2(){


    int ConexionStatus=ConexionBlynk();

    if (ConexionStatus==20){
    
        Blynk.virtualWrite(V1, temp2);
        Blynk.virtualWrite(V2, String((temp2), 2) + String(" ºC")); 
        Blynk.virtualWrite(V3, hum2);
        Blynk.virtualWrite(V4, String((hum2), 2) + String(" %")); 
      
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



    
