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

Web
  http://<ip>/       antenna switch status + band x output matrix (AntMatrix module)
  http://<ip>/setup  network, callsign, TrxNet
Remote USB access
  screen /dev/ttyUSB0 115200

HARDWARE ESP32-POE

Changelog:
20221029 - initial version
20260925 - web config (AntMatrix band x output matrix per TRX, NVS), /setup page, MQTT removed

ToDo
- LCD https://squareline.io/ +

Použití knihovny BluetoothSerial ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/BluetoothSerial

Použití knihovny WiFi ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/WiFi
Použití knihovny EEPROM ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/EEPROM
Použití knihovny Ethernet ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/Ethernet
Použití knihovny ESPmDNS ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/ESPmDNS
Použití knihovny ArduinoOTA ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/ArduinoOTA
Použití knihovny Update ve verzi 2.0.0 v adresáři: /home/dan/Arduino/hardware/espressif/esp32/libraries/Update
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
const char* REV = "20260925";
char hardware[] = "ANT";
// const char* HWNAME = "IP-ROT";
int ANT = 8;
unsigned int TRX=1;
unsigned int TRXselect=0;

//--- ANT configure ------------------------------
// Band x output matrix, antenna names and selection logic live in the AntMatrix module
// (AntMatrix.h/.cpp, web + NVS in AntMatrixEsp.cpp), configured on http://<ip>/
// First-boot defaults reproduce the former hard-coded table, see AmHostDefaults().
#include "AntMatrix.h"
#include "AntMatrixWeb.h"
#include "SetupPage.h"
#include <Preferences.h>
#include <ArduinoJson.h>
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
161-168  - free (former MQTT broker IP/port)
169-509  - free (antenna config moved to NVS "antmx", host settings to NVS "anthub")

!! Increment EEPROM_SIZE #define !!

*/
unsigned int RebootWatchdog;
unsigned int OutputWatchdog;
unsigned long WatchdogTimer=0;

#include <WebServer.h>
WebServer server(HTTP_SERVER_PORT);
#include <ETH.h>
static bool eth_connected = false;
#if defined(EnableOTA)
  #include <ESPmDNS.h>
  #include <ArduinoOTA.h>
#endif

// Host settings, NVS namespace "anthub", edited on /setup, applied after reboot
#define HOST_NVS_NS "anthub"
bool      DHCP_ENABLE = 1;
IPAddress StaticIp(192, 168, 1, 188);
IPAddress StaticMask(255, 255, 255, 0);
IPAddress StaticGw(192, 168, 1, 1);
IPAddress StaticDns(8, 8, 8, 8);

// TrxNet
#include <TrxNet.h>
WiFiUDP  trxUdp;
TrxNet   net(trxUdp);
bool     trxNetEnabled   = false;
char     trxnetAntId[8]  = "01";
char     trxnetTrx1Name[TRXNET_MAX_DEVICE_NAME] = "705.01";  // /hz source for TRX1, picked on /setup
char     trxnetTrx2Name[TRXNET_MAX_DEVICE_NAME] = "OI3.02";  // /hz source for TRX2, picked on /setup
uint16_t trxnetPort      = 5683;
// Last /hz per sender, shown in the /setup device list (filled in onTrxNetHz)
#define HZ_SRC_MAX 8
struct HzSrc {
  char     name[TRXNET_MAX_DEVICE_NAME];
  uint32_t hz;
  uint32_t ms;     // millis() of last /hz, 0 = empty slot
};
HzSrc hzSrc[HZ_SRC_MAX];
// Priority name-prefixes: peers whose name begins with one of these are protected
// from eviction when the peer table (TRXNET_MAX_PEERS) fills — the stalest
// non-priority peer is dropped instead. Configured on /setup (NVS), see
// TrxNet::setPriorityPrefixes().
char        trxnetPrio[2][8]  = { "OI3", "705" };
const char* trxnetPrioPtr[2]  = { trxnetPrio[0], trxnetPrio[1] };

// --- DIN band-switch = AntMatrix external confirmation -------------------------
// Outputs with "Ext confirm" (default #9, multiband vertical) sit behind a remote
// band-switch driven by the DIN device over TrxNet. AntMatrix calls AmExtRequest()
// with the band row ext code; we command DIN's 8 FREE GPIO via /s-gpio (1 byte) and
// confirm via its /gpio echo, then report amExtResult(). Meanwhile AntMatrix keeps the
// output's fallback (default #1 Dummy) active.
// DIN bit->GPIO map is {0,2,4,12,13,14,32,33}; unused bits stay 0 (DIN is dedicated).
// Default codes: 160m=0x90, 80m=0x14, 40m=0x00, 30m=0x18 (GPIO33/4/12/13).
// Set once per band change; retry only on failure; recover on DIN rejoin.
char        trxnetDinName[TRXNET_MAX_DEVICE_NAME] = "DIN.01";  // configured on /setup
enum DinState { DIN_IDLE, DIN_PENDING, DIN_CONFIRMED, DIN_FAILED };
DinState      dinState        = DIN_IDLE;
int           dinReqTrx       = -1;    // which TRX requested the ext output
uint8_t       dinExpectedByte = 0;     // band byte we want DIN to apply
uint8_t       dinConfirmedByte= 0;     // last confirmed byte
uint8_t       dinLastGpio     = 0;     // last /gpio value received from DIN
volatile bool dinGpioRx       = false; // set in /gpio callback, drained in DinBandLoop()
volatile bool dinPeerReappeared = false; // set in onPeerAdded, drained in DinBandLoop()
uint8_t       dinAttempts     = 0;     // /s-gpio sends in current PENDING sequence
unsigned long dinSendTimer    = 0;     // millis() of last /s-gpio send (0 = send now)
const uint8_t DIN_MAX_ATTEMPTS = 3;
const unsigned long DIN_RETRY_MS = 3000;

// Host settings as stored on /setup (see HostRead/HostSave)
struct HostCfg {
  bool      dhcp;
  IPAddress ip, mask, gw, dns;
  uint16_t  port;
  char      antId[sizeof(trxnetAntId)];
  char      trx1[TRXNET_MAX_DEVICE_NAME];
  char      trx2[TRXNET_MAX_DEVICE_NAME];
  char      din[TRXNET_MAX_DEVICE_NAME];
  char      prio[2][sizeof(trxnetPrio[0])];
};

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


  HostLoad();

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
      ETH.config(StaticIp, StaticGw, StaticMask, StaticDns);
      //config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0x00000000, IPAddress dns2 = (uint32_t)0x00000000);
    }
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

   // AntMatrix antenna switch + web
   AmHooks amHooks = {};
   amHooks.writeOutputs = AmWriteOutputs;
   amHooks.extRequest   = AmExtRequest;
   amHooks.extCancel    = AmExtCancel;
   amHooks.changed      = AmChanged;
   amHooks.millis       = AmMillis;
   amHooks.defaults     = AmHostDefaults;
   amInit(amHooks);
   AmConfig amCfg;
   if(!amStoreLoad(amCfg)){
     amLoadDefaults(amCfg);
     Serial.println("AntMatrix: no stored config, using defaults");
   }
   amApplyConfig(amCfg);
   TrxEnabledLoad();
   amWebBegin(server, "<a class=\"tab\" href=\"/setup\">SETUP</a>");
   server.on("/setup",      HTTP_GET,  HttpSetupPage);
   server.on("/api/setup",  HTTP_GET,  HttpSetupGet);
   server.on("/api/setup",  HTTP_POST, HttpSetupPost);
   server.on("/api/reboot", HTTP_POST, HttpReboot);
   server.on("/api/peers",  HTTP_GET,  HttpPeersGet);
   server.on("/api/trxsrc", HTTP_POST, HttpTrxSource);
   server.on("/api/trxen",  HTTP_POST, HttpTrxEnable);
   server.onNotFound([](){ server.send(404, "text/plain", "Not found"); });
   server.begin();

   EnableSerialDebug = 0;
}

//-------------------------------------------------------------------------------------------------------

void loop() {
  server.handleClient();
  if (trxNetEnabled) net.loop();
  if (trxNetEnabled) DinBandLoop();
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
      LcdTrx(0, 0);
      bitClear(LcdNeedRefresh, 1);
    }

    // freq2
    if(bitRead(LcdNeedRefresh, 2)==1){
      tft.drawLine(0,107,340,107, ILI9341_DARKGREY);
      LcdTrx(1, 110);
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

#if defined(TFTLCD)
// One TRX block: frequency + active antenna, second line antenna pool
// pool colors: green = active, orange = waiting for ext confirmation, red = held by other TRX
void LcdTrx(int trx, int y){
  const AmTrx& s = amTrx(trx);
  const AmConfig& c = amConfig();
  tft.fillRect(0, y, 320, 18, ILI9341_BLACK);
  if(s.hz==0){
    tft.setTextColor(ILI9341_DARKGREY);
  }else{
    tft.setTextColor(ConvertRGB(102,178,255));
  }
  tft.setCursor(6,y+2);
  tft.setTextSize(2);

  int MHZ = s.hz/1000000;
  if(MHZ<100 && MHZ>9){
    tft.print(" ");
  }else if(MHZ<10){
    tft.print("  ");
  }
  tft.print(MHZ);
  tft.print(".");
  int KHZ = s.hz/1000-(MHZ*1000);
  if(KHZ<100 && KHZ>9){
    tft.print("0");
  }else if(KHZ<10){
    tft.print("00");
  }
  tft.print(KHZ);
  tft.print(" kHz ");
  tft.setTextColor(ILI9341_LIGHTGREY);
  if(s.active>=0){
    if(s.active+1<10){
      tft.print(" ");
    }
    tft.print(s.active+1);
    tft.print("-");
    tft.print(c.out[s.active].name);
    if(s.want>=0 && s.want!=s.active){
      tft.print("*");  // waiting for ext confirmation
    }
  }else if(s.disabled){
    tft.print("disabled");
  }else if(s.starved){
    tft.setTextColor(ILI9341_RED);
    tft.print("busy");
  }else{
    tft.print("off");
  }

  // POOL
  tft.fillRect(0, y+19, 320, 18, ILI9341_BLACK);
  tft.setCursor(6,y+21);
  tft.setTextSize(2);
  uint16_t taken = amTakenByOthers(trx);
  for(int o=0; o<AM_OUT; o++){
    if(!bitRead(s.pool, o)) continue;
    if(o==s.active){
      tft.setTextColor(ILI9341_GREEN);
    }else if(o==s.want){
      tft.setTextColor(ILI9341_ORANGE);
    }else if(bitRead(taken, o)){
      tft.setTextColor(ConvertRGB(150,0,0));
    }else{
      tft.setTextColor(ILI9341_DARKGREY);
    }
    tft.print(o+1);
    tft.print("-");
    tft.print(c.out[o].name);
    tft.print(" ");
  }
}
#endif
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
                if(strcmp(amConfig().out[i+j*Xbutons+PageShift].name, "n/a")==0){
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
                  tft.print(amConfig().out[i+j*Xbutons+PageShift].name);
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
            tft.setCursor(80, 22+2*12);
            tft.print("ANT "+String(ANT)+" | TRX "+String(TRX)+" | LCD rotation "+String(TftRotation));
            tft.setCursor(80, 22+3*12);
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


  Prn(0,"  Firmware: ");
  Prn(1, String(REV));
  Prn(0,"  Hardware: ");
  Prn(1, String(hardware));
  Prn(0, "  ShiftOut GPIO [data, latch, clock]: ");
  Prn(1, String(ShiftOutDataPin)+", "+String(ShiftOutLatchPin)+", "+String(ShiftOutClockPin));
  for(int t=0; t<AM_TRX; t++){
    const AmTrx& s = amTrx(t);
    String ant = s.active>=0 ? String(s.active+1)+"-"+amConfig().out[s.active].name : String(s.disabled ? "disabled" : s.starved ? "busy" : "off");
    if(s.want>=0 && s.want!=s.active) ant += " (want "+String(s.want+1)+", ext "+amExtName(s.ext)+")";
    Prn(1, "  TRX"+String(t+1)+" "+String(s.hz)+" Hz | ANT "+ant);
  }
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
        net.subscribe("/gpio", onTrxNetGpio);      // DIN band-switch echo
        net.onPeerAdded(onTrxNetPeerAdded);        // re-send /s-gpio on DIN rejoin
        trxNetEnabled = true;
        Prn(1, String("TrxNet begin ") + trxnetDeviceName
                + " prio " + trxnetPrio[0] + "," + trxnetPrio[1]);
      }

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
//------------------------------------------------------------------------------------
void onTrxNetHz(const char* from, const uint8_t* data, size_t len) {
  if (len < 4 || from[0] == '\0') return;   // sender not in peer table yet
  uint32_t freq;
  memcpy(&freq, data, 4);
  HzSrcNote(from, freq);
  if (strcmp(from, trxnetTrx1Name) == 0) {
    if (EnableSerialDebug > 0) { Prn(0, "TrxNet /hz TRX1 "); Prn(1, String(freq)); }
    amSetFreq(0, freq);
  } else if (strcmp(from, trxnetTrx2Name) == 0) {
    if (EnableSerialDebug > 0) { Prn(0, "TrxNet /hz TRX2 "); Prn(1, String(freq)); }
    amSetFreq(1, freq);
  }
}

// Remember last /hz of a sender (reuses its slot, else the oldest one)
void HzSrcNote(const char* from, uint32_t hz){
  int slot = 0;
  for(int i=0; i<HZ_SRC_MAX; i++){
    if(hzSrc[i].ms && strcmp(hzSrc[i].name, from)==0){
      slot = i;
      break;
    }
    if(hzSrc[i].ms < hzSrc[slot].ms) slot = i;
  }
  strlcpy(hzSrc[slot].name, from, sizeof(hzSrc[slot].name));
  hzSrc[slot].hz = hz;
  hzSrc[slot].ms = millis() | 1;
}

const HzSrc* HzSrcFind(const char* name){
  for(int i=0; i<HZ_SRC_MAX; i++){
    if(hzSrc[i].ms && strcmp(hzSrc[i].name, name)==0) return &hzSrc[i];
  }
  return nullptr;
}

// Assign TrxNet device as frequency source of TRX (live + NVS). Empty name = none.
// A device can feed only one TRX, taking it from the other TRX clears that one.
void TrxSourceSet(int trx, const char* name){
  char* dst   = trx==0 ? trxnetTrx1Name : trxnetTrx2Name;
  char* other = trx==0 ? trxnetTrx2Name : trxnetTrx1Name;
  if(name[0] && strcmp(other, name)==0){
    other[0] = '\0';
    amSetFreq(1-trx, 0);
  }
  strlcpy(dst, name, TRXNET_MAX_DEVICE_NAME);
  Preferences p;
  if(p.begin(HOST_NVS_NS, false)){
    p.putString("trx1", trxnetTrx1Name);
    p.putString("trx2", trxnetTrx2Name);
    p.end();
  }
  // take over the last known frequency of the new source, 0 = wait for its /hz
  const HzSrc* h = name[0] ? HzSrcFind(name) : nullptr;
  amSetFreq(trx, h ? h->hz : 0);
  Prn(1, "TRX"+String(trx+1)+" source "+String(name[0] ? name : "none"));
}

// TRX input enable (NVS "anthub" trx1en/trx2en, default enabled), applied live
void TrxEnabledLoad(){
  Preferences p;
  if(!p.begin(HOST_NVS_NS, true)) return;
  amSetEnabled(0, p.getBool("trx1en", true));
  amSetEnabled(1, p.getBool("trx2en", true));
  p.end();
}

void TrxEnabledSet(int trx, bool en){
  amSetEnabled(trx, en);
  Preferences p;
  if(p.begin(HOST_NVS_NS, false)){
    p.putBool(trx==0 ? "trx1en" : "trx2en", en);
    p.end();
  }
  Prn(1, "TRX"+String(trx+1)+(en ? " enabled" : " disabled"));
}

//------------------------------------------------------------------------------------
// DIN band-switch (AntMatrix external confirmation) -------------------------------

// Forget the DIN band-switch state (leaves DIN relays in their last commanded state).
void DinReset() {
  dinState         = DIN_IDLE;
  dinReqTrx        = -1;
  dinExpectedByte  = 0;
  dinConfirmedByte = 0;
  dinAttempts      = 0;
  dinSendTimer     = 0;
}

// /gpio echo from DIN: record it, defer matching to DinBandLoop(). Keep short (runs in net.loop()).
void onTrxNetGpio(const char* from, const uint8_t* data, size_t len) {
  if (len < 1) return;
  if (strcmp(from, trxnetDinName) != 0) return;
  dinLastGpio = data[0];
  dinGpioRx   = true;
}

// DIN (re)joined the peer table: re-arm one send if we still want a band. Keep short.
void onTrxNetPeerAdded(const TrxPeer* peer) {
  if (strcmp(peer->name, trxnetDinName) == 0) dinPeerReappeared = true;
}

// Drives the DIN band-switch state machine from loop() (never from a net.loop() callback).
void DinBandLoop() {
  // confirm: expected /gpio echo arrived while waiting -> AntMatrix routes the ext output
  if (dinGpioRx) {
    dinGpioRx = false;
    if (dinState == DIN_PENDING && dinReqTrx >= 0 && dinLastGpio == dinExpectedByte) {
      dinState         = DIN_CONFIRMED;
      dinConfirmedByte = dinExpectedByte;
      if (EnableSerialDebug > 0) Prn(1, "DIN band confirmed 0x"+String(dinExpectedByte, HEX));
      amExtResult(dinReqTrx, true);
    }
  }

  // DIN reappeared: retry a pending/failed band once more
  if (dinPeerReappeared) {
    dinPeerReappeared = false;
    if (dinState == DIN_FAILED || dinState == DIN_PENDING) {
      dinState     = DIN_PENDING;
      dinAttempts  = 0;
      dinSendTimer = 0;   // send now
    }
  }

  // pending: send /s-gpio (reliable), retry up to DIN_MAX_ATTEMPTS, then give up (fallback stays)
  if (dinState == DIN_PENDING) {
    if (dinSendTimer == 0 || millis() - dinSendTimer >= DIN_RETRY_MS) {
      if (dinAttempts >= DIN_MAX_ATTEMPTS) {
        dinState = DIN_FAILED;
        if (EnableSerialDebug > 0) Prn(1, "DIN band FAILED, stay on fallback");
        amExtResult(dinReqTrx, false);
      } else {
        bool ok = net.publishTo(trxnetDinName, "/s-gpio", &dinExpectedByte, 1, TRX_CON);
        dinAttempts++;
        dinSendTimer = millis();
        if (EnableSerialDebug > 0)
          Prn(1, "DIN /s-gpio 0x"+String(dinExpectedByte, HEX)+" try "+String(dinAttempts)+(ok?" sent":" (peer unknown)"));
      }
    }
  }
}

//------------------------------------------------------------------------------------
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

// AntMatrix glue ----------------------------------------------------------------------------------------

// Defaults (first boot, "Defaults" button): the former hard-coded antenna table.
// Output is allowed in a band row when its old range covered the whole band.
void AmHostDefaults(AmConfig& c){
  static const unsigned long legacyRange[AM_OUT][2] = {  // Hz from, to
    {1810000, 52000000},  // #1 Dummy
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {1810000, 10150000},  // #9 Vertical behind DIN band-switch
    {0, 0},               // #10
    {14000000, 14350000}, // #11
    {21000000, 21450000}, // #12
    {28000000, 29700000}, // #13
    {50000000, 52000000}, // #14
    {0, 0}, {0, 0}
  };
  static const char* legacyName[AM_OUT] = {
    "Dummy", "free", "free", "free", "free", "free", "free", "free",
    "Vertical", "Dipole", "Quad", "Quad", "Quad", "Yagi", "free", "free"
  };
  static const bool legacyTx[AM_OUT] = {1,0,0,0,0,0,0,0, 1,1,1,1,1,1,0,0};

  c.rows[11].fMin = 1810;   // catch-all row: Dummy also outside the bands, as before
  c.rows[11].fMax = 52000;
  for(int i=0; i<AM_ROWS; i++){
    if(c.rows[i].fMax==0) continue;
    for(int o=0; o<AM_OUT; o++){
      if(legacyRange[o][1]!=0 && legacyRange[o][0]<=c.rows[i].fMin*1000UL && c.rows[i].fMax*1000UL<=legacyRange[o][1]){
        for(int t=0; t<AM_TRX; t++) c.rows[i].mask[t] |= (1u << o);
      }
    }
  }
  for(int o=0; o<AM_OUT; o++){
    strlcpy(c.out[o].name, legacyName[o], AM_NAME_LEN);
    c.out[o].disabled = !legacyTx[o];
  }
  c.out[8].extConfirm = true;   // #9 waits for DIN, #1 Dummy meanwhile
  c.out[8].fallback   = 0;
  c.rows[0].extCode = 0x90;     // 160m
  c.rows[1].extCode = 0x14;     // 80m
  c.rows[3].extCode = 0x00;     // 40m
  c.rows[4].extCode = 0x18;     // 30m
}

// TRX1 -> ShiftOutByte[0..1], TRX2 -> ShiftOutByte[2..3], one bit per TRX
void AmWriteOutputs(const int8_t* active){
  for(int i=0; i<4; i++){
    ShiftOutByte[i]=0x00;
  }
  for(int t=0; t<2 && t<AM_TRX; t++){
    if(active[t]>=0 && active[t]<16){
      bitSet(ShiftOutByte[t*2+active[t]/8], active[t]%8);
    }
  }
  digitalWrite(ShiftOutLatchPin, LOW);
  shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[3]);
  shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[2]);
  shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[1]);
  shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftOutByte[0]);
  digitalWrite(ShiftOutLatchPin, HIGH);
  if(EnableSerialDebug>0){
    Prn(1, "TRX1/2 "+String(active[0]+1)+"/"+String(active[1]+1)+"|ShiftOut 0123 "+String(ShiftOutByte[0], BIN)+"|"+String(ShiftOutByte[1], BIN)+"|"+String(ShiftOutByte[2], BIN)+"|"+String(ShiftOutByte[3], BIN));
  }
}

// ext output selected -> start DIN confirm sequence (DinBandLoop() sends immediately)
void AmExtRequest(uint8_t trx, uint8_t out, uint8_t code){
  dinState        = DIN_PENDING;
  dinReqTrx       = trx;
  dinExpectedByte = code;
  dinAttempts     = 0;
  dinSendTimer    = 0;
  if(EnableSerialDebug>0) Prn(1, "TRX"+String(trx+1)+" wants #"+String(out+1)+", DIN code 0x"+String(code, HEX));
}

// TRX left the ext output -> forget state (DIN keeps its last relay state)
void AmExtCancel(uint8_t trx){
  if(dinReqTrx==trx) DinReset();
}

void AmChanged(uint8_t trx){
  #if defined(TFTLCD)
    if(trx<2) bitSet(LcdNeedRefresh, trx+1);
  #endif
}

uint32_t AmMillis(){
  return millis();
}

//-------------------------------------------------------------------------------------------------------
// Host settings (NVS "anthub") --------------------------------------------------------------------------

static void HostGetStr(Preferences& p, const char* key, char* dst, size_t len){
  if(p.isKey(key)) p.getString(key, dst, len);
}

// stored settings, running values where nothing is stored
void HostRead(HostCfg& h){
  h.dhcp = DHCP_ENABLE;
  h.ip   = StaticIp;
  h.mask = StaticMask;
  h.gw   = StaticGw;
  h.dns  = StaticDns;
  h.port = trxnetPort;
  strlcpy(h.antId, trxnetAntId, sizeof(h.antId));
  strlcpy(h.trx1, trxnetTrx1Name, sizeof(h.trx1));
  strlcpy(h.trx2, trxnetTrx2Name, sizeof(h.trx2));
  strlcpy(h.din, trxnetDinName, sizeof(h.din));
  for(int i=0; i<2; i++) strlcpy(h.prio[i], trxnetPrio[i], sizeof(h.prio[i]));
  Preferences p;
  if(!p.begin(HOST_NVS_NS, true)) return;   // nothing saved yet
  h.dhcp = p.getBool("dhcp", h.dhcp);
  h.ip   = p.getUInt("ip",   h.ip);
  h.mask = p.getUInt("mask", h.mask);
  h.gw   = p.getUInt("gw",   h.gw);
  h.dns  = p.getUInt("dns",  h.dns);
  h.port = p.getUShort("port", h.port);
  HostGetStr(p, "antId", h.antId, sizeof(h.antId));
  HostGetStr(p, "trx1",  h.trx1,  sizeof(h.trx1));
  HostGetStr(p, "trx2",  h.trx2,  sizeof(h.trx2));
  HostGetStr(p, "din",   h.din,   sizeof(h.din));
  HostGetStr(p, "prio0", h.prio[0], sizeof(h.prio[0]));
  HostGetStr(p, "prio1", h.prio[1], sizeof(h.prio[1]));
  p.end();
}

// apply stored settings at boot
void HostLoad(){
  HostCfg h;
  HostRead(h);
  DHCP_ENABLE = h.dhcp;
  StaticIp    = h.ip;
  StaticMask  = h.mask;
  StaticGw    = h.gw;
  StaticDns   = h.dns;
  trxnetPort  = h.port;
  strlcpy(trxnetAntId, h.antId, sizeof(trxnetAntId));
  strlcpy(trxnetTrx1Name, h.trx1, sizeof(trxnetTrx1Name));
  strlcpy(trxnetTrx2Name, h.trx2, sizeof(trxnetTrx2Name));
  strlcpy(trxnetDinName, h.din, sizeof(trxnetDinName));
  for(int i=0; i<2; i++){
    strlcpy(trxnetPrio[i], h.prio[i], sizeof(trxnetPrio[i]));
    trxnetPrioPtr[i] = trxnetPrio[i][0] ? trxnetPrio[i] : nullptr;   // empty prefix would match all
  }
}

bool HostSave(const HostCfg& h){
  Preferences p;
  if(!p.begin(HOST_NVS_NS, false)) return false;
  bool ok = p.putBool("dhcp", h.dhcp)
         && p.putUInt("ip", h.ip) && p.putUInt("mask", h.mask) && p.putUInt("gw", h.gw) && p.putUInt("dns", h.dns)
         && p.putUShort("port", h.port)
         && p.putString("antId", h.antId) == strlen(h.antId)
         && p.putString("trx1", h.trx1) == strlen(h.trx1)
         && p.putString("trx2", h.trx2) == strlen(h.trx2)
         && p.putString("din", h.din) == strlen(h.din)
         && p.putString("prio0", h.prio[0]) == strlen(h.prio[0])
         && p.putString("prio1", h.prio[1]) == strlen(h.prio[1]);
  p.end();
  return ok;
}

//-------------------------------------------------------------------------------------------------------
// HTTP /setup ---------------------------------------------------------------------------------------------

void HttpJson(int code, JsonDocument& d){
  String out;
  serializeJson(d, out);
  server.sendHeader("Cache-Control", "no-cache");
  server.send(code, "application/json", out);
}

void HttpError(const char* msg){
  JsonDocument d;
  d["error"] = msg;
  HttpJson(400, d);
}

void HttpSetupPage(){
  server.sendHeader("Cache-Control", "no-cache");
  server.send_P(200, "text/html", SETUP_PAGE);
}

void HttpSetupGet(){
  HostCfg h;
  HostRead(h);
  String call;
  for(int i=141; i<161; i++){
    if(EEPROM.read(i)!=0xff) call += char(EEPROM.read(i));
  }
  JsonDocument d;
  d["call"]  = call;
  d["dhcp"]  = h.dhcp;
  d["ip"]    = h.ip.toString();
  d["mask"]  = h.mask.toString();
  d["gw"]    = h.gw.toString();
  d["dns"]   = h.dns.toString();
  d["port"]  = String(h.port);
  d["antId"] = h.antId;
  d["trx1"]  = h.trx1;
  d["trx2"]  = h.trx2;
  d["din"]   = h.din;
  d["prio0"] = h.prio[0];
  d["prio1"] = h.prio[1];
  d["mac"]   = ETH.macAddress();
  d["curIp"] = ETH.localIP().toString();
  d["fw"]    = REV;
  HttpJson(200, d);
}

// copy name if it fits and uses only [A-Za-z0-9._/-]
bool HttpName(JsonVariantConst v, char* dst, size_t len, bool allowEmpty){
  const char* s = v | "";
  size_t n = strlen(s);
  if(n>=len || (!allowEmpty && n==0)) return false;
  for(size_t i=0; i<n; i++){
    if(!isalnum((unsigned char)s[i]) && !strchr("._/-", s[i])) return false;
  }
  strlcpy(dst, s, len);
  return true;
}

bool HttpIp(JsonVariantConst v, IPAddress& ip){
  const char* s = v | "";
  return ip.fromString(s);
}

void HttpSetupPost(){
  JsonDocument d;
  if(deserializeJson(d, server.arg("plain"))){
    HttpError("bad json");
    return;
  }
  HostCfg h;
  HostRead(h);   // keeps TRX sources, those are set live on the device list
  char call[21];
  h.dhcp = d["dhcp"] | true;
  if(!HttpIp(d["ip"], h.ip) || !HttpIp(d["mask"], h.mask) || !HttpIp(d["gw"], h.gw) || !HttpIp(d["dns"], h.dns)){
    HttpError("bad IP address");
    return;
  }
  long port = d["port"] | 0L;
  if(port<1 || port>65535){
    HttpError("bad UDP port");
    return;
  }
  h.port = port;
  if(!HttpName(d["call"], call, sizeof(call), true)){
    HttpError("bad callsign (max 20 chars A-Z 0-9 . _ / -)");
    return;
  }
  if(!HttpName(d["antId"], h.antId, sizeof(h.antId), false)
     || !HttpName(d["din"], h.din, sizeof(h.din), true)
     || !HttpName(d["prio0"], h.prio[0], sizeof(h.prio[0]), true)
     || !HttpName(d["prio1"], h.prio[1], sizeof(h.prio[1]), true)){
    HttpError("bad TrxNet name (allowed A-Z 0-9 . _ / -)");
    return;
  }
  if(!HostSave(h)){
    HttpError("NVS write failed");
    return;
  }
  // callsign in EEPROM 141-160 (shared with CLI 'L'), 0xff = unused
  for(int i=0; i<20; i++){
    EEPROM.write(141+i, i<(int)strlen(call) ? call[i] : 0xff);
  }
  EEPROM.commit();
  JsonDocument ok;
  ok["ok"] = true;
  HttpJson(200, ok);
}

// TrxNet device list for picking TRX frequency sources
void HttpPeersGet(){
  uint32_t now = millis();
  JsonDocument d;
  d["trx1"] = trxnetTrx1Name;
  d["trx2"] = trxnetTrx2Name;
  d["din"]  = trxnetDinName;
  JsonArray en = d["en"].to<JsonArray>();
  for(int t=0; t<2; t++) en.add(!amTrx(t).disabled);
  JsonArray a = d["peers"].to<JsonArray>();
  for(int i=0; i<net.peerCount(); i++){
    const TrxPeer* p = net.peer(i);
    if(!p) continue;
    JsonObject o = a.add<JsonObject>();
    o["name"] = p->name;
    o["ip"]   = p->ip.toString();
    o["port"] = p->port;
    o["seen"] = now - p->lastSeen;
    const HzSrc* h = HzSrcFind(p->name);
    if(h){
      o["hz"]    = h->hz;
      o["hzAge"] = now - h->ms;
    }
  }
  HttpJson(200, d);
}

// {"trx":0,"name":"705.01"}, empty name = no source
void HttpTrxSource(){
  JsonDocument d;
  char name[TRXNET_MAX_DEVICE_NAME];
  if(deserializeJson(d, server.arg("plain"))){
    HttpError("bad json");
    return;
  }
  long trx = d["trx"] | -1L;
  if(trx<0 || trx>1 || !HttpName(d["name"], name, sizeof(name), true)){
    HttpError("bad trx/name");
    return;
  }
  TrxSourceSet(trx, name);
  JsonDocument ok;
  ok["ok"] = true;
  HttpJson(200, ok);
}

// {"trx":0,"en":false}
void HttpTrxEnable(){
  JsonDocument d;
  if(deserializeJson(d, server.arg("plain"))){
    HttpError("bad json");
    return;
  }
  long trx = d["trx"] | -1L;
  if(trx<0 || trx>1 || !d["en"].is<bool>()){
    HttpError("bad trx/en");
    return;
  }
  TrxEnabledSet(trx, d["en"].as<bool>());
  JsonDocument ok;
  ok["ok"] = true;
  HttpJson(200, ok);
}

void HttpReboot(){
  server.send(200, "application/json", "{\"ok\":true}");
  delay(300);
  ESP.restart();
}
