#include "BLEDevice.h"
#include "Seeed_rpcUnified.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <Arduino_JSON.h>
//Display
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite img = TFT_eSprite(&tft);  // Sprite
String Value11;

#define len(arr) sizeof (arr)/sizeof (arr[0])


// The remote service we wish to connect to.
static BLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID    TXUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID    RXUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");


static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;

int count_address = 0;
int sCount = 0;
static BLERemoteCharacteristic* RRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;


char *rName = "iRobotCreate3"; //name of the SPIKE we want to connect to ... have to name all the SPIKEs robot
char *dataStream; //port information from SPIKE
char *dataGeneral; //general data from SPIKE
int screenValue = 2;
int num=5;


char buf[12]; // "-2147483648\0"



//SPIRTE data
int x = 320;
float scrollSpeed = 1;

//SPIKE port data

String dat;
String addr[10];
int _RSSIvalue[10];
String state = "";
String screen = "Stream"; //default value
int menuState = 0;
int oldmenuState = 0;

bool wasOne = false;
bool wasTwo = false;

//63- Force
//65 - Small Motor
//62 - Distance
//61 - Color sensor
//48 - Medium Motor


int types[6]; //to store types of sensors or Motors in ports
uint8_t portNames[6][2] = {{10, 10}, {250, 10}, {10, 70}, {250, 70}, {10, 130}, {250, 130}}; //location on wio screen for ports A to F
int recData[6][6]; //place to store port values

/*************************************/
/*************************************/
//button interrupt functions

void scanBLE() {
  Serial.println("Button Int PRESS");
  count_address = 0;
  menuState = 0;
  state = "scan";
  if (connected) {
    BLEDevice::deinit();
  }
}

void switchLEFT() {
    screenValue = screenValue - 1;
}
void switchRIGHT() {
    screenValue = screenValue + 1;
}
void switchUP() {
    num = num + 5;
}
void switchDOWN() {
    num = num - 5;
}
void connectBLE() {
  Serial.println("Connect Button Pressed");
  state = "connect";
}
/*************************************/
/*************************************/



static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  dataStream = (char*)pData; //converting uint8_t to char array
  dataStream[length] = '\0'; //adding end of string
  state = "data";
}


class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
    }
    void onDisconnect(BLEClient* pclient) {
      connected = false;
      tft.fillScreen(TFT_BLUE);
      img.setTextFont(4);
      Serial.println("onDisconnect");
    }
};




bool connectToServer() {
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  Serial.println(serviceUUID.toString().c_str());
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  Serial.println(" - Found our service");
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  RRemoteCharacteristic = pRemoteService->getCharacteristic(TXUUID);

  if (RRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    pClient->disconnect();
    return false;
  }
  RRemoteCharacteristic = pRemoteService->getCharacteristic(RXUUID);
  Serial.println(RRemoteCharacteristic->getUUID().toString().c_str());
  Serial.println(" - Found our characteristic");
  // Read the value of the characteristic.
  if (RRemoteCharacteristic->canNotify()) {
    RRemoteCharacteristic->registerForNotify(notifyCallback);
  }
  connected = true;
  return true;
}
char scrollText[250];
void drawTerminal(char *s1="", char *s2="") {
  tft.setTextColor(TFT_BLUE,TFT_BLUE);
  tft.drawString( scrollText,50,220);
  
  scrollText[0]='\0';
  strcpy(scrollText, s1);
  strcat(scrollText, s2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE,TFT_WHITE);
  tft.drawString( scrollText,50,220);

}

//Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {

      if (memcmp(advertisedDevice.getName().c_str(), rName, 5) == 0) { //if the name is SPIKE then add the address to address variable
        addr[count_address] = advertisedDevice.getAddress().toString().c_str();
        _RSSIvalue[count_address] = advertisedDevice.getRSSI(); //save all the RSSI values with name robot
         drawTerminal(itoa(count_address, buf, 10),"devices found");
        count_address += 1;
      }
    }
};

//create a function here that then connects to the selected address and connect
class toConnectMyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //Serial.println(advertisedDevice.getAddress().toString().c_str());
      if (strcmp(advertisedDevice.getAddress().toString().c_str() , addr[0].c_str()) == 0) {
        drawTerminal("SPIKE Prime found");
        Serial.print("BATT Device found: ");
        Serial.println(advertisedDevice.toString().c_str());
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;
      } // onResult
    }
}; // MyAdvertisedDeviceCallbacks


void sortRSSI() {
  for (int i = 0; i < 9; i++)
  {
    if (abs(_RSSIvalue[0]) > abs(_RSSIvalue[i + 1]) && _RSSIvalue[i + 1]) {
      _RSSIvalue[0] = _RSSIvalue[i + 1];
      addr[0] = addr[i + 1];
    }
  }
}

void scanSPIKE() {
  drawTerminal("Scanning SPIKE PRIMEs nearby ... ");
  delay(1000);
  BLEDevice::deinit();
  BLEDevice::init("");
  drawTerminal("BLE initialized");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(2, false);
  


}


void connectSPIKE() {

  //drawTerminal("Starting connection with ", addr[0].toCharArray(buf,10));
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new toConnectMyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

}


/*************************/
//Sprite functions


void build_banner(String msg, int xpos) {
  img.fillSprite(TFT_GREEN);
  img.setTextWrap(false);       // Turn of wrap so we can print past end of sprite
  img.setCursor(xpos + 20, 2); // Print text at xpos
  img.print(msg);

}



void showMessage(String msg, int posx, int posy) {

  img.createSprite(200, 50);

  img.fillSprite(TFT_GREEN);
  img.setTextWrap(false);       // Turn of wrap so we can print past end of sprite

  img.setCursor(posx, 0); // Print text at xpos - sprite width
  img.print(msg);
  img.setCursor(posx - 360, 0); // Print text at xpos
  img.print(msg);
  img.pushSprite(60, posy);
  img.deleteSprite();

}
void hideMessage(int posy) {
  img.createSprite(200, 50);
  img.fillSprite(TFT_BLUE);
  img.pushSprite( 60, posy);
  img.deleteSprite();

}




// #########################################################################
// Sensor , Motor and SPIKE Hub Sprites
// #########################################################################

void build_banner(String msg) {

  img.setTextWrap(false);       // Turn of wrap so we can print past end of sprite
  img.setCursor(18, 30); // Print text at xpos
  img.print(msg);

}

void MOTOR(uint8_t *port , int value) {
  img.createSprite(50, 50);
  img.fillSprite(TFT_WHITE);
  img.setTextFont(1);
  img.fillCircle(25, 15, 10, TFT_BLACK);
  img.fillCircle(25, 15, 9, TFT_WHITE);

  img.fillCircle(25, 10, 2, TFT_BLACK);
  img.fillCircle(25, 20, 2, TFT_BLACK);
  img.fillCircle(20, 15, 2, TFT_BLACK);
  img.fillCircle(30, 15, 2, TFT_BLACK);
  build_banner(String(value));
  img.pushSprite(port[0], port[1]);
  img.deleteSprite();
}

//0 Black
//9 RED
//7 Yellow
//5 green
//4 cyan/blue
int color[11] = {TFT_BLACK, TFT_BLACK, TFT_BLACK, TFT_BLACK, TFT_BLUE, TFT_GREEN, TFT_BLACK, TFT_YELLOW, TFT_BLACK, TFT_RED, TFT_WHITE}; //
void COLOR(uint8_t *port , int value) {
  img.createSprite(50, 50);
  img.fillSprite(TFT_WHITE);
  img.setTextFont(1);
  img.fillRect(15, 5, 20, 20, TFT_BLACK);
  img.fillRect(16, 6, 18, 18, TFT_WHITE);
  img.drawCircle(25, 15, 5, TFT_BLACK);
  img.fillCircle(25, 15, 3, color[value]);
  build_banner(String(value));

  img.pushSprite(port[0], port[1]);
  img.deleteSprite();
}

void DISTANCE(uint8_t *port , int value) {
  img.createSprite(50, 50);
  img.fillSprite(TFT_WHITE);
  img.setTextFont(1);
  img.drawRoundRect(10, 8, 30, 18, 4, TFT_BLACK);
  img.fillRect(14, 12, 22, 8, TFT_BLACK);
  img.drawCircle(16, 16, 3, TFT_WHITE);
  img.fillCircle(16, 16, 2, TFT_BLACK);
  img.drawCircle(34, 16, 3, TFT_WHITE);
  img.fillCircle(34, 16, 2, TFT_BLACK);

  build_banner(String(value));
  img.pushSprite(port[0], port[1]);
  img.deleteSprite();
}

void FORCE(uint8_t *port , int value) {
  img.createSprite(50, 50);
  img.fillSprite(TFT_WHITE);
  img.setTextFont(1);
  img.fillRect(15, 5, 20, 20, TFT_BLACK);
  img.fillRect(16, 6, 18, 18, TFT_WHITE);
  img.fillRoundRect(18, 13, 15, 5, 2, TFT_BLACK);
  img.fillCircle(25, 15, 4, TFT_WHITE);
  img.fillCircle(25, 15, 3, TFT_BLACK);

  build_banner(String(value));
  img.pushSprite(port[0], port[1]);
  img.deleteSprite();
}
void SPIKE(float scale) {
  float SW = 55;
  float SH = 90;
  float BBR = 8;
  float BW = 30;
  float BH = 8;
  float PW = 15;

  SW = scale * SW;
  SH = scale * SH;
  BBR = scale * BBR;
  BW = scale * BW;
  BH = scale * BH;


  img.createSprite(SW, SH);
  img.fillSprite(TFT_WHITE); // Optional here as we fill the sprite later anyway


  img.fillRect(0, 0, SW, SH, TFT_WHITE);
  img.fillRect(SW / 2 - BW / 2, SH - 2 * BH, BW, BH, TFT_LIGHTGREY);
  img.fillCircle(SW / 2 - BW / 2, SH - 1.5 * BH, BH / 2, TFT_LIGHTGREY);
  img.fillCircle(SW / 2 + BW / 2, SH - 1.5 * BH, BH / 2, TFT_LIGHTGREY);
  img.fillCircle(SW / 2, SH - 1.5 * BH, BBR + 1, TFT_BLACK);
  img.fillCircle(SW / 2, SH - 1.5 * BH, BBR, TFT_LIGHTGREY);


  img.fillRect(0, SH / 2 - PW / 2, scale * 2, PW, TFT_DARKGREY);
  img.fillRect(0, SH / 2 - 13 * PW / 6, scale * 2, PW, TFT_DARKGREY);
  img.fillRect(0, SH / 2 + 7 * PW / 6, scale * 2, PW, TFT_DARKGREY);

  img.fillRect(SW - scale * 2, SH / 2 - PW / 2, scale * 2, PW, TFT_DARKGREY);
  img.fillRect(SW - scale * 2, SH / 2 - 13 * PW / 6, scale * 2, PW, TFT_DARKGREY);
  img.fillRect(SW - scale * 2, SH / 2 + 7 * PW / 6, scale * 2, PW, TFT_DARKGREY);

  img.pushSprite(tft.width() / 2 - SW / 2, 20);

  img.deleteSprite();
}


void HIDE(uint8_t *port) {
  img.createSprite(50, 50);
  img.fillSprite(TFT_BLUE);
  img.pushSprite(port[0], port[1]);
  img.deleteSprite();
}

void GRAPH(int light, int motor) {

  img.createSprite(200, 100);
  img.fillSprite(TFT_WHITE);
  img.setTextFont(1);
  img.fillCircle((motor + 180) / 2 , light / 1024 * 100 , 8, TFT_RED);
  img.pushSprite(10, 10);
  img.deleteSprite();
}


/**************************/
///GRAPH SPRITES//
/**************************/


void GRAPH1() {

  JSONVar myObject = JSON.parse(dataStream);
  img.createSprite(250, 200);
  img.fillSprite(TFT_WHITE);
  int x, y;
   //Serial.println(myObject);

  for (int i = 0; i < myObject["data"]["training"].length(); i++) {
    x = map(myObject["data"]["training"][i][0], -180, 180, 60, 240);
    y = map(myObject["data"]["training"][i][1], 0, 1024, 25, 195);
    img.fillCircle(x, 200 - y, 4, TFT_BLUE);
  }

  x = map(myObject["data"]["data"][0], -180, 180, 60, 180);
  y = map(myObject["data"]["data"][1], 0, 1024, 25, 195);

  img.fillCircle(x,  200 - y, 10, TFT_RED);
  img.pushSprite(50, 50);
  img.deleteSprite();
 // Serial.println("done and dusted too");
}


void GRAPH2() {
  int SWidth = 200;
  int SHeight = 200;
  float scale = SWidth/num;
  JSONVar myObject = JSON.parse(dataStream);
  img.createSprite(200, 200);
  img.fillSprite(TFT_WHITE);
  int x, y;
  for (int i = 0; i < num; i++) {
    img.drawFastHLine(0, scale * i, SWidth , TFT_GREEN); //Horizontal line
    img.drawFastVLine(scale * i, 0, SHeight, TFT_GREEN); //Verical line
  }
  if (myObject["data"]["training"].length() > 0) {
    for (int i = 0; i < myObject["data"]["training"].length(); i++) {

      x = map(myObject["data"]["training"][i][0], -180, 180, 0, num);
      y = map(myObject["data"]["training"][i][1], 0, 1024, 0, num);
      img.fillRect(x * scale, y * scale, scale, scale, TFT_BLUE);
     // Serial.println(x,y);
    }
  }
  x = map(myObject["data"]["data"][0], -180, 180, 0, num);
  y = map(myObject["data"]["data"][1], 0, 1024, 0, num);
//  Serial.println(x,y);
  img.drawRect(x * scale, y * scale, scale, scale, TFT_RED);
  img.pushSprite(60, 10);
  img.deleteSprite();
 // Serial.println("done and dusted");
}


void GRAPH3() {
  int SWidth = 200;
  int SHeight = 200;
  float scale = SWidth/num;
  JSONVar myObject = JSON.parse(dataStream);
  img.createSprite(200, 200);
  img.fillSprite(TFT_WHITE);
  int x, y;
  for (int i = 0; i < num; i++) {
    img.drawFastHLine(0, scale * i, SWidth , TFT_GREEN); //Horizontal line
    img.drawFastVLine(scale * i, 0, SHeight, TFT_GREEN); //Verical line
  }
  if (myObject["data"]["training"].length() > 0) {
    for (int i = 0; i < myObject["data"]["training"].length(); i++) {

      x = map(myObject["data"]["training"][i][0], -180, 180, 0, num);
      y = map(myObject["data"]["training"][i][1], 0, 1024, 0, num);
      img.fillRect(x * scale, y * scale, scale, scale, TFT_BLUE);
      img.drawFastHLine(0, scale * y +scale/2, SWidth , TFT_BLUE); //Horizontal line
      img.drawFastVLine(scale * x+scale/2 , 0, SHeight, TFT_BLUE); //Verical line
    }
  }
  x = map(myObject["data"]["data"][0], -180, 180, 0, num);
  y = map(myObject["data"]["data"][1], 0, 1024, 0, num);
//  Serial.println(x,y);
  img.drawRect(x * scale, y * scale, scale, scale, TFT_RED);
  img.pushSprite(60, 10);
  img.deleteSprite();
 // Serial.println("done and dusted");
}

void GRAPH4() {
  int SWidth = 200;
  int SHeight = 200;
  float scale = SWidth/num;
  JSONVar myObject = JSON.parse(dataStream);
  img.createSprite(200, 200);
  img.fillSprite(TFT_WHITE);
  int x, y;
  for (int i = 0; i < num; i++) {
    img.drawFastHLine(0, scale * i, SWidth , TFT_GREEN); //Horizontal line
    img.drawFastVLine(scale * i, 0, SHeight, TFT_GREEN); //Verical line
  }
  if (myObject["data"]["training"].length() > 0) {
    for (int i = 0; i < myObject["data"]["training"].length(); i++) {

      x = map(myObject["data"]["training"][i][0], -180, 180, 0, num);
      y = map(myObject["data"]["training"][i][1], 0, 1024, 0, num);

            img.fillRect(0, scale * y ,SWidth,scale , TFT_BLUE); //Horizontal rect
      img.fillRect(scale * x , 0, scale, SHeight, TFT_BLUE); //Verical rect
      img.fillRect(x * scale, y * scale, scale, scale, TFT_BLUE);

    }
  }
  x = map(myObject["data"]["data"][0], -180, 180, 0, num);
  y = map(myObject["data"]["data"][1], 0, 1024, 0, num);
//  Serial.println(x,y);
  img.drawRect(x * scale, y * scale, scale, scale, TFT_RED);
  img.pushSprite(60, 10);
  img.deleteSprite();
 // Serial.println("done and dusted");
}
void GRAPH5() {

  int SWidth = 200;
  int SHeight = 200;
  float scale = SWidth/num;
  JSONVar myObject = JSON.parse(dataStream);
  img.createSprite(200, 200);
  img.fillSprite(TFT_WHITE);
  int x, y;
  for (int i = 0; i < num; i++) {
    img.drawFastHLine(0, scale * i, SWidth , TFT_GREEN); //Horizontal line
    img.drawFastVLine(scale * i, 0, SHeight, TFT_GREEN); //Verical line
  }
  if (myObject["data"]["training"].length() > 0) {
    for (int i = 0; i < myObject["data"]["training"].length(); i++) {

      x = map(myObject["data"]["training"][i][0], -180, 180, 0, num);
      y = map(myObject["data"]["training"][i][1], 0, 1024, 0, num);

            img.fillRect(0, scale * y ,SWidth,scale , TFT_LIGHTGREY); //Horizontal rect
      img.fillRect(scale * x , 0, scale, SHeight, TFT_LIGHTGREY); //Verical rect
      img.fillRect(x * scale, y * scale, scale, scale, TFT_BLUE);

    }
  }
  x = map(myObject["data"]["data"][0], -180, 180, 0, num);
  y = map(myObject["data"]["data"][1], 0, 1024, 0, num);
//  Serial.println(x,y);
  img.drawRect(x * scale, y * scale, scale, scale, TFT_RED);
  img.pushSprite(60, 10);
  img.deleteSprite();
  Serial.println("number was");

}
void GRAPH6() {
  Serial.println("to be added");
}
void GRAPH7() {
  Serial.println("to be added");
}
void GRAPH8() {
  Serial.println("to be added");
}
void GRAPH9() {
  Serial.println("to be added");
}

void setup() {
  Serial.begin(115200);
  // while (!Serial) {};


  //screen stuff
  tft.begin();
  tft.init();
  tft.setRotation(3);

  tft.fillScreen(TFT_BLUE);
  img.setTextSize(1);           // Font size scaling is x1
  img.setTextFont(4);           // Font 2 selected
  img.setTextColor(TFT_BLACK);  // Black text, no background colour


  //scan bluetooth
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  //selection buttons
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WIO_KEY_C),     scanBLE, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_5S_LEFT),    switchLEFT, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_5S_RIGHT),  switchRIGHT, FALLING);
    attachInterrupt(digitalPinToInterrupt(WIO_5S_UP),    switchUP, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_5S_DOWN),  switchDOWN, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_5S_PRESS), connectBLE, FALLING);

  delay(2000);

}




void decodeSPIKEdataStream() {

  JSONVar myObject = JSON.parse(dataStream);
  if (int(myObject["m"]) == 1) {
    if (wasOne) {
    }
    else {
      tft.fillScreen(TFT_BLUE);
      SPIKE(2); //draw SPIKE Sprite
      wasOne = true;
      wasTwo = false;

    }
    types[0] = myObject["A"]["t"];
    types[1] = myObject["B"]["t"];
    types[2] = myObject["C"]["t"];
    types[3] = myObject["D"]["t"];
    types[4] = myObject["E"]["t"];
    types[5] = myObject["F"]["t"];

    for (int i = 0; i < myObject["A"]["d"].length(); i++) {
      recData[0][i] = myObject["A"]["d"][i];
    }

    for (int i = 0; i < myObject["B"]["d"].length(); i++) {
      recData[1][i] = myObject["B"]["d"][i];
    }
    for (int i = 0; i < myObject["C"]["d"].length(); i++) {
      recData[2][i] = myObject["C"]["d"][i];
    }
    for (int i = 0; i < myObject["D"]["d"].length(); i++) {
      recData[3][i] = myObject["D"]["d"][i];
    }
    for (int i = 0; i < myObject["E"]["d"].length(); i++) {
      recData[4][i] = myObject["E"]["d"][i];
    }
    for (int i = 0; i < myObject["F"]["d"].length(); i++) {
      recData[5][i] = myObject["F"]["d"][i];
    }
/*
 *
MOTOR_MEDIUM = 48
MOTOR_LARGE = 49
COLOR_SENSOR = 61
ULTRASOUND_SENSOR = 62
TOUCH_SENSOR = 63
TILT_SENSOR = 34 
 */
 

    for (int i = 0; i < 6; i++) {
      if (types[i] == 48 ||types[i] == 49 || types[i] == 65) {
        MOTOR(portNames[i], recData[i][1]);
      }
      else if (types[i] == 61) {
        COLOR(portNames[i], recData[i][1]);
      }
      else if (types[i] == 62) {
        DISTANCE(portNames[i], recData[i][0]);
      }
      else if (types[i] == 63) {
        FORCE(portNames[i], recData[i][0]);
      }
      else {
        HIDE(portNames[i]);
      }
    }
  }
  else {
    if (wasTwo) {
    }
    else {
     tft.fillScreen(TFT_YELLOW);
     Serial.println("forbidden but yet you go????");
      //draw GRAPH sprite Sprite
      wasOne = false;
      wasTwo = true;

    }
    //data format
    // {"m": 2, "data": {"data": [1024, 107], "training": [[3, 104], [3, 104], [-112, 107], [104, 121], [107, 1024]]}}


    if (screenValue == 1) {
      GRAPH1();
    }
    else if (screenValue == 2) {
      GRAPH2();
    }
    else if (screenValue == 3) {
      GRAPH3();
    }
    else if (screenValue == 4) {
      GRAPH4();
    }
    else if (screenValue == 5) {
      GRAPH5();
    }
  }
}


void loop() {
  if (state == "connect" && not connected ) {
    //if the central button is pressed , connect the selected robot
    //scan all the BLE devices with name robot
    //sort find the one that has  largest RSSI -1 > -30
    //connect to the SPIKE with the largest RSSI
    scrollSpeed = 100;
    hideMessage(180);
    showMessage("Connecting ", 50, 50);
    scanSPIKE();
    sortRSSI();
    //initializes the BLE and searches and connects to the given BLE
    connectSPIKE();
    if (doConnect == true) {
      if (connectToServer()) {
        hideMessage(50);
        hideMessage(180);
        Serial.println("We are now connected to the BLE Server.");
      } else {
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      }
      doConnect = false;
    }
    state = "";
    hideMessage(50);
  }

  else if (state == "data" &&  connected) {
    state = "";
    decodeSPIKEdataStream();
  }

  else if (state == "" && not connected) {
    scrollSpeed = 1;
    showMessage("Press round button to connect!!", x, 180);
    x = x - 1;
    if (x < 1) {
      x = 320;
    }
  }

  Serial.println(screenValue);
  Serial.printf(".");
  delay(scrollSpeed);
} // 
