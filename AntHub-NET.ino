/*

AntHub-NET
----------------------

___               _        ___ _____ _  _
| _ \___ _ __  ___| |_ ___ / _ \_   _| || |  __ ___ _ __
|   / -_) '  \/ _ \  _/ -_) (_) || | | __ |_/ _/ _ \ '  \
|_|_\___|_|_|_\___/\__\___|\__\_\|_| |_||_(_)__\___/_|_|_|


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Send test packet ANT
  $ mosquitto_pub -h 192.168.1.200 -d -t OK1HRA/OI3/1/hz -m "21060081"
Remote USB access
  screen /dev/ttyUSB0 115200

HARDWARE ESP32-POE

Changelog:
20221029 - initial version

ToDo
- LCD https://squareline.io/ +

Použití knihovny BluetoothSerial ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/BluetoothSerial

Použití knihovny WiFi ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/WiFi
Použití knihovny EEPROM ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/EEPROM
Použití knihovny Ethernet ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/Ethernet
Použití knihovny ESPmDNS ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/ESPmDNS
Použití knihovny ArduinoOTA ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/ArduinoOTA
Použití knihovny Update ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/Update
Použití knihovny PubSubClient ve verzi 2.8 v adresáři: /home/dan/Arduino/libraries/PubSubClient
Použití knihovny FS ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/FS
Použití knihovny SD_MMC ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/SD_MMC
Použití knihovny SPI ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/SPI
Použití knihovny Adafruit_GFX_Library ve verzi 1.11.3 v adresáři: /home/dan/Arduino/libraries/Adafruit_GFX_Library
Použití knihovny Adafruit_BusIO ve verzi 1.14.1 v adresáři: /home/dan/Arduino/libraries/Adafruit_BusIO
Použití knihovny Wire ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/Wire
*/
//-------------------------------------------------------------------------------------------------------
// #define Ser2net                  // Serial to ip proxy
#define EnableOTA                // Enable flashing ESP32 Over The Air
#define TFTLCD                     //
#define ETHERNET                    // Enable ESP32 ethernet (DHCP IPv4)
#define ETH_ADDR 0
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_POWER 5                // mosfet on VDDIO //12
#define ETH_MDC 23                  // MDC pin17
#define ETH_MDIO 18                 // MDIO pin16
#define ETH_CLK ETH_CLOCK_GPIO17_OUT    // CLKIN pin5 | settings for ESP32 GATEWAY rev f-g

//-------------------------------------------------------------------------------------------------------
const char* REV = "20260707";
char hardware[] = "ANT";
// const char* HWNAME = "IP-ROT";
int ANT = 8;
unsigned int TRX=1;
unsigned int TRXselect=0;
unsigned long TRXfreq[2];

//--- ANT configure ------------------------------
unsigned long ANTrange[16][2] = {/* TRXfreq[0]
Freq Hz from       to          ANT
*/   {1810000,  52000000},  // #1
     {0,   0},  // #2
     {0,   0},  // #3
     {0,   0},  // #4
     {0,   0},  // #5
     {0,   0},  // #6
     {0,   0},  // #7
     {0,   0},  // #8

     {1810000,  7200000},   // #9
    {10100000,  10150000},  // #10
    {14000000,  14350000},  // #11
    {21000000,  21450000},  // #12
    {28000000,  29700000},  // #13
    {50000000,  52000000},  // #14
   {0, 0},  // #15
   {0, 0},  // #16
};
int TRXselectANT[2] = {42,42}; // how antenna selcted - blocked for second TRX - don't be the same!
int AvailableANTpool[16][2];
bool TXANT[16] = {1,0,0,0,0,0,0,0, 1,1,1,1,1,1,0,0};  // 1=TX, 0=RX only
String ANTname[16] = {
"Dummy",
"free",
"free",
"free",
"free",
"free",
"free",
"free",

"Vertical",
"Dipole",
"Quad",
"Quad",
"Quad",
"Yagi",
"free",
"free"
};
const int ShiftOutDataPin  = 33;
const int ShiftOutLatchPin = 32;
const int ShiftOutClockPin = 12;
byte ShiftOutByte[4];

//---------------------------------



// unsigned int DeactRxonlyTx[Outputs]={2,2,2,2,2,2,2,2, 1,1,1,1,0,0,0,0};
// bool SelectTXbutton[Outputs][2];
// bool PTT[TRX];
// unsigned int Groups[Outputs]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

// https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
// TEST |= 1UL << i; // SetBit
// bit = (TEST >> i) & 1U; // CheckBit
// TEST &= ~(1UL << i);  // ClearBit
/*
informace ke kazdemu vystupu 16 x 4

staticke
- band(s) - muze byt aktivni na vice pasmech 10+, 0=nepouzito
- RX only - neni urcen pro TX vyber
- ANTname (8 znaku?)
- groups

dynamicke
- TX used - 4 TRX
- client PTT on/off spotvrzovanim prepnuti mezi rx/tx nastavenim
*/





String YOUR_CALL = "";
long MeasureTimer[2]={2800000,1800000};

// used by LcdDisplay()
bool RXonly[16] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};
int Groups[16]  = {1,2,3,4,5,6,7,8, 9,10,11,12,13,14,15,16};

// 73 seconds WDT (WatchDogTimer)
#include <esp_task_wdt.h>
#define WDT_TIMEOUT 73
long WdtTimer=0;

byte InputByte[21];
int EnableSerialDebug     = 2;
#define HTTP_SERVER_PORT  80     // Web server port

const int SERIAL_BAUDRATE = 115200;
int incomingByte = 0;

int i = 0;
#include <WiFi.h>
#include "EEPROM.h"
#define EEPROM_SIZE 510   /* 512 max
0    -TftRotation
1    -ANT
2    -TRX
3-13
14-17  - SERIAL1_BAUDRATE
18-21 - SerialServerIPport
22-25 - IncomingSwitchUdpPort
26-29 - RebootWatchdog
30-33 - OutputWatchdog
34    - Master
35    - TRXselect
36
37-40 - Authorised telnet client IP
41-140 - Authorised telnet client key
141-160 - YOUR_CALL
161-164 - MQTT broker IP
165-168? - MQTT broker port
169-201  - QRVbands[16] // 16bit/band for each ant
202-203  - RXonly[16];  // 1bit (ON/OFF) by ant
204-220  - Groups[16];     // 1-16
221-380  - ANTname[16];   // 10 char by ant
381-509  - Freq2Band[16][2]

!! Increment EEPROM_SIZE #define !!

*/
unsigned int RebootWatchdog;
unsigned int OutputWatchdog;
unsigned long WatchdogTimer=0;

WiFiServer server(HTTP_SERVER_PORT);
bool DHCP_ENABLE = 1;
char linebuf[80];
int charcount=0;
bool connected = false;
#include <ETH.h>
static bool eth_connected = false;
String HTTP_req;
#if defined(EnableOTA)
  #include <ESPmDNS.h>
  #include <ArduinoOTA.h>
#endif

#define MQTT               // Enable MQTT debug
// #if defined(MQTT)
//   #include <PubSubClient.h>
//   // #include "PubSubClient.h" // lokalni verze s upravou #define MQTT_MAX_PACKET_SIZE 128
//   // WiFiClient esp32Client;
//   // PubSubClient mqttClient(esp32Client);
//   WiFiClient espClient;
//   PubSubClient mqttClient(espClient);
//   // PubSubClient mqttClient(ethClient);
//   // PubSubClient mqttClient(server, 1883, callback, ethClient);
//   long lastMqttReconnectAttempt = 0;
  bool HeartBeatCfm = true;
  unsigned long HeartBeatCounter;
  unsigned long HeartBeatTimer;
  unsigned long LatencyTimer;
//   bool MQTT_ENABLE     = 1;          // enable public to MQTT broker
//   IPAddress mqtt_server_ip(0, 0, 0, 0);
//   // byte BrokerIpArray[2][4]{
//   //   // {192,168,1,200},   // MQTT broker remoteqth.com
//   //   {54,38,157,134},   // MQTT broker remoteqth.com
//   // };
//   // IPAddress server(10, 24, 213, 92);    // MQTT broker
//   int MQTT_PORT;       // MQTT broker PORT
//   // int MQTT_PORT_Array[2] = {
//   //   1883,
//   //   1883
//   // };       // MQTT broker PORT
//   boolean MQTT_LOGIN      = 0;          // enable MQTT broker login
//   // char MQTT_USER= 'login';    // MQTT broker user login
//   // char MQTT_PASS= 'passwd';   // MQTT broker password
//   const int MqttBuferSize = 1000; // 1000
//   char mqttTX[MqttBuferSize];
//   char mqttPath[MqttBuferSize];
//   // char mqttTX[100];
//   // char mqttPath[100];
//   long MqttStatusTimer[2]{1500,1000};
//   // long HeartBeatTimer[2]={0,1000};
// #endif

// MQTT
#include <PubSubClient.h>  // #include "PubSubClient.h" // lokalni verze s upravou #define MQTT_MAX_PACKET_SIZE 128
WiFiClient espClient;
PubSubClient mqttClient(espClient);
long lastMqttReconnectAttempt = 0;
boolean MQTT_ENABLE     = 1;          // enable public to MQTT broker
IPAddress mqtt_server_ip(0, 0, 0, 0);
int MQTT_PORT;       // MQTT broker PORT
boolean MQTT_LOGIN      = 0;          // enable MQTT broker login
// char MQTT_USER= 'login';    // MQTT broker user login
// char MQTT_PASS= 'passwd';   // MQTT broker password
const int MqttBuferSize = 1000; // 1000
char mqttTX[MqttBuferSize];
char mqttPath[MqttBuferSize];
long MqttStatusTimer[2]{1500,1000};
// long HeartBeatTimer[2]={0,1000};

// TrxNet
#include <TrxNet.h>
WiFiUDP  trxUdp;
TrxNet   net(trxUdp);
bool     trxNetEnabled   = false;
char     trxnetAntId[]   = "01";
char     trxnetTrx1Name[]= "705.01";
char     trxnetTrx2Name[]= "OI3.02";
uint16_t trxnetPort      = 5683;
// Priority name-prefixes: peers whose name begins with one of these are protected
// from eviction when the peer table (TRXNET_MAX_PEERS) fills — the stalest
// non-priority peer is dropped instead. Kept in writable buffers so a future
// config web can overwrite them at runtime. See TrxNet::setPriorityPrefixes().
char        trxnetPrio[2][8]  = { "OI3", "705" };
const char* trxnetPrioPtr[2]  = { trxnetPrio[0], trxnetPrio[1] };

// https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/
// #include <Wire.h>
// #define I2C_SDA 33
// #define I2C_SCL 32

#if defined(BMP280)||defined(HTU21D)
  #include <SPI.h>
  #include <Adafruit_Sensor.h>
  TwoWire I2Cone = TwoWire(0);
#endif

#if defined(BMP280)
  #include <Adafruit_BMP280.h>
  Adafruit_BMP280 bmp(&I2Cone); // use I2C interface
  Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
  Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
  bool BMP280enable;
#endif

#if defined(HTU21D)
  #include "Adafruit_HTU21DF.h"
  Adafruit_HTU21DF htu = Adafruit_HTU21DF();
  bool HTU21Denable;
#endif

// WX end

// SD
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12
// #include "FS.h"
// #include "SD_MMC.h"

// ntp
#include "time.h"
const char* ntpServer = "pool.ntp.org";
// const char* ntpServer = "tik.cesnet.cz";
// const char* ntpServer = "time.google.com";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

#define MAX_SRV_CLIENTS 1

int TelnetServerIPport = 23;
WiFiServer TelnetServer;
WiFiClient TelnetServerClients[MAX_SRV_CLIENTS];
bool TelnetAuthorized = false;
bool FirstListCommands=true;

int CompareInt;

// DS18B20
#if defined(DS18B20)
  #include <OneWire.h>
  #include <DallasTemperature.h>
  // const int DsPin = 3;
  // OneWire ds(DsPin);
  // DallasTemperature sensors(&ds);
  const int oneWireBus = 3;
  OneWire oneWire(oneWireBus);
  DallasTemperature sensors(&oneWire);
  bool ExtTemp = false;
#endif

#if defined(TFTLCD)
  int Ybutons = 3;
  int BodyContent = 0;
  byte TftRotation = 3;        // tft.setRotation(3)
  byte LcdNeedRefresh = B11111111;

  // https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts

  /***************************************************
  This is an example made by Adafruit and modifed by Olimex for MOD-LCD2.8RTP
  This demo was tested with Olimex MOD-LCD2.8RTP and ESP32-EVB and OLIMEXINO-2560.
  The boards were connected via UEXT connector and cable.

  Make sure to establish proper hardware connections with your board.
  The display requires SPI, the touschreen I2C. Refer to Board_Pinout.h.

  The original example is a GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the link above for Adafruit's tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing the open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
  ****************************************************/

  // In order to work you have to install Adafruit GFX Library
  // To do so go to:
  // Main menu --> Sketch --> Inlcude Librariy --> Manage Libraries...
  // In the search box filter "Adafruit GFX Library" and install it
  // Tested with version 1.2.3 of the library

  // #include "Board_Pinout.h"
  #include "SPI.h"
  #include "Adafruit_GFX.h"
  #include "Adafruit_ILI9341.h"
  #include "Wire.h"
  #include "Adafruit_STMPE610.h"

  // This is calibration data for the raw touch data to the screen coordinates
  #define TS_MINX 575
  #define TS_MINY 530
  #define TS_MAXX 7700
  #define TS_MAXY 7700
  #define TS_I2C_ADDRESS 0x4d

  // This is pinouts for ESP32-EVB
  #define TFT_DC 15
  #define TFT_CS 5
  #define TFT_MOSI 2
  #define TFT_CLK 14

  Adafruit_STMPE610 ts = Adafruit_STMPE610();

  // Size of the color selection boxes and the paintbrush size
  #define BOXSIZE 40

  Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

  uint8_t tp[5];
  /*
  #define ILI9341_BLACK       0x0000  ///<   0,   0,   0
  #define ILI9341_NAVY        0x000F  ///<   0,   0, 123
  #define ILI9341_DARKGREEN   0x03E0  ///<   0, 125,   0
  #define ILI9341_DARKCYAN    0x03EF  ///<   0, 125, 123
  #define ILI9341_MAROON      0x7800  ///< 123,   0,   0
  #define ILI9341_PURPLE      0x780F  ///< 123,   0, 123
  #define ILI9341_OLIVE       0x7BE0  ///< 123, 125,   0
  #define ILI9341_LIGHTGREY   0xC618  ///< 198, 195, 198
  #define ILI9341_DARKGREY    0x7BEF  ///< 123, 125, 123
  #define ILI9341_BLUE        0x001F  ///<   0,   0, 255
  #define ILI9341_GREEN       0x07E0  ///<   0, 255,   0
  #define ILI9341_CYAN        0x07FF  ///<   0, 255, 255
  #define ILI9341_RED         0xF800  ///< 255,   0,   0
  #define ILI9341_MAGENTA     0xF81F  ///< 255,   0, 255
  #define ILI9341_YELLOW      0xFFE0  ///< 255, 255,   0
  #define ILI9341_WHITE       0xFFFF  ///< 255, 255, 255
  #define ILI9341_ORANGE      0xFD20  ///< 255, 165,   0
  #define ILI9341_GREENYELLOW 0xAFE5  ///< 173, 255,  41
  #define ILI9341_PINK        0xFC18  ///< 255, 130, 198
  */

#endif

// LCD encoder pins (unused, reserved)
const int EncAPin = 37;
const int EncBPin = 35;
// const int PttDetectorPin = 37;
// const int TXinhibitPin = 32;

//-------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  while(!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (!EEPROM.begin(EEPROM_SIZE)){
      Serial.println("failed to initialise EEPROM"); delay(1);
  }

  //0    -TftRotation
  if(EEPROM.readByte(0)<4){
    #if defined(TFTLCD)
      TftRotation=EEPROM.readByte(0);
    #endif
  }
  //1    -ANT
  if(EEPROM.readByte(1)<17){
    ANT=EEPROM.readByte(1);
  }
  //2    -TRX
  if(EEPROM.readByte(2)<0x05){
    TRX=EEPROM.readByte(2);
  }


  RebootWatchdog=EEPROM.readUInt(26);
  if(RebootWatchdog>10080){
    RebootWatchdog=0;
  }
  OutputWatchdog=EEPROM.readUInt(30);
  if(OutputWatchdog>10080){
    OutputWatchdog=0;
  }

  // 35    - TRXselect
  if(EEPROM.readByte(35)<5){
    TRXselect=EEPROM.readByte(35);
  }

  // YOUR_CALL
  // use MAC, move after ETH init
  for (int i=141; i<161; i++){
    if(EEPROM.read(i)!=0xff){
      YOUR_CALL=YOUR_CALL+char(EEPROM.read(i));
    }
  }


  #if defined(MQTT)
    // MQTT broker IP
    for(int i=0; i<4; i++){
      mqtt_server_ip[i]=EEPROM.readByte(i+161);
    }
    MQTT_PORT = EEPROM.readInt(165);
    if(mqtt_server_ip[0]==255 && mqtt_server_ip[1]==255 && mqtt_server_ip[2]==255 && mqtt_server_ip[3]==255 && MQTT_PORT==-1){
      mqtt_server_ip[0]=192;
      mqtt_server_ip[1]=168;
      mqtt_server_ip[2]=1;
      mqtt_server_ip[3]=200;
      // mqtt_server_ip[0]=54;
      // mqtt_server_ip[1]=38;
      // mqtt_server_ip[2]=157;
      // mqtt_server_ip[3]=134;
      MQTT_PORT=1883;
    }
  #endif

  Serial.println("===============================");
  Serial.println("  press '?' for list commands");
  Serial.println("===============================");

  #if defined(ETHERNET)
    // mqtt_server_ip=BrokerIpArray[0];
    // MQTT_PORT = MQTT_PORT_Array[0];

    WiFi.onEvent(EthEvent);
    // ETH.begin();
    ETH.begin(ETH_ADDR, ETH_POWER, ETH_MDC, ETH_MDIO, ETH_TYPE, ETH_CLK);
    if(DHCP_ENABLE==false){
      ETH.config(IPAddress(192, 168, 1, 188), IPAddress(192, 168, 1, 255),IPAddress(255, 255, 255, 0),IPAddress(8, 8, 8, 8));
      //config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0x00000000, IPAddress dns2 = (uint32_t)0x00000000);
    }
    server.begin();
    // chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
    //   unsigned long long1 = (unsigned long)((chipid & 0xFFFF0000) >> 16 );
    //   unsigned long long2 = (unsigned long)((chipid & 0x0000FFFF));
    //   ChipidHex = String(long1, HEX) + String(long2, HEX); // six octets
    //   YOUR_CALL=ChipidHex;

    // EEPROM YOUR_CALL
    if(EEPROM.read(141)==0xff){
      YOUR_CALL=String(ETH.macAddress()[0], HEX)+String(ETH.macAddress()[1], HEX)+String(ETH.macAddress()[2], HEX)+String(ETH.macAddress()[3], HEX)+String(ETH.macAddress()[4], HEX)+String(ETH.macAddress()[5], HEX);
    }else{
      // for (int i=141; i<161; i++){
      //   if(EEPROM.read(i)!=0xff){
      //     YOUR_CALL=YOUR_CALL+char(EEPROM.read(i));
      //   }
      // }
    }

  #endif

  #if defined(TFTLCD)
    // pinMode(StatusLedAPin, OUTPUT);
    //  digitalWrite(StatusLedAPin, LOW);
    // pinMode(StatusLedBPin, OUTPUT);
    //  digitalWrite(StatusLedBPin, LOW);
    // delay(1000);

    tft.begin();
    Wire.begin();
    pinMode(TFT_DC, OUTPUT);
    // read diagnostics (optional but can help debug problems)
    //uint8_t x = tft.readcommand8(ILI9341_RDMODE);
    delay(1000);
    ts.begin(TS_I2C_ADDRESS);
    // Clear Screen
    tft.fillScreen(ILI9341_BLACK);
    tft.setRotation(TftRotation);
  #endif

  #if defined(EnableOTA)
    // Port defaults to 3232
    // ArduinoOTA.setPort(3232);
    // Hostname defaults to esp3232-[MAC]
    String StrBuf;

    StrBuf="ipANT";

    String StringHostname = StrBuf+"-"+String(YOUR_CALL);
    char copy[13];
    StringHostname.toCharArray(copy, 13);

    ArduinoOTA.setHostname(copy);
    ArduinoOTA.setPassword("remoteqth");
    // $ echo password | md5sum
    // ArduinoOTA.setPasswordHash("5587ba7a03b12a409ee5830cea97e079");
    ArduinoOTA
      .onStart([]() {
        esp_task_wdt_reset();
        WdtTimer=millis();

        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
        #if defined(TFTLCD)
          tft.fillScreen(ILI9341_ORANGE);
          tft.setRotation(TftRotation);
          tft.setTextColor(ILI9341_WHITE);
          tft.setCursor(155,30);
          tft.setTextSize(6);
          tft.println("!");
          tft.setCursor(70,100);
          tft.setTextSize(3);
          tft.println("OTA update");
          tft.drawRect(57, 150, 206, 16, ILI9341_WHITE);
          tft.setCursor(100,180);
          tft.setTextSize(1);
          tft.println("RemoteQTH.com firmware");
        #endif
        TelnetServerClients[0].stop();
      })
      .onEnd([]() {
        Serial.println("\nEnd");
        #if defined(TFTLCD)
          // Clear Screen
          tft.fillScreen(ILI9341_BLACK);
          tft.setRotation(TftRotation);
        #endif
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        #if defined(TFTLCD)
          tft.fillRect(60, 153, (progress / (total / 100))*2, 10, ILI9341_WHITE);
        #endif
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
      });

    ArduinoOTA.begin();
  #endif

  TelnetServer.begin(TelnetServerIPport);
  // TelnetlServer.setNoDelay(true);

  // WDT
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  WdtTimer=millis();

  //init and get the time
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

   pinMode(ShiftOutLatchPin, OUTPUT);
   pinMode(ShiftOutClockPin, OUTPUT);
   pinMode(ShiftOutDataPin, OUTPUT);
   digitalWrite(ShiftOutLatchPin, LOW);
   shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, B10000000);
   shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, B00000000);
   shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, B00000000);
   shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, B00000000);
   shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, B00000000);
   digitalWrite(ShiftOutLatchPin, HIGH);
   EnableSerialDebug = 0;
}

//-------------------------------------------------------------------------------------------------------

void loop() {
  // MQTT
  httpWall();
  Mqtt();
  if (trxNetEnabled) net.loop();
  Telnet();
  CLI();
  Watchdog();
  // LcdDisplay(18,4,12);  //YheaderSize, Xbutons, YfooterSize
  Lcd();  //YheaderSize, Xbutons, YfooterSize

  // RX_UDP();
  #if defined(EnableOTA)
   ArduinoOTA.handle();
  #endif
}
// SUBROUTINES -------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------

void Lcd() { // 320x240 px
  #if defined(TFTLCD)
  // display show ------------------------------------------------------------------
  /*
  // bit 7 header
  // bit 6 body content
  // bit 5 menu
  bit 2 freq2
  bit 1 freq1
  bit 0 footer
  */
  if(LcdNeedRefresh > B00000000){
    // freq1
    if(bitRead(LcdNeedRefresh, 1)==1){
      tft.fillRect(0, 0, 320, 18, ILI9341_BLACK);
      // tft.setTextColor(ConvertRGB(51,153,255));
      if(TRXfreq[0]==0){
        tft.setTextColor(ILI9341_DARKGREY);
      }else{
        tft.setTextColor(ConvertRGB(102,178,255));
      }
      tft.setCursor(6,2);
      tft.setTextSize(2);

      int MHZ = TRXfreq[0]/1000000;
      if(MHZ<100 && MHZ>9){
        tft.print(" ");
      }else if(MHZ<10){
        tft.print("  ");
      }
      tft.print(MHZ);
      tft.print(".");
      int KHZ = TRXfreq[0]/1000-(MHZ*1000);
      if(KHZ<100 && KHZ>9){
        tft.print("0");
      }else if(KHZ<10){
        tft.print("00");
      }
      tft.print(KHZ);
      tft.print(" kHz ");
      tft.setTextColor(ILI9341_LIGHTGREY);
      if(TRXselectANT[0]<16){
        if(TRXselectANT[0]+1<10){
          tft.print(" ");
        }
        tft.print(TRXselectANT[0]+1);
        tft.print("-"+ANTname[TRXselectANT[0]]);
      }else{
        tft.print("off");
      }

      // POOL
      tft.fillRect(0, 19, 320, 18, ILI9341_BLACK);
      tft.setCursor(6,21);
      tft.setTextSize(2);
      for (int counter=0; counter<16; counter++) {
        if(AvailableANTpool[counter][0]!=0){
          if(TXANT[AvailableANTpool[counter][0]-1]==1){
            tft.setTextColor(ConvertRGB(150,0,0));
          }else{
            tft.setTextColor(ILI9341_DARKGREEN);
          }
          tft.print(AvailableANTpool[counter][0]);
          tft.print("-"+ANTname[AvailableANTpool[counter][0]-1]+" ");
        }
      }

      // bitSet(LcdNeedRefresh, 7);
      bitClear(LcdNeedRefresh, 1);
    }


    // freq2
    if(bitRead(LcdNeedRefresh, 2)==1){
      tft.drawLine(0,107,340,107, ILI9341_DARKGREY);
      tft.fillRect(0, 110, 320, 18, ILI9341_BLACK);
      if(TRXfreq[1]==0){
        tft.setTextColor(ILI9341_DARKGREY);
      }else{
        tft.setTextColor(ConvertRGB(102,178,255));
      }
      tft.setCursor(6,112);
      tft.setTextSize(2);

      int MHZ = TRXfreq[1]/1000000;
      if(MHZ<100 && MHZ>9){
        tft.print(" ");
      }else if(MHZ<10){
        tft.print("  ");
      }
      tft.print(MHZ);
      tft.print(".");
      int KHZ = TRXfreq[1]/1000-(MHZ*1000);
      if(KHZ<100 && KHZ>9){
        tft.print("0");
      }else if(KHZ<10){
        tft.print("00");
      }
      tft.print(KHZ);
      tft.print(" kHz ");
      tft.setTextColor(ILI9341_LIGHTGREY);
      if(TRXselectANT[1]<16){
        if(TRXselectANT[0]+1<10){
          tft.print(" ");
        }
        tft.print(TRXselectANT[1]+1);
        tft.print("-"+ANTname[TRXselectANT[1]]);
      }else{
        tft.print("off");
      }

      // POOL
      tft.fillRect(0, 129, 320, 18, ILI9341_BLACK);
      tft.setCursor(6,131);
      tft.setTextSize(2);
      for (int counter=0; counter<16; counter++) {
        if(AvailableANTpool[counter][1]!=0){
          if(TXANT[AvailableANTpool[counter][0]-1]==0){
            tft.setTextColor(ConvertRGB(150,0,0));
          }else{
            tft.setTextColor(ILI9341_DARKGREEN);
          }
          tft.print(AvailableANTpool[counter][1]);
          tft.print("-"+ANTname[AvailableANTpool[counter][1]-1]+" ");
        }
      }

      bitClear(LcdNeedRefresh, 2);
    }

    // footer status
    if(bitRead(LcdNeedRefresh, 0)==1){
      tft.fillRect(0, 240-12, 320, 12, ILI9341_BLACK);
      tft.drawLine(0,240-12,340,240-12, ILI9341_LIGHTGREY);
      tft.setTextColor(ILI9341_LIGHTGREY);
      tft.setTextSize(1);
      tft.setCursor(0,240-12+3);
      if(eth_connected==true){
        tft.print(String(ETH.localIP()[0])+"."+String(ETH.localIP()[1])+"."+String(ETH.localIP()[2])+"."+String(ETH.localIP()[3])+" ");
      }else{
        tft.print("ETH-OFF ");
      }

      char charbuf[50];
      ETH.macAddress().toCharArray(charbuf, 18);
      // charbuf[6] = 0;
      #if defined(MQTT)
        if(mqttClient.connected()==true){
          if (mqttClient.connect(charbuf)) {
            tft.print("mqtt ");
            // tft.setTextColor(ILI9341_WHITE);
          }else{
            tft.print("mqtt-OFF ");
          //   tft.setTextColor(ILI9341_LIGHTGREY);
          }
        }
      #endif
      tft.setCursor(235,240-12+3);
      tft.print("ip"+String(hardware)+" ");
      tft.setCursor(270,240-12+3);
      tft.print(String(REV));
      // bitSet(LcdNeedRefresh, 0);
      bitClear(LcdNeedRefresh, 0);
    }

  }
  #endif
}
//-------------------------------------------------------------------------------------------------------
void LcdDisplay(int YheaderSize, int Xbutons, int YfooterSize) { // 320x240 px
  #if defined(TFTLCD)
  unsigned int Frame=6;
  unsigned int XSize=320/Xbutons;
  unsigned int YSize=209/Ybutons;

  // unsigned int DeactRxonlyTx[Outputs]={2,2,2,2,2,2,2,2, 1,1,1,1,0,0,0,0};
  // bool SelectTXbutton[Outputs][2];
  // bool PTT[TRX];
  // unsigned int Groups[Outputs]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

  // https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
  // TEST |= 1UL << i; // SetBit
  // bit = (TEST >> i) & 1U; // CheckBit
  // TEST &= ~(1UL << i);  // ClearBit

  // LCD rx from AntSw [mqtt]

  /*

co by mel ant sw poslat do LCD
-



BD send frequency to AntSw




  https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
  https://cdn-learn.adafruit.com/downloads/pdf/adafruit-gfx-graphics-library.pdf

  informace ke kazdemu vystupu 16 x 4

  staticke
  - band(s) - muze byt aktivni na vice pasmech 10+, 0=nepouzito
  - RX only - neni urcen pro TX vyber
  - ANTname (8 znaku?)
  - groups

  dynamicke
  - TX used - 4 TRX
  - client PTT on/off spotvrzovanim prepnuti mezi rx/tx nastavenim




  */

  // TOUCH ------------------------------------------------------------------
  static long TouchTimer;
  static long TouchTimerDelay;
  static int TouchResult;
  static bool TXbutton;
  int Xcursor;
  int Ycursor;
  static bool ActivePage=false;
  if(millis()-TouchTimer > TouchTimerDelay){
    TS_Point p;
    p = ts.getPoint();
    if (p.z == 128){  // if (p.z != 129){
      if(TftRotation==3){
        Xcursor = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
        Ycursor = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());
      }else if(TftRotation==1){
        Xcursor = map(p.y, TS_MINX, TS_MAXX, tft.width(), 0);
        Ycursor = map(p.x, TS_MINY, TS_MAXY, tft.height(), 0);
      }
      // tft.fillCircle(TouchXY[0], TouchXY[1], 5, ILI9341_YELLOW);
      TouchResult=0;
      if(Ycursor < YheaderSize+YSize*(Ybutons-1)){
        // Touch buttons
        for (int j = 0; j < Ybutons-1; j++) {
          for (int i = 0; i < Xbutons; i++) {
            if( (XSize*i)<Xcursor && Xcursor<(XSize*(i+1)) && (YheaderSize+YSize*j)<Ycursor && Ycursor<(YheaderSize+YSize*(j+1)) ) {
              TouchResult=i+j*Xbutons;
              break;
            }
          }
        }
      }else{
        // Touch menu
        for (int i = 0; i < Xbutons; i++) {
          if( (XSize*i)<Xcursor && Xcursor<(XSize*(i+1)) ) {
            TouchResult=-(i+1);
            break;
          }
        }
      }

      if(EnableSerialDebug==true){
        Prn(1,"Touch "+String(Xcursor)+"px|"+String(Ycursor)+"px|"+String(millis()/1000)+"s|"+String(TouchResult) );
        tft.fillCircle(Xcursor, Ycursor, 5, ILI9341_YELLOW);
      }

      switch (TouchResult){
        case -1: // PAGE
          // switch 8/16 buttons
          if(BodyContent==0){
            if(ANT>8){
              ActivePage=!ActivePage;
            }
          }else if(BodyContent>=0){
            if(Ybutons==3){
              Ybutons=5;
            }else if(Ybutons==5){
              Ybutons=3;
            }
            // BodyContent=0;
          }
          bitSet(LcdNeedRefresh, 6);  // body
          bitSet(LcdNeedRefresh, 5);  // menu
          break;
        case -2: // SET
          BodyContent++;
          if(BodyContent>1){
            BodyContent=0;
          }
          bitSet(LcdNeedRefresh, 6);  // body
          bitSet(LcdNeedRefresh, 5);  // menu
          break;
        case -3: // TRXselect
          TRXselect++;
          if(TRXselect+1 > TRX){
            TRXselect=0;
          }
          EEPROM.writeByte(35, TRXselect);
          EEPROM.commit();
          bitSet(LcdNeedRefresh, 5); // menu
          bitSet(LcdNeedRefresh, 0); // footer
          break;
        case -4: // RX/TX
          if(BodyContent==0){
            TXbutton=!TXbutton;
            bitSet(LcdNeedRefresh, 6);  // body
            bitSet(LcdNeedRefresh, 5); // menu
          }
          break;
      }
      TouchTimer=millis();
      TouchTimerDelay=1000;
    }else{
      TouchTimer=millis();
      TouchTimerDelay=200;
    }
  }

  XSize=320/Xbutons;
  YSize=209/Ybutons;
  // display show ------------------------------------------------------------------
  /*
  bit 7 header
  bit 6 body content
  bit 5 menu
  bit 0 footer
  */
  if(LcdNeedRefresh > B00000000){
    // Show header
    if(bitRead(LcdNeedRefresh, 7)==1){
      tft.fillRect(0, 0, 320, YheaderSize, ILI9341_BLACK);
      // tft.setTextColor(ConvertRGB(51,153,255));
      tft.setTextColor(ConvertRGB(102,178,255));
      tft.setCursor(Frame,2);
      tft.setTextSize(2);
      tft.print("14.123 kHz");
      // bitSet(LcdNeedRefresh, 7);
      bitClear(LcdNeedRefresh, 7);
    }

    // Show body
    if(bitRead(LcdNeedRefresh, 6)==1){
      tft.fillRect(0, YheaderSize+1, 320, YSize*(Ybutons-1), ILI9341_BLACK);

      for (int i = 0; i < Xbutons; i++) {
        switch (BodyContent){
          // ANT buttons content
          case 0:
            for (int j = 0; j < Ybutons-1; j++) {
              for (int i = 0; i < Xbutons; i++) {
                  static int PageShift;
                  if(ActivePage==false){
                    PageShift = 0;
                  }else{
                    PageShift = 8;
                  }
                  // fill button
                if(ANTname[i+j*Xbutons+PageShift]=="n/a"){
                  // nil
                }else{
                  // QRVbands[16];  // 16bit/band for each ant
                  if(TXbutton==true && RXonly[i+j*Xbutons+PageShift]==true){
                    tft.fillRoundRect(XSize*i+Frame, YheaderSize+YSize*j+Frame, XSize-2*Frame, YSize-2*Frame, 2*Frame, ILI9341_DARKGREY);
                    tft.setTextColor(ILI9341_LIGHTGREY);
                  }else{
                    tft.fillRoundRect(XSize*i+Frame, YheaderSize+YSize*j+Frame, XSize-2*Frame, YSize-2*Frame, 2*Frame, ILI9341_LIGHTGREY);
                    tft.setTextColor(ILI9341_WHITE);
                  }
                  tft.setTextSize(1);
                  tft.setCursor(XSize*i+Frame*3, YheaderSize+YSize*j+2.5*Frame);
                  // name
                  tft.print(ANTname[i+j*Xbutons+PageShift]);
                  if(Ybutons==3){
                    // group
                    tft.setTextSize(1);
                    tft.setCursor(XSize*i+XSize-Frame*4, YheaderSize+YSize*j+7.5*Frame);
                    tft.print(String(Groups[i+j*Xbutons+PageShift]));
                    // number
                    tft.setTextSize(2);
                    tft.setTextColor(ILI9341_BLACK);
                    tft.setCursor(XSize*i+Frame*3, YheaderSize+YSize*j+6*Frame);
                    tft.print(String(i+j*Xbutons+PageShift+1));
                  }
                }
              }
            }
            break;
          // SETTINGS content
          case 1:
            tft.setTextColor(ILI9341_WHITE);
            tft.setTextSize(1);
            tft.setCursor(80, 22);
            tft.print("CALLSIGN "+YOUR_CALL );
            tft.setCursor(80, 22+12);
            #if defined(MQTT)
              tft.print("MQTT broker "+String(mqtt_server_ip[0])+"."+String(mqtt_server_ip[1])+"."+String(mqtt_server_ip[2])+"."+String(mqtt_server_ip[3])+":"+String(MQTT_PORT));
            #endif
            tft.setCursor(80, 22+2*12);
            tft.print("ANT "+String(ANT)+" | TRX "+String(TRX)+" | LCD rotation "+String(TftRotation));
            tft.setCursor(80, 22+3*12);
            #if defined(MQTT)
              tft.print("Latency "+String(LatencyTimer/2)+"ms (half path)" );
            #endif
            break;
          }
      }
      // bitSet(LcdNeedRefresh, 6);
      bitClear(LcdNeedRefresh, 6);
    }

    // Show menu Buttons
    if(bitRead(LcdNeedRefresh, 5)==1){
      tft.fillRect(0, YheaderSize+(Ybutons-1)*YSize, 320, YSize, ILI9341_BLACK);
      for (int i = 0; i < Xbutons; i++) {
        switch (i){
          case 0:
            if(BodyContent==0){
              tft.drawRoundRect(XSize*i+Frame, YheaderSize+YSize*(Ybutons-1)+Frame, XSize-2*Frame, YSize-2*Frame, 2*Frame, ILI9341_DARKGREY);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.setCursor(XSize*i+Frame*3, YheaderSize+YSize*(Ybutons-1)+2.5*Frame);
              tft.print("PAGE");
              if(Ybutons==3){
                tft.setTextSize(2);
                tft.setCursor(XSize*i+XSize/2-10, YheaderSize+YSize*(Ybutons-1)+YSize/2);
                if(ActivePage==false){
                  tft.print("1");
                }else{
                  tft.print("2");
                }
                tft.setTextSize(1);
                if(ANT>8){
                  tft.print("2");
                }else{
                  tft.print("1");
                }
              }
            }
            if(BodyContent==1){
              tft.drawRoundRect(XSize*i+Frame, YheaderSize+YSize*(Ybutons-1)+Frame, XSize-2*Frame, YSize-2*Frame, 2*Frame, ILI9341_DARKGREY);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.setCursor(XSize*i+Frame*3, YheaderSize+YSize*(Ybutons-1)+2.5*Frame);
              if(Ybutons==3){
                tft.print("4x4");
              }
              if(Ybutons==5){
                tft.print("4x2");
              }
            }
            break;
          case 1:
            tft.drawRoundRect(XSize*i+Frame, YheaderSize+YSize*(Ybutons-1)+Frame, XSize-2*Frame, YSize-2*Frame, 2*Frame, ILI9341_DARKGREY);
            tft.setTextColor(ILI9341_WHITE);
            tft.setTextSize(1);
            tft.setCursor(XSize*i+Frame*3, YheaderSize+YSize*(Ybutons-1)+2.5*Frame);
            if(BodyContent==0){
              tft.print("SET");
            }else{
              tft.print("BACK");
            }
            break;
          case 2:
            if(BodyContent>0){
              tft.drawRoundRect(XSize*i+Frame, YheaderSize+YSize*(Ybutons-1)+Frame, XSize-2*Frame, YSize-2*Frame, 2*Frame, ILI9341_BLUE);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.setCursor(XSize*i+Frame*3, YheaderSize+YSize*(Ybutons-1)+2.5*Frame);
              tft.print("TRX nr");
              tft.setTextColor(ILI9341_BLUE);
              tft.setTextSize(2);
              tft.setCursor(XSize*i+XSize/2-10, YheaderSize+YSize*(Ybutons-1)+YSize/2);
              tft.print(String(TRXselect+1));
            }
            break;
          case 3:
            if(BodyContent==0){
              if(TXbutton==false){
                tft.fillRoundRect(XSize*i+Frame, YheaderSize+YSize*(Ybutons-1)+Frame, XSize-2*Frame, YSize-2*Frame, 2*Frame, ILI9341_DARKGREEN);
              }else{
                tft.fillRoundRect(XSize*i+Frame, YheaderSize+YSize*(Ybutons-1)+Frame, XSize-2*Frame, YSize-2*Frame, 2*Frame, ILI9341_RED);
              }
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.setCursor(XSize*i+Frame*3, YheaderSize+YSize*(Ybutons-1)+2.5*Frame);
              tft.print("RX/TX");
            }
            break;
        }
      }
      // bitSet(LcdNeedRefresh, 5);
      bitClear(LcdNeedRefresh, 5);
    }

    // footer status
    if(bitRead(LcdNeedRefresh, 0)==1){
      tft.fillRect(0, 240-YfooterSize, 320, YfooterSize, ILI9341_BLACK);
      tft.drawLine(0,240-YfooterSize,340,240-YfooterSize, ILI9341_LIGHTGREY);
      tft.setTextColor(ILI9341_LIGHTGREY);
      tft.setTextSize(1);
      tft.setCursor(0,240-YfooterSize+3);
      tft.print(String(ETH.localIP()[0])+"."+String(ETH.localIP()[1])+"."+String(ETH.localIP()[2])+"."+String(ETH.localIP()[3])+" " );

      char charbuf[50];
      ETH.macAddress().toCharArray(charbuf, 18);
      // charbuf[6] = 0;
      #if defined(MQTT)
        if(mqttClient.connected()==true){
          if (mqttClient.connect(charbuf)) {
            tft.print("mqtt ");
            // tft.setTextColor(ILI9341_WHITE);
          // }else{
          //   tft.setTextColor(ILI9341_LIGHTGREY);
          }
        }
      #endif
      tft.setCursor(200,240-YfooterSize+3);
      tft.print("TRX-"+String(TRXselect+1)+" ");
      tft.setTextColor(ILI9341_LIGHTGREY);
      tft.print("ANT");
      tft.setCursor(270,240-YfooterSize+3);
      tft.print(String(REV));
      // bitSet(LcdNeedRefresh, 0);
      bitClear(LcdNeedRefresh, 0);
    }

  }
  #endif
}

word ConvertRGB( byte R, byte G, byte B)
{
  return ( ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3) );
}

//-------------------------------------------------------------------------------------------------------
// digitalWrite(ShiftOutLatchPin, LOW);  // když dáme latchPin na LOW mužeme do registru poslat data
// shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[2]);
// shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[1]);
// shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[0]);
// digitalWrite(ShiftOutLatchPin, HIGH);    // jakmile dáme latchPin na HIGH data se objeví na výstupu



//-------------------------------------------------------------------------------------------------------
void Watchdog(){

  // WDT
  if(millis()-WdtTimer > 60000){
    esp_task_wdt_reset();
    WdtTimer=millis();
    if(EnableSerialDebug>1){
      Prn(0,"WDT reset ");
      Prn(1, UtcTime(1));
    }
  }

  if(!TelnetServerClients[0].connected() && FirstListCommands==false){
    FirstListCommands=true;
  }

  #if defined(MQTT)
    if( (HeartBeatCfm==true && millis()-HeartBeatTimer > 2000) || (millis()-HeartBeatTimer > 20000) ){
        MqttPubString("HeartBeat", String(HeartBeatCounter), 0);
      HeartBeatTimer=millis();
      if(EnableSerialDebug>1){
        Prn(1, String("TX HeartBeat"));
      }
      HeartBeatCfm=false;
      HeartBeatCounter++;
    }
  #endif



}
//-------------------------------------------------------------------------------------------------------

void ListCommands(int OUT){

  #if defined(ETHERNET)
    Prn(1,"");
    Prn(1,"------  ip"+String(hardware)+" status  ------");
    Prn(0,"  http://");
    Prn(1, String(ETH.localIP()[0])+"."+String(ETH.localIP()[1])+"."+String(ETH.localIP()[2])+"."+String(ETH.localIP()[3]) );
    Prn(0,"  ETH: MAC ");
    Prn(0, String(ETH.macAddress()[0], HEX)+":"+String(ETH.macAddress()[1], HEX)+":"+String(ETH.macAddress()[2], HEX)+":"+String(ETH.macAddress()[3], HEX)+":"+String(ETH.macAddress()[4], HEX)+":"+String(ETH.macAddress()[5], HEX)+", " );
    Prn(0, String(ETH.linkSpeed()) );
    Prn(0,"Mbps");
    if (ETH.fullDuplex()) {
      Prn(0,", FULL_DUPLEX ");
    }
  #else
    Prn(0,"  ETHERNET OFF ");
  #endif

  Prn(0,"  NTP UTC:");
  Prn(1, UtcTime(1));
  Prn(0,"  Uptime: ");
  if(millis() < 60000){
    Prn(0, String(millis()/1000) );
    Prn(1," second");
  }else if(millis() > 60000 && millis() < 3600000){
    Prn(0, String(millis()/60000) );
    Prn(1," minutes");
  }else if(millis() > 3600000 && millis() < 86400000){
    Prn(0, String(millis()/3600000) );
    Prn(1," hours");
  }else{
    Prn(0, String(millis()/86400000) );
    Prn(1," days");
  }

  if(RebootWatchdog > 0){
    Prn(0,"> Reboot countdown in ");
    Prn(0, String(RebootWatchdog-((millis()-WatchdogTimer)/60000)) );
    Prn(1, " minutes");
  }
  if(OutputWatchdog > 0 && OutputWatchdog<123456){
    Prn(0,"> Clear output countdown in ");
    Prn(0, String(OutputWatchdog-((millis()-WatchdogTimer)/60000)) );
    Prn(1," minutes");
  }

  #if defined(MQTT)
    Prn(0,"  MqttSubscribe: "+String(mqtt_server_ip[0])+"."+String(mqtt_server_ip[1])+"."+String(mqtt_server_ip[2])+"."+String(mqtt_server_ip[3])+":"+String(MQTT_PORT)+"/");
    String topic = String(YOUR_CALL) + "/OI3/0/hz";
    const char *cstr = topic.c_str();
    if(mqttClient.subscribe(cstr)==true){
      Prn(1, String(cstr));
    }else{
      Prn(1, "FALSE");
    }
  #endif

  Prn(0,"  Firmware: ");
  Prn(1, String(REV));
  Prn(0,"  Hardware: ");
  Prn(1, String(hardware));
  Prn(0, "  ShiftOut GPIO [data, latch, clock]: ");
  Prn(1, String(ShiftOutDataPin)+", "+String(ShiftOutLatchPin)+", "+String(ShiftOutClockPin));
  Prn(1, "  TRX1 "+String(TRXselectANT[0]+1)+" | TRX2 "+String(TRXselectANT[1]+1));
  Prn(0, "  Bank status ABCD [LSBFIRST]: ");
  Prn(0, String(ShiftOutByte[0], BIN) );
  Prn(0, " ");
  Prn(0, String(ShiftOutByte[1], BIN) );
  Prn(0," ");
  Prn(0, String(ShiftOutByte[2], BIN) );
  Prn(0," ");
  Prn(1, String(ShiftOutByte[3], BIN) );
  Prn(1,"---------------------------------------------");

  Prn(1,"      ?  list status and commands");
  #if defined(TFTLCD)
  Prn(1,"      r  LCD rotation ["+String(TftRotation)+"]");
  #endif
  #if defined(MQTT)
    Prn(1,"      +  change MQTT broker IP | "+String(mqtt_server_ip[0])+"."+String(mqtt_server_ip[1])+"."+String(mqtt_server_ip[2])+"."+String(mqtt_server_ip[3])+":"+String(MQTT_PORT));
  #endif
  Prn(1,"      L  change CALLSIGN ["+YOUR_CALL+"]");
  Prn(1,"");
//   Prn(0,"      %  group buttons (select one from group) [");
//                        if(EnableGroupButton==true){Prn(1,"ON]");
// Prn(1,"         !  SET group buttons");
// Prn(1,"         :  list group buttons");
//                        }else{Prn(1,"OFF]");}
  Prn(0,"      w  inactivity reboot watchdog ");
                       if(RebootWatchdog>0){Prn(1,"after ["+String(RebootWatchdog)+"] minutes");
                       }else{Prn(1,"[disable]");}
  Prn(0,"      W  inactivity clear output watchdog ");
                       if(OutputWatchdog>0){Prn(1,"after ["+String(OutputWatchdog)+"] minutes");
                       }else{Prn(1,"[disable]");}

  if(TelnetServerClients[0].connected()){
    Prn(1,"      q  disconnect and close telnet");
    Prn(1,"      Q  logout and close telnet");
  }
  Prn(1,"      *  enable debug ["+String(EnableSerialDebug)+"]");
  Prn(1,"      E  erase whole eeprom");
  Prn(1,"      e  list EEPROM");
  Prn(1,"      @  restart device");
  Prn(1,"---------------------------------------------");
  Prn(1, "" );
}

//-------------------------------------------------------------------------------------------------------
void CLI(){
  int OUT=2;
  // incomingByte = 0;

  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    OUT = 0;
  }

  if(TelnetServerClients[0].connected() && OUT!=0){
    TelnetAuthorized = true;
    if(incomingByte!=0){
      OUT=1;
      if(FirstListCommands==true){
        FirstListCommands=false;
      }
    }
  }

  if(OUT<2){
    esp_task_wdt_reset();
    WdtTimer=millis();
    if(EnableSerialDebug>0){
      Prn(1, "DebugRx "+String(char(incomingByte))+"|"+String( (int)incomingByte ) );
    }




    // ?
    if(incomingByte==63){
      ListCommands(OUT);

    // *
    }else if(incomingByte==42){
      EnableSerialDebug++;
      if(EnableSerialDebug>2){
        EnableSerialDebug=0;
      }
      Prn(0,"** Serial DEBUG ");
      if(EnableSerialDebug==0){
        Prn(1,"DISABLE **");
      }else if(EnableSerialDebug==1){
        Prn(1,"ENABLE **");
      }else if(EnableSerialDebug==2){
        Prn(1,"ENABLE with frenetic mode **");
      }

    // r
    }else if(incomingByte==114){
      #if defined(TFTLCD)
        if(TftRotation==1){
          TftRotation=3;
        }else{
          TftRotation=1;
        }
        EEPROM.writeByte(0, TftRotation);
        EEPROM.commit();
        tft.setRotation(TftRotation);
        LcdNeedRefresh = B11111111;
        Prn(1,"Rotation set to "+String(TftRotation));
      #endif

    #if defined(MQTT)
    // +
    }else if(incomingByte==43){
      Prn(1,"  enter MQTT broker IP address by four number (0-255) and press [enter] after each");
      Prn(1,"  NOTE: public remoteqth broker 54.38.157.134:1883");
      for (int i=0; i<5; i++){
        if(i==4){
          Prn(0,"enter IP port (1-65535) and press [");
        }
        if(TelnetAuthorized==true){
          Prn(1,"enter]");
        }else{
          Prn(1,";]");
        }
        Enter();
        int intBuf=0;
        int mult=1;
        for (int j=InputByte[0]; j>0; j--){  // [0] is counter
          intBuf = intBuf + ((InputByte[j]-48)*mult);
          mult = mult*10;
        }
        if( (i<4 && intBuf>=0 && intBuf<=255) || (i==4 && intBuf>=1 && intBuf<=65535) ){
          if(i==4){
            EEPROM.writeInt(165, intBuf);
          }else{
            EEPROM.writeByte(161+i, intBuf);
          }
          // Prn(1,"EEPROMcomit");
          EEPROM.commit();
        }else{
          Prn(1,"Out of range.");
          break;
        }
      }
      Prn(0,"** device will be restarted **");
      delay(1000);
      TelnetServerClients[0].stop();
      ESP.restart();
    #endif

    // L
    }else if(incomingByte==76){
        Prn(0,"  Input new CALLSIGN (-ssid) and press [");
        if(TelnetAuthorized==true){
          Prn(0,"enter");
        }else{
          Prn(0,";");
        }
        Prn(1,"]. If blank, will be use MAC");
        Enter();
        YOUR_CALL="";
        for (int i=1; i<21; i++){
          YOUR_CALL=YOUR_CALL+char(InputByte[i]);
          if(i<InputByte[0]+1){  // [0] is counter
            EEPROM.write(140+i, InputByte[i]);
          }else{
            EEPROM.write(140+i, 0xff);
          }
        }
        EEPROM.commit();
        Prn(1,"** Device will be restarted **");
        delay(1000);
        TelnetServerClients[0].stop();
        ESP.restart();

      // w
      }else if(incomingByte==119){
        Prn(1,"Write reboot watchdog in minutes (0-10080), 0-disable");
        Prn(1,"recomended 1440 (1 day)");
        EnterInt(OUT);
        if(CompareInt>=0 && CompareInt<=10080){
          RebootWatchdog = CompareInt;
          EEPROM.writeUInt(26, RebootWatchdog);
          EEPROM.commit();
          Prn(0," Set ");
          Prn(0, String(EEPROM.readUInt(26)) );
          Prn(1," minutes");
        }else{
          Prn(0,"Out of range.");
        }

      // W
      }else if(incomingByte==87){
        Prn(1,"Write clear output watchdog in minutes (0-10080), 0-disable");
        Prn(1,"note: if you need clear output after reboot watchdog, set smaller than it");
        EnterInt(OUT);
        if(CompareInt>=0 && CompareInt<=10080){
          OutputWatchdog = CompareInt;
          EEPROM.writeUInt(30, OutputWatchdog);
          EEPROM.commit();
          Prn(0," Set ");
          Prn(0, String(EEPROM.readUInt(30)) );
          Prn(1," minutes");
        }else{
          Prn(0,"Out of range.");
        }

      // q
      }else if(incomingByte==113 && TelnetServerClients[0].connected() ){
        TelnetServerClients[0].stop();
        TelnetAuthorized=false;
        FirstListCommands=true;

      // Q
      }else if(incomingByte==81 && TelnetServerClients[0].connected() ){
        TelnetServerClients[0].stop();
        TelnetAuthorized=false;
        FirstListCommands=true;

      // E
      }else if(incomingByte==69){
          Prn(1,"  Erase whole eeprom (also telnet key)? (y/n)");
          EnterChar(OUT);
          if(incomingByte==89 || incomingByte==121){
            Prn(1,"  Stop erase? (y/n)");
            EnterChar(OUT);
            if(incomingByte==78 || incomingByte==110){
              for(int i=0; i<EEPROM_SIZE; i++){
                EEPROM.write(i, 0xff);
                Prn(0,".");
              }
              EEPROM.commit();
              Prn(1,"");
              Prn(1,"  Eeprom erased done");
              Prn(0,"** device will be restarted **");
              delay(1000);
              TelnetServerClients[0].stop();
              ESP.restart();
            }else{
              Prn(1,"  Erase aborted");
            }
          }else{
            Prn(1,"  Erase aborted");
          }

      // e
      }else if(incomingByte==101){
          Prn(1,"List EEPROM");
          for (int i=0; i<EEPROM_SIZE; i++){
            Prn(0, String(i));
            Prn(0, ">" );
            Prn(0, String(EEPROM.read(i)) );
            Prn(0, " " );
          }
          Prn(1, "" );

      // @
      }else if(incomingByte==64){
        Prn(1,"** IP switch will be restarted **");
        TelnetServerClients[0].stop();
        ESP.restart();

    // anykey
    }else{
      // if(EnableSerialDebug>0){
      //   Prn(0," [");
      //   Prn(0, String(incomingByte) ); //, DEC);
      //   Prn(1,"] unknown command");
      // }
    }
    incomingByte=0;
  }
}

//-------------------------------------------------------------------------------------------------------
void Enter(){
  int OUT;
  if(TelnetAuthorized==true){
    OUT=1;
  }else{
    OUT=0;
  }
  // clear
  for(int i=0; i<21; i++){
    InputByte[i] = 0;  // [0] is counter
  }
  incomingByte = 0;
  bool br = false;
  Prn(0,"> ");

  if(OUT==0){
    while(br==false) {
      if(Serial.available()){
        incomingByte=Serial.read();
        if(incomingByte==13 || incomingByte==59){ // CR or ;
          br=true;
          Prn(1,"");
        }else{
          Serial.write(incomingByte);
          if(incomingByte!=10 && incomingByte!=13){
            if(incomingByte==127){
              InputByte[0]--;  // [0] is counter
            }else{
              InputByte[InputByte[0]+1]=incomingByte;
              InputByte[0]++;  // [0] is counter
            }
          }
        }
        if(InputByte[0]==20){  // [0] is counter
          br=true;
          Prn(1," too long");
        }
      }
    }

  }else if(OUT==1){
    if (TelnetServerClients[0] && TelnetServerClients[0].connected()){

        while(br==false){
          if(TelnetServerClients[0].available()){
            incomingByte=TelnetServerClients[0].read();
            if( (incomingByte==10 && InputByte[0]<1) || (incomingByte==13 && InputByte[0]<1) ){  // LF CR
              // nil
            }else if( (incomingByte==10 && InputByte[0]>0) || (incomingByte==13 && InputByte[0]>0) || incomingByte==46 || incomingByte==58){  // LF CR . :
              br=true;
              Prn(1, String(char(incomingByte)));
            }else{
              TelnetServerClients[0].write(incomingByte);
              if(incomingByte==127){
                InputByte[0]--;  // [0] is counter
              }else{
                InputByte[InputByte[0]+1]=incomingByte;
                InputByte[0]++;  // [0] is counter
              }
            }
            if(InputByte[0]==20){  // [0] is counter
              br=true;
              Prn(1," too long");
            }
          }
        }

        // while(br==false){
        //   if(TelnetServerClients[0].available()){
        //     incomingByte=TelnetServerClients[0].read();
        //     if(incomingByte==10 || incomingByte==13 || incomingByte==46 || incomingByte==58){  // LF CR . :
        //       br=true;
        //       Prn(1, "");
        //     }else{
        //       TelnetServerClients[0].write(incomingByte);
        //       if(incomingByte!=10 && incomingByte!=13){
        //         if(incomingByte==127){
        //           InputByte[0]--;
        //         }else{
        //           InputByte[InputByte[0]+1]=incomingByte;
        //           InputByte[0]++;
        //         }
        //       }
        //     }
        //     if(InputByte[0]==20){
        //       br=true;
        //       Prn(1," too long");
        //     }
        //   }
        // }
        //



    }
  }

  // Serial.println();
  // for (int i=1; i<InputByte[0]+1; i++){
    // Serial.write(InputByte[i]);
  // }
  // Serial.println();

  // Prn(1, "out"+String(CompareInt) );
}

//-------------------------------------------------------------------------------------------------------
void EnterChar(int OUT){
  incomingByte = 0;
  Prn(0,">");
  if(OUT==0){
    while (Serial.available() == 0) {
      // Wait
    }
    incomingByte = Serial.read();
  }else if(OUT==1){
    if (TelnetServerClients[0] && TelnetServerClients[0].connected()){
      while(incomingByte==0){
        if(TelnetServerClients[0].available()){
          incomingByte=TelnetServerClients[0].read();
        }
      }
      if(EnableSerialDebug>0){
        Serial.println();
        Serial.print("Telnet rx-");
        Serial.print(incomingByte, DEC);
        Prn(1, "DebugRx "+String(char(incomingByte)) );
      }
    }
  }
  Prn(1, String(char(incomingByte)) );
}

//-------------------------------------------------------------------------------------------------------

void EnterInt(int OUT){
  incomingByte = 0;
  Prn(0,"> ");
  if(OUT==0){
    while(!Serial.available()) {
    }
    delay(3000);
    CompareInt = Serial.parseInt();
  }else if(OUT==1){
    if (TelnetServerClients[0] && TelnetServerClients[0].connected()){
      bool br=true;
      int intField[10];
      int count=0;

      while(incomingByte==0 && br==true){
        if(TelnetServerClients[0].available()){
          incomingByte=TelnetServerClients[0].read();
          // out of 0-9
          if(incomingByte<48 || incomingByte>57){
            br=false;
            intField[count]=0;
            Prn(1,"");
          }else{
            intField[count]=incomingByte-48;
            Prn(0,String(intField[count]));
            count++;
            incomingByte=0;
          }
        }
      }

      count--;
      CompareInt=0;
      int i=1;
      while(count>-1){
        CompareInt=CompareInt+intField[count]*i;
        // Prn(1, String(intField[count])+"*"+String(i)+"="+String(CompareInt) );
        i=i*10;
        count--;
      }
    }
  }
  // Prn(1, "out"+String(CompareInt) );
}

//-------------------------------------------------------------------------------------------------------
void Prn(int LN, String STR){
  if(TelnetAuthorized==false){
    Serial.print(STR);
    if(LN==1){
      Serial.println();
    }
  }else{
    size_t len = STR.length()+1;
    // uint8_t sbuf[len];
    char sbuf[len];
    STR.toCharArray(sbuf, len);
    //push data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (TelnetServerClients[i] && TelnetServerClients[i].connected()){
        TelnetServerClients[i].write(sbuf, len);
        // delay(1);
        if(LN==1){
          TelnetServerClients[i].write(13); // CR
          TelnetServerClients[i].write(10); // LF
        }
      }
    }
  }
}
//-------------------------------------------------------------------------------------------------------

void httpWall(){
  #if defined(MQTT)
  // listen for incoming clients
  WiFiClient webClient = server.available();
  if (webClient) {
    if(EnableSerialDebug>0){
      Serial.println("WIFI New webClient");
    }
    memset(linebuf,0,sizeof(linebuf));
    charcount=0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (webClient.connected()) {
      if (webClient.available()) {
        char c = webClient.read();
        HTTP_req += c;
        // if(EnableSerialDebug>0){
        //   Serial.write(c);
        // }
        //read char by char HTTP request
        linebuf[charcount]=c;
        if (charcount<sizeof(linebuf)-1) charcount++;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header

          // send a standard http response header
          webClient.println(F("HTTP/1.1 200 OK"));
          webClient.println(F("Content-Type: text/html"));
          webClient.println(F("Connection: close"));  // the connection will be closed after completion of the response
          webClient.println();
          webClient.println(F("  <!DOCTYPE html>"));
          webClient.println(F("  <html>"));
          webClient.println(F("      <head>"));
          webClient.println(F("          <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/>"));
          webClient.println(F("          <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
          // webClient.println(F("          <meta http-equiv=\"refresh\" content=\"10\">"));
          webClient.println(F("          <link rel=\"stylesheet\" type=\"text/css\" href=\"https://remoteqth.com/mqtt-wall/style.css\">"));
          // TITLE
          webClient.print(F("           <title>"));
          webClient.print(hardware);
          webClient.print(F(" station "));
          webClient.print(YOUR_CALL);
          webClient.println(F("</title>"));
          // END TITLE
          webClient.println(F("          <link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"style/apple-touch-icon.png\">"));
          webClient.println(F("          <link rel=\"mask-icon\" href=\"style/safari-pinned-tab.svg\" color=\"#5bbad5\">"));
          webClient.println(F("          <link rel=\"icon\" type=\"image/png\" href=\"style/favicon-32x32.png\" sizes=\"32x32\">"));
          webClient.println(F("          <link rel=\"icon\" type=\"image/png\" href=\"style/favicon-16x16.png\" sizes=\"16x16\">"));
          webClient.println(F("          <link rel=\"manifest\" href=\"style/manifest.json\">"));
          webClient.println(F("          <link rel=\"shortcut icon\" href=\"style/favicon.ico\">"));
          webClient.println(F("          <meta name='apple-mobile-web-app-capable' content='yes'>"));
          webClient.println(F("          <meta name='mobile-web-app-capable' content='yes'>"));
          webClient.println(F("          <meta name=\"msapplication-config\" content=\"style/browserconfig.xml\">"));
          webClient.println(F("          <meta name=\"theme-color\" content=\"#ffffff\">"));
          webClient.println(F("          <script type=\"text/javascript\">"));
          webClient.println(F("          var config = {"));
          webClient.println(F("              server: {"));
          webClient.print(F("                  uri: \"ws://"));
          webClient.print(mqtt_server_ip[0]);
          webClient.print(F("."));
          webClient.print(mqtt_server_ip[1]);
          webClient.print(F("."));
          webClient.print(mqtt_server_ip[2]);
          webClient.print(F("."));
          webClient.print(mqtt_server_ip[3]);
          webClient.println(":1884/\",");
          webClient.println(F("              },"));
          // TOPIC
          webClient.print(F("              defaultTopic: \""));
          webClient.print(YOUR_CALL);
          webClient.print(F("/"));
          webClient.print(hardware);
          webClient.println(F("/#\","));
          // END TOPIC
          webClient.println(F("              showCounter: true,"));
          webClient.println(F("              alphabeticalSort: true,"));
          webClient.println(F("              qos: 0"));
          webClient.println(F("          };"));
          webClient.println(F("          </script>"));
          // END TOPIC
          webClient.println(F("      </head>"));
          webClient.println(F("      <body>"));
          webClient.print(F("          <div id=\"frame\" "));
          webClient.println(F(">"));
          webClient.println(F("              <div id=\"footer\">"));
          webClient.println(F("                  <p class=\"status\" style=\"font-size: 150%;\">"));
          // STATUS
          webClient.print(F("Uptime: "));
          if(millis() < 60000){
            webClient.print(millis()/1000);
            webClient.print(F(" seconds"));
          }else if(millis() > 60000 && millis() < 3600000){
            webClient.print(millis()/60000);
            webClient.print(F(" minutes"));
          }else if(millis() > 3600000 && millis() < 86400000){
            webClient.print(millis()/3600000);
            webClient.print(F(" hours"));
          }else{
            webClient.print(millis()/86400000);
            webClient.print(F(" days"));
          }
          webClient.print(F(" | version: "));
          webClient.println(REV);
          webClient.print(F(" | eth mac: "));
          for (int i = 0; i < 6; i++) {
            webClient.print(ETH.macAddress()[i], HEX);
            webClient.print(F(":"));
          }
          webClient.println();

          webClient.print(F(" | dhcp: "));
          if(DHCP_ENABLE==1){
            webClient.print(F("ON"));
          }else{
            webClient.print(F("OFF"));
          }
          webClient.print(F(" | ip: "));
          webClient.println(ETH.localIP());
          // webClient.print(F(" | utc from ntp: "));
          // webClient.println(F("timeClient.getFormattedTime()"));
          // webClient.println(F("<br>MQTT subscribe command: $ mosquitto_sub -v -h mqttstage.prusa -t prusa-debug/prusafil/extrusionline/+/#"));
          webClient.print(F(" | Broker ip: "));
          webClient.print(mqtt_server_ip[0]);
          webClient.print(F("."));
          webClient.print(mqtt_server_ip[1]);
          webClient.print(F("."));
          webClient.print(mqtt_server_ip[2]);
          webClient.print(F("."));
          webClient.print(mqtt_server_ip[3]);
          webClient.print(F(":"));
          webClient.print(MQTT_PORT);
          // webClient.print(F(" | Refresh time "));
          // webClient.print(MeasureTimer[1]/60000);
          // webClient.println(F(" min"));
          // if(AprsON==true){
          //   webClient.print(F(" | <a href=\"https://aprs.fi/#!call="));
          //   webClient.print(YOUR_CALL);
          //   webClient.println(F("\" target=_blank>APRS</a>"));
          // }
          // END STATUS
          webClient.println(F("              </p>"));
          webClient.println(F("              </div>"));
          webClient.println(F("              <div id=\"header\">"));
          webClient.println(F("                  <div id=\"topic-box\">"));
          webClient.println(F("                      <input type=\"text\" id=\"topic\" value=\"\" title=\"Topic to subscribe\">"));
          webClient.println(F("                  </div>"));
          webClient.println(F("              </div>"));
          webClient.println(F("              <div id=\"toast\"></div>"));
          webClient.println(F("              <section class=\"messages\"></section>"));
          webClient.println(F("              <div id=\"footer\">"));
          webClient.println(F("                  <p class=\"status\">"));
          webClient.println(F("                      Client <code id=\"status-client\" title=\"Client ID\">?</code> is "));
          webClient.println(F("                      <code id=\"status-state\" class=\"connecting\"><em>&bull;</em> <span>connecting...</span></code> to "));
          webClient.println(F("                      <code id=\"status-host\">?</code>"));
          webClient.println(F("                      <em>via</em> MQTT Wall 0.3.0 (<a href=\"https://github.com/bastlirna/mqtt-wall\">github</a>)"));
          // webClient.println(F("                      | <a href=\"https://remoteqth.com/wiki/\" target=\"_blank\">WX Wiki</a>."));
          webClient.println(F("                  </p>"));
          webClient.println(F("              </div>"));
          webClient.println(F("          </div>"));
          webClient.println(F("          <script type=\"text/javascript\" src=\"https://code.jquery.com/jquery-2.1.4.min.js\"></script>"));
          webClient.println(F("          <script type=\"text/javascript\" src=\"https://code.jquery.com/color/jquery.color-2.1.2.min.js\"></script>"));
          webClient.println(F("          <script type=\"text/javascript\" src=\"https://cdnjs.cloudflare.com/ajax/libs/paho-mqtt/1.0.1/mqttws31.min.js\"></script>"));
          webClient.println(F("          <script type=\"text/javascript\" src=\"https://remoteqth.com/mqtt-wall/wall.js\"></script>"));
          webClient.println(F("      </body>"));
          webClient.println(F("  </html>"));

          if(EnableSerialDebug>0){
            Serial.print(HTTP_req);
          }
          HTTP_req = "";

          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          // if (strstr(linebuf,"GET /h0 ") > 0){digitalWrite(GPIOS[0], HIGH);}else if (strstr(linebuf,"GET /l0 ") > 0){digitalWrite(GPIOS[0], LOW);}
          // else if (strstr(linebuf,"GET /h1 ") > 0){digitalWrite(GPIOS[1], HIGH);}else if (strstr(linebuf,"GET /l1 ") > 0){digitalWrite(GPIOS[1], LOW);}

          // you're starting a new line
          currentLineIsBlank = true;
          memset(linebuf,0,sizeof(linebuf));
          charcount=0;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    webClient.stop();
   if(EnableSerialDebug>0){
     Serial.println("WIFI webClient disconnected");
     MeasureTimer[0]=millis()+5000-MeasureTimer[1];
   }
  }
  #endif
}
//-------------------------------------------------------------------------------------------------------

void EthEvent(WiFiEvent_t event)
{
  switch (event) {
    // case SYSTEM_EVENT_ETH_START:
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH  Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    // case SYSTEM_EVENT_ETH_CONNECTED:
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH  Connected");
      break;
    // case SYSTEM_EVENT_ETH_GOT_IP:
    case ARDUINO_EVENT_ETH_GOT_IP:
    #if defined(TFTLCD)
      bitSet(LcdNeedRefresh, 0);  // footer
    #endif
    Serial.println("===============================");
      Serial.print("    MAC: ");
      Serial.println(ETH.macAddress());
      Serial.print("   IPv4: ");
      Serial.println(ETH.localIP());
      Serial.println("===============================");
      if (ETH.fullDuplex()) {
        Serial.print("FULL_DUPLEX, ");
      }
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;

      { // TrxNet init / reconnect
        char trxnetDeviceName[TRXNET_MAX_DEVICE_NAME];
        snprintf(trxnetDeviceName, sizeof(trxnetDeviceName), "ANT.%s", trxnetAntId);
        net.setPort(trxnetPort);
        net.begin(trxnetDeviceName);
        net.setPriorityPrefixes(trxnetPrioPtr, sizeof(trxnetPrioPtr) / sizeof(trxnetPrioPtr[0]));
        net.subscribe("/hz", onTrxNetHz);
        trxNetEnabled = true;
        Prn(1, String("TrxNet begin ") + trxnetDeviceName
                + " prio " + trxnetPrio[0] + "," + trxnetPrio[1]);
      }

      #if defined(MQTT)
        if (MQTT_ENABLE == true && MQTT_LOGIN == true){
          // if (mqttClient.connect("esp32gwClient", MQTT_USER, MQTT_PASS)){
          //   AfterMQTTconnect();
          // }
        }else if(MQTT_ENABLE == true){

          mqttClient.setServer(mqtt_server_ip, MQTT_PORT);
          Prn(1, "EthEvent-MQTTclient ");
          mqttClient.setCallback(MqttRx);
          Prn(1, "EthEvent-MQTTcallback ");
          lastMqttReconnectAttempt = 0;

          char charbuf[50];
           // memcpy( charbuf, ETH.macAddress(), 6);
           ETH.macAddress().toCharArray(charbuf, 18);
           // charbuf[6] = 0;
          if (mqttClient.connect(charbuf)){
            Prn(0, "EthEvent-MQTTconnect ");
            Prn(1, String(charbuf));
            mqttReconnect();
            AfterMQTTconnect();
          }
        }
      #endif
      // ListCommands(0);

      // EnableSerialDebug=1;
      // EnableSerialDebug=0;
      break;

    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH  Disconnected");
      eth_connected = false;
      #if defined(TFTLCD)
        bitSet(LcdNeedRefresh, 0);
      #endif
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH  Stopped");
      eth_connected = false;
      #if defined(TFTLCD)
        bitSet(LcdNeedRefresh, 0);
      #endif
      break;
    default:
      break;
  }
}
//-------------------------------------------------------------------------------------------------------
void Mqtt(){
  #if defined(MQTT)
    if (millis()-MqttStatusTimer[0]>MqttStatusTimer[1] && MQTT_ENABLE == true && eth_connected==1){
      if(!mqttClient.connected()){
        long now = millis();
        if (now - lastMqttReconnectAttempt > 5000) {
          lastMqttReconnectAttempt = now;
          Serial.print("Attempt to MQTT reconnect | ");
          Serial.println(millis()/1000);
          if (mqttReconnect()) {
            lastMqttReconnectAttempt = 0;
          }
        }
      }else{
        // Client connected
        mqttClient.loop();
      }
      MqttStatusTimer[0]=millis();
    }
  #endif
}

//-------------------------------------------------------------------------------------------------------

#if defined(MQTT)
bool mqttReconnect() {
    // charbuf[6] = 0;
    char charbuf[50];
    // memcpy( charbuf, ETH.macAddress(), 6);
    ETH.macAddress().toCharArray(charbuf, 18);
    if (mqttClient.connect(charbuf)) {
      #if defined(TFTLCD)
        bitSet(LcdNeedRefresh, 0);  // footer
      #endif
      Prn(1, "mqttReconnect-connected");
      // IPAddress IPlocalAddr = ETH.localIP();                           // get
      // String IPlocalAddrString = String(IPlocalAddr[0]) + "." + String(IPlocalAddr[1]) + "." + String(IPlocalAddr[2]) + "." + String(IPlocalAddr[3]);   // to string
      // MqttPubStringQC(1, "IP", IPlocalAddrString, true);

      // resubscribe

      String topic = String(YOUR_CALL) + "/OI3/1/hz";
      topic.reserve(50);
      const char *cstr0 = topic.c_str();
      if(mqttClient.subscribe(cstr0)==true){
        if(EnableSerialDebug>0){
          Prn(0, "mqttReconnect-subscribe ");
          Prn(1, String(cstr0));
        }
      }
      topic = String(YOUR_CALL) + "/OI3/2/hz";
      topic.reserve(50);
      const char *cstr1 = topic.c_str();
      if(mqttClient.subscribe(cstr1)==true){
        if(EnableSerialDebug>0){
          Prn(0, "mqttReconnect-subscribe ");
          Prn(1, String(cstr1));
        }
      }

    }
    return mqttClient.connected();
}
#endif

//------------------------------------------------------------------------------------
void onTrxNetHz(const char* from, const uint8_t* data, size_t len) {
  if (len < 4) return;
  uint32_t freq;
  memcpy(&freq, data, 4);
  if (strcmp(from, trxnetTrx1Name) == 0) {
    TRXfreq[0] = freq;
    if (EnableSerialDebug > 0) { Prn(0, "TrxNet /hz TRX1 "); Prn(1, String(freq)); }
    SelectANT(0);
    #if defined(TFTLCD)
      bitSet(LcdNeedRefresh, 1);
    #endif
  } else if (strcmp(from, trxnetTrx2Name) == 0) {
    TRXfreq[1] = freq;
    if (EnableSerialDebug > 0) { Prn(0, "TrxNet /hz TRX2 "); Prn(1, String(freq)); }
    SelectANT(1);
    #if defined(TFTLCD)
      bitSet(LcdNeedRefresh, 2);
    #endif
  }
}

//------------------------------------------------------------------------------------
void MqttRx(char *topic, byte *payload, unsigned int length) {
  #if defined(MQTT)
    String CheckTopicBase;
    CheckTopicBase.reserve(100);
    byte* p = (byte*)malloc(length);
    memcpy(p,payload,length);
    static bool HeardBeatStatus;
    if(EnableSerialDebug>0){
      Prn(0, String("RX mqtt..."));
    }

        CheckTopicBase = String(YOUR_CALL) + "/OI3/1/hz";
        if ( CheckTopicBase.equals( String(topic) )){
          if(EnableSerialDebug>0){
            Prn(0, String("/OI3/1/hz "));
          }
          TRXfreq[0] = 0;
          unsigned long exp = 1;
          for (int i = length-1; i >=0 ; i--) {
            TRXfreq[0] = TRXfreq[0] + (p[i]-48)*exp;
            exp = exp*10;
          }
          if(EnableSerialDebug>0){
            Prn(1, String(TRXfreq[0]));
          }
          SelectANT(0);
          #if defined(TFTLCD)
            bitSet(LcdNeedRefresh, 1);
          #endif
        }

        CheckTopicBase = String(YOUR_CALL) + "/OI3/2/hz";
        if ( CheckTopicBase.equals( String(topic) )){
          if(EnableSerialDebug>0){
            Prn(0, String("/OI3/2/hz "));
          }
          TRXfreq[1] = 0;
          unsigned long exp = 1;
          for (int i = length-1; i >=0 ; i--) {
            TRXfreq[1] = TRXfreq[1] + (p[i]-48)*exp;
            exp = exp*10;
          }
          if(EnableSerialDebug>0){
            Prn(1, String(TRXfreq[1]));
          }
          SelectANT(1);
          #if defined(TFTLCD)
            bitSet(LcdNeedRefresh, 2);
          #endif
          // FreqToBandRules(freq);
        }

  #endif
} // MqttRx END

//-------------------------------------------------------------------------------------------------------
void AfterMQTTconnect(){
  #if defined(MQTT)
  //    if (mqttClient.connect("esp32gwClient", MQTT_USER, MQTT_PASS)) {          // public IP addres to MQTT
        IPAddress IPlocalAddr = ETH.localIP();                           // get
        String IPlocalAddrString = String(IPlocalAddr[0]) + "." + String(IPlocalAddr[1]) + "." + String(IPlocalAddr[2]) + "." + String(IPlocalAddr[3]);   // to string
        IPlocalAddrString.toCharArray( mqttTX, 50 );                          // to array
        String path2 = String(YOUR_CALL) + "/"+String(hardware)+"/ip";
        path2.toCharArray( mqttPath, 100 );
          mqttClient.publish(mqttPath, mqttTX, true);
            // Serial.print("MQTT-TX ");
            // Serial.print(mqttPath);
            // Serial.print(" ");
            // Serial.println(mqttTX);

        String MAClocalAddrString = ETH.macAddress();   // to string
        MAClocalAddrString.toCharArray( mqttTX, 50 );                          // to array
        path2 = String(YOUR_CALL) + "/"+String(hardware)+"/mac";
        path2.toCharArray( mqttPath, 100 );
          mqttClient.publish(mqttPath, mqttTX, true);
            // Serial.print("MQTT-TX ");
            // Serial.print(mqttPath);
            // Serial.print(" ");
            // Serial.println(mqttTX);

    // MeasureTimer[0]=2800000;
    MeasureTimer[0]=millis()-MeasureTimer[1];

  #endif
}
//-----------------------------------------------------------------------------------
void MqttPubString(String TOPIC, String DATA, bool RETAIN){
  #if defined(MQTT)
    char charbuf[50];
     // memcpy( charbuf, mac, 6);
     ETH.macAddress().toCharArray(charbuf, 18);
     // charbuf[6] = 0;
    // if(EnableEthernet==1 && MQTT_ENABLE==1 && EthLinkStatus==1 && mqttClient.connected()==true){
    if(mqttClient.connected()==true){
      if (mqttClient.connect(charbuf)) {
        String topic = String(YOUR_CALL) + "/"+String(hardware)+"/"+TOPIC;
        topic.toCharArray( mqttPath, 50 );
        DATA.toCharArray( mqttTX, 50 );
        mqttClient.publish(mqttPath, mqttTX, RETAIN);
      }
    }
  #endif
}
//-------------------------------------------------------------------------------------------------------
void Telnet(){
  uint8_t i;
  // if (wifiMulti.run() == WL_CONNECTED) {
  if (eth_connected==true) {

    //check if there are any new clients
    if (TelnetServer.hasClient()){
      for(i = 0; i < MAX_SRV_CLIENTS; i++){
        //find free/disconnected spot
        if (!TelnetServerClients[i] || !TelnetServerClients[i].connected()){
          if(TelnetServerClients[i]) TelnetServerClients[i].stop();
          TelnetServerClients[i] = TelnetServer.available();
          if (!TelnetServerClients[i]) Serial.println("Telnet available broken");
          if(EnableSerialDebug>0){
            Serial.println();
            Serial.print("New Telnet client: ");
            Serial.print(i); Serial.print(' ');
            Serial.println(TelnetServerClients[i].remoteIP());
          }
          break;
        }
      }
      if (i >= MAX_SRV_CLIENTS) {
        //no free/disconnected spot so reject
        TelnetServer.available().stop();
      }
    }

    //check clients for data
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (TelnetServerClients[i] && TelnetServerClients[i].connected()){
        if(TelnetServerClients[i].available()){
          //get data from the telnet client and push it to the UART
          // while(TelnetServerClients[i].available()) Serial_one.write(TelnetServerClients[i].read());
          if(EnableSerialDebug>0){
            Serial.println();
            Serial.print("TelnetRX ");
          }

          while(TelnetServerClients[i].available()){
            incomingByte=TelnetServerClients[i].read();
            // Serial_one.write(RX);
            if(EnableSerialDebug>0){
              // Serial.write(RX);
              Serial.print(char(incomingByte));
            }
          }
        }
      }else{
        if (TelnetServerClients[i]) {
          TelnetServerClients[i].stop();
          TelnetAuthorized=false;
          FirstListCommands=true;
          // TelnetServerClientAuth = {0,0,0,0};
        }
      }
    }

    //check UART for data
    // if(Serial_one.available()){
    //   size_t len = Serial_one.available();
    //   uint8_t sbuf[len];
    //   Serial_one.readBytes(sbuf, len);
    //   //push UART data to all connected telnet clients
    //   for(i = 0; i < MAX_SRV_CLIENTS; i++){
    //     if (TelnetServerClients[i] && TelnetServerClients[i].connected()){
    //       TelnetServerClients[i].write(sbuf, len);
    //       // delay(1);
    //       if(EnableSerialDebug>0){
    //         Serial.println();
    //         Serial.print("Telnet tx-");
    //         Serial.write(sbuf, len);
    //       }
    //     }
    //   }
    // }

  }else{
    // if(EnableSerialDebug>0){
    //   Serial.println("Telnet not connected!");
    // }
    for(i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (TelnetServerClients[i]) TelnetServerClients[i].stop();
    }
    delay(1000);
  }
}

//-------------------------------------------------------------------------------------------------------
String UtcTime(int format){
  tm timeinfo;
  char buf[50]; //50 chars should be enough
  if (eth_connected==false) {
    strcpy(buf, "n/a");
  }else{
    if(!getLocalTime(&timeinfo)){
      strcpy(buf, "n/a");
    }else{
      if(format==1){
        strftime(buf, sizeof(buf), "%Y-%b-%d %H:%M:%S", &timeinfo);
      }else if(format==2){
        strftime(buf, sizeof(buf), "%d", &timeinfo);
      }else if(format==3){
        strftime(buf, sizeof(buf), "%Y", &timeinfo);
      }
    }
  }
  // Serial.println(buf);
  return String(buf);
}

//-------------------------------------------------------------------------------------------------------

void SelectANT(int TRX){  // TRX 0 1
  TRXselectANT[TRX] = 42;
  int counter=0;
  for(int ant=0; ant<16; ant++){
    if(ANTrange[ant][0]<TRXfreq[TRX] && TRXfreq[TRX]<ANTrange[ant][1]){
      AvailableANTpool[counter][TRX]=ant+1;
      counter++;
      for(int NumberOfTrx=0; NumberOfTrx<2; NumberOfTrx++){
        if(NumberOfTrx!=TRX){
          if(TRXselectANT[NumberOfTrx]!=ant && TXANT[ant]==1){
            TRXselectANT[TRX]=ant;
            // Prn(1, "ant|NumberOfTrx|TRX|TRXselectANT[TRX] "+String(ant)+"|"+String(NumberOfTrx)+"|"+String(TRX)+"|"+String(TRXselectANT[TRX]));
          }
        }
      }
    }else{
      AvailableANTpool[counter][TRX]=0;
      counter++;
    }
  }
  ShiftOut();
  MqttPubString("TRX"+String(TRX+1)+"ant", String(TRXselectANT[TRX]+1), 0);
  MqttPubString("TRX"+String(TRX+1)+"antName", ANTname[TRXselectANT[TRX]], 0);
}

//-------------------------------------------------------------------------------------------------------

void ShiftOut(){
  for(int i=0; i<4; i++){
    ShiftOutByte[i]=0x00;
  }
  // TRX1
  if(TRXselectANT[0]<8){
    bitSet(ShiftOutByte[0], TRXselectANT[0]);
  }else if(TRXselectANT[0]>7 && TRXselectANT[0]<16){
    bitSet(ShiftOutByte[1], TRXselectANT[0]-8);
  }
  // TRX2
  if(TRXselectANT[1]<8){
    bitSet(ShiftOutByte[2], TRXselectANT[1]);
  }else if(TRXselectANT[1]>7 && TRXselectANT[1]<16){
    bitSet(ShiftOutByte[3], TRXselectANT[1]-8);
  }
  digitalWrite(ShiftOutLatchPin, LOW);
  shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[3]);
  shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[2]);
  shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[1]);
  shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[0]);
  digitalWrite(ShiftOutLatchPin, HIGH);
  if(EnableSerialDebug>0){
    Prn(1, "TRX1/2 "+String(TRXselectANT[0]+1)+"/"+String(TRXselectANT[1]+1)+"|ShiftOut 0123 "+String(ShiftOutByte[0], BIN)+"|"+String(ShiftOutByte[1], BIN)+"|"+String(ShiftOutByte[2], BIN)+"|"+String(ShiftOutByte[3], BIN));
  }

}

