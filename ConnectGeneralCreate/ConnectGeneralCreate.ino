/**
   A BLE client example that is rich in capabilities.
   There is a lot new capabilities implemented.
   author unknown
   updated by chegewara
*/

#include "rpcBLEDevice.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

//Display
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite img = TFT_eSprite(&tft);  // Sprite




#define SERVICE_ID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// The remote service we wish to connect to.
//static BLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
// The characteristic of the remote service we are interested in.
//static BLEUUID    charUUID(0x2A19);
bool response=false;

uint8_t *dataSt;

int count_address = 0;
int sCount = 0;

char buf[12]; // "-2147483648\0"

char *Identifier= "48c5d828-ac2a-442d-97a3-0c9822b04979"; //identifier of create we want to connect to

//SPIRTE data
int x = 320;
float scrollSpeed = 1;


static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;


static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteCharacteristic* TRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

uint8_t predata[2][20]={{0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A},//call name    //the id is 1 (the 3rd byte)
                        {0x64, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEA}}; //call ip   //the id is 2 (the third byte)



String addr[20];
int _RSSIvalue[20];
String state = "";
String screen = "Stream"; //default value


int menuState = 0;
int oldmenuState = 0;


//button callbacks

void scanBLE() {
  Serial.println("Button Int PRESS");
  count_address = 0;
  menuState = 0;
  state = "scan";
  if (connected) {
    BLEDevice::deinit();
  }
}

void connectBLE() {
  Serial.println("Connect Button Pressed");
  state = "connect";
}

void C() {
  state = "buttonPressed";
}

static void notifyCallback(
  BLERemoteCharacteristic* TBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  dataSt=pData;
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
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);
  Serial.println(" - Connected to server");
  BLERemoteService* pRemoteService = pClient->getService(SERVICE_ID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");
  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_RX);
  TRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_TX);
  if (TRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(CHARACTERISTIC_UUID_RX);
    pClient->disconnect();
    return false;
  }

  //make a button green or something
  Serial.println(" - Found our characteristic");
hideMessage(50);
 showMessage("Press the bumper ", 10, 50);
Serial.println(TRemoteCharacteristic->canNotify());
Serial.println(pRemoteCharacteristic->canNotify());
if (TRemoteCharacteristic->canNotify()) {
   TRemoteCharacteristic->registerForNotify(notifyCallback);
  }
  Serial.println("done");
  connected = true;
  hideMessage(10);
  showMessage("Press the button left button on top ", 10, 50);
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
/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      
    // store all the ip and rssi values with the identifier for creates 
   if (memcmp(advertisedDevice.getServiceUUID().toString().c_str(), Identifier, 12) == 0){
        addr[count_address] = advertisedDevice.getAddress().toString().c_str();
        _RSSIvalue[count_address] = advertisedDevice.getRSSI(); //save all the RSSI values with Create identifier
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
        drawTerminal("Create found");
        Serial.println(advertisedDevice.toString().c_str());
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;
      } // onResult
    }
}; // MyAdvertisedDeviceCallbacks


void sortRSSI() {
  for (int i = 0; i < 20; i++)
  {
    if (abs(_RSSIvalue[0]) > abs(_RSSIvalue[i + 1]) && _RSSIvalue[i + 1]) {
      _RSSIvalue[0] = _RSSIvalue[i + 1];
      addr[0] = addr[i + 1];
    }
  }
}


void scanCREATE() {
  drawTerminal("Scanning CREATEs nearby ... ");
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


void connectCREATE() {

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
void build_banner(String msg) {

  img.setTextWrap(false);       // Turn of wrap so we can print past end of sprite
  img.setCursor(18, 30); // Print text at xpos
  img.print(msg);

}


void showMessage(String msg, int posx, int posy) {

  img.createSprite(280, 50);

  img.fillSprite(TFT_GREEN);
  img.setTextWrap(false);       // Turn of wrap so we can print past end of sprite

  img.setCursor(posx, 0); // Print text at xpos - sprite width
  img.print(msg);
  img.setCursor(posx - 360, 0); // Print text at xpos
  img.print(msg);
  img.pushSprite(20, posy);
  img.deleteSprite();

}
void hideMessage(int posy) {
  img.createSprite(280, 50);
  img.fillSprite(TFT_BLUE);
  img.pushSprite( 20, posy);
  img.deleteSprite();

}



void DisplayName() {
  char Cname[20];
  int a=0;
  img.createSprite(250, 50);
  img.fillSprite(TFT_WHITE);
  img.drawRoundRect(10, 8, 30, 18, 4, TFT_BLACK);
   
   for (int i=3;dataSt[i]!='\0' or i<19;i++){
      Cname[i-3]=(char)dataSt[i];
      a=i;
   }
   Cname[a+1]='\0';
  showMessage(String("Name : ")+ String(Cname),10,10);
  img.pushSprite(10, 50);
  img.deleteSprite();
}

void DisplayIp(){
  char ip[20];
  String temp="";
  img.createSprite(250, 200);
  img.fillSprite(TFT_WHITE);
  img.drawRoundRect(10, 8, 30, 18, 4, TFT_BLACK);
  for (int i=3;i<7;i++){
    temp+=String(dataSt[i])+".";
        }
   showMessage(String("wlan0 :") + temp,10,70);
   Serial.println(String("wlan0 :") + temp);
   temp="";
  for (int i=7;i<11;i++){
    temp+=String(dataSt[i])+".";
        }
   showMessage(String("wlan1 :") + temp,10,100);
   temp="";
     for (int i=11;i<15;i++){
    temp+=String(dataSt[i])+".";
        }
   showMessage(String("usb :") + temp,10,130);



  img.pushSprite(10, 10);
  img.deleteSprite();


}

void setup() {

  Serial.begin(9600);
 // while (!Serial) {};
  delay(1000);
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
  attachInterrupt(digitalPinToInterrupt(WIO_KEY_C),     C, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_5S_PRESS), connectBLE, FALLING);

} 
// End of setup.
// This is the Arduino main loop function.
void loop() {
  if (connected) {
    if (state == "data") {
         state = " ";
      if(dataSt[0]==12){ //the bumper is pressed
        pRemoteCharacteristic->writeValue(predata[0],20);
        
      }  
      else if(dataSt[2]==1){ //the return should be name
        DisplayName();        
        Serial.println("Name");
        for (int i=3;dataSt[i]!='\0';i++){
        //Serial.print(dataSt[i]);
        Serial.print((char)dataSt[i]);
      }
      // call the ip from inside the name 
      pRemoteCharacteristic->writeValue(predata[1],20);//response);
      }
      else if(dataSt[2]==2){ //the return should be ip
        DisplayIp();
        /*
        Serial.println("IP Address: ");
        Serial.print("wlan0: ");
        for (int i=3;i<7;i++){
          Serial.print(dataSt[i],DEC);
          Serial.print(".");
        }
        Serial.println("");
        Serial.print("wlan1: ");
        for (int i=7;i<11;i++){
          Serial.print(dataSt[i],DEC);
          Serial.print(".");
        }
        Serial.println("");
        Serial.print("usb: ");
        for (int i=11;i<15;i++){
          Serial.print(dataSt[i],DEC);
          Serial.print(".");
        }*/
      }
      
    }
    else if (state =="buttonPressed") { // when the central button is pressed it will start the chain of commands starting with calling the name
      //call name
      pRemoteCharacteristic->writeValue(predata[0],20);//response);  // the response will create a callback and will call ip command
    }
  }
  else if (state == "connect" && not connected) {
    scrollSpeed = 100;
    hideMessage(180);
    showMessage("Connecting ", 50, 50);
    scanCREATE();
    sortRSSI();
    //initializes the BLE and searches and connects to the given BLE
    connectCREATE();
    if (doConnect == true) {
      if (connectToServer()) {
       hideMessage(50);
        hideMessage(180);
        Serial.println("We are now connected to the BLE Server.");
      } else {
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      }
      doConnect = false;
      state = " ";
    }
  }
  else if (state == "" && not connected) {
    scrollSpeed = 1;
    showMessage("Press round button to connect!!", x, 180);
    x = x - 1;
    if (x < 1) {
      x = 320;
    }
  }
  //Serial.println(screenValue);
  Serial.printf(".");
  delay(scrollSpeed);
} // End of loop
