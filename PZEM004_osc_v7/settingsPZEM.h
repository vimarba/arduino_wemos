
/*
   DATOS DE CONEXION
*/
char ssid[] = "PECEZACOS";                  
char pass[] = "tordo123perdiz";             
#define AUTH    "41429b6e3bed4ccbb7cee6cc61ed6fe9"    
const char* host = "ESP8266_ENERGIA";

const IPAddress ipOscar(192, 168, 0, 12); // Remote host
const IPAddress ipOlga(192, 168, 0, 19); // Remote host

   
/* 
   CIRTUAL PINS
*/
#define vPIN_VOLTAGE                   V0
#define vPIN_CURRENT_USAGE             V1
#define vPIN_ACTIVE_POWER              V2
#define vPIN_FREQUENCY                 V4
#define vPIN_POWER_FACTOR              V5
#define vPIN_OVER_POWER_ALARM          V6  
#define vPIN_ENERGITOTAL               V12  
#define vPIN_RRSI                      V13
#define vPIN_FECHA                     V54

#define vPIN_COSTE                     V50
#define vPIN_StrCOSTE                  V55
#define vPIN_CosteMesAnterior          V51
#define vPIN_StrCosteMesAnterior       V52


#define vPIN_EnergiaMesActual          V30
#define vPIN_EnergiaIteracionAnterior  V31
#define vPIN_EnergiaMesAnterior        V32
#define vPIN_StrEnergiaMesAnterior     V33


#define vPIN_UmbralAviso               V53

#define vPIN_ResetManual               V20
#define vPIN_EnableNotificacion        V21

#define vPIN_Tabla                     V40
#define vPIN_TablaRow                  V41
#define vPIN_TablaClean                V42
