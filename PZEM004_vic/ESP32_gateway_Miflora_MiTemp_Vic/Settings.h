

//**********GENERALES**********//

    int LED_BUILTIN = 2;
    #define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
    #define TIME_TO_SLEEP  900//900        /* Time ESP32 will go to sleep (in seconds) */
    #define EMERGENCY_HIBERNATE 1 * 60

    TaskHandle_t hibernateTaskHandle = NULL;

//**********TOKEN (OK)**********//

    char auth[] = "DGIOOJEgVk4p0mT82FrsNu2YFmOX7fsw";

//**********WIFI (OK)**********//

    char ssid[] = "DRDALASL";
    char pass[] = "doradapaellamanitas";


//******VARIABLES DEVICE 1 MiFlora******//


    float temp1;
    int moisture,light,conductivity,freeHeapstart,freeHeapstop;

// FLORA_ADDR "C4:7C:8D:6A:85:05"
    #define FLORA_ADDR "C4:7C:8D:6A:99:85"
    static BLEAddress AddressDev1(FLORA_ADDR);
    
    BLEClient* pClient1;
    static BLERemoteCharacteristic* pRemoteCharacteristic1;
    static BLEUUID serviceUUID1("00001204-0000-1000-8000-00805f9b34fb");
    static BLEUUID uuid_sensor_data1("00001a01-0000-1000-8000-00805f9b34fb");
    static BLEUUID uuid_write_mode1("00001a00-0000-1000-8000-00805f9b34fb");


//******VARIABLES DEVICE 2 TempHum******//

    
    float temp2,hum2;
    int n=0;
    
    
    //#define MJ_ADDR1 "4C:65:A8:DF:32:F4" //VICENTE
    //static BLEAddress MJAddress1(MJ_ADDR1);
    
    #define HUMTEMP_ADDR "4C:65:A8:DF:32:F4" //VICENTE
    static BLEAddress AddressDev2(HUMTEMP_ADDR);
    
    BLEClient* pClient2;
    static BLERemoteCharacteristic* pRemoteCharacteristic2;
    static BLEUUID serviceUUID2("226c0000-6476-4566-7562-66734470666d");
    static BLEUUID    charUUID2("226caa55-6476-4566-7562-66734470666d");
    
    
    static boolean connected = false;
    uint8_t* pData;
