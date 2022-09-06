/**
   A BLE client example that is rich in capabilities.
   There is a lot new capabilities implemented.
   author unknown
   updated by chegewara
*/

#include "rpcBLEDevice.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>



#define SERVICE_ID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// The remote service we wish to connect to.
//static BLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
// The characteristic of the remote service we are interested in.
//static BLEUUID    charUUID(0x2A19);
bool response=false;
String state = "";
char button;
char *rName = "iRobotCreate3"; //name of the SPIKE we want to connect to ... have to name all the SPIKEs robot
char *dataStream; //port information from SPIKE
uint8_t *dataSt;
char *dataGeneral; //general data from SPIKE
int screenValue = 2;
int num = 5;
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteCharacteristic* TRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
uint8_t bd_addr[6] = {0xc1, 0x5e, 0x4b, 0xa4, 0x16, 0x00};
uint8_t predata[5][20]={ {0x64, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF6},
                        {0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54},    
                        {0x00, 0x00, 0x00, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8B},
                        {0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25},
                       {0x64, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF6}};
                       //   {0x64, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF6} // get ip
                        //0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x64,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD1} // go forward
                        //{0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25} // go right
                        //{0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E}//stop
                        //{0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8A} //left
                 //   {0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54},//get name
                 //{0x00, 0x00, 0x00, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8B}, //get version of color board
                  //{0x00, 0x00, 0x00, 0xA5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D}   //get the version of main board   
                  //{0x01, 0x04, 0x00 ,0xFF ,0xFF, 0xFF, 0x9C, 0xFF, 0xFF, 0xFF, 0x9C,0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x71} //go back
uint8_t command[20]={0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//create the command programmatically

BLEAddress BattServer(bd_addr);

//button callbacks

void switchLEFT() {
  state = "buttonPressed";
  button = 'l';
}
void switchRIGHT() {
  state = "buttonPressed";
  button = 'r';
}
void switchUP() {
  state = "buttonPressed";
  button = 'u';

}
void switchDOWN() {
  state = "buttonPressed";
  button = 'd';
}
void switchPRESS() {
  state = "buttonPressed";
  button = 's';
}

static void notifyCallback(
  BLERemoteCharacteristic* TBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  //dataStream = (char*)pData; //converting uint8_t to char array
  dataSt=pData;
 // dataStream[length] = '\0'; //adding end of stringA5
  state = "data";

}




class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
    }
    void onDisconnect(BLEClient* pclient) {
      connected = false;
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
  Serial.println(" - Found our characteristic");
Serial.println(TRemoteCharacteristic->canNotify());
Serial.println(pRemoteCharacteristic->canNotify());
if (TRemoteCharacteristic->canNotify()) {
   TRemoteCharacteristic->registerForNotify(notifyCallback);
  }
  Serial.println("done");
  connected = true;
  return true;
}
/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());
      Serial.println(advertisedDevice.getServiceUUID().toString().c_str());

      // We have found a device, let us now see if it contains the service we are looking for.
      if (memcmp(advertisedDevice.getAddress().getNative(), BattServer.getNative(), 6) == 0) {
        Serial.print("BATT Device found: ");
        Serial.println(advertisedDevice.toString().c_str());
        BLEDevice::getScan()->stop();
        Serial.println("new BLEAdvertisedDevice");
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        Serial.println("new BLEAdvertisedDevice done");
        doConnect = true;
        doScan = true;
      } // onResult
    }
}; // MyAdvertisedDeviceCallbacks

void buildCommand(){
  if (button == 'u') {
    Serial.println("up");
    for (int i=0;i<20;i++){
      command[i]=predata[0][i];
    }
    response=false;
    button=' ';
    state=" ";
    
  }
  else if (button == 'd') {
    Serial.println("down");
    for (int i=0;i<20;i++){
      command[i]=predata[1][i];
    }
    response=true;
    button=' ';
    state=" ";
  }
  else if (button == 'l') {
    Serial.println("left");    
    for (int i=0;i<20;i++){
      command[i]=predata[2][i];
    }
    response=false;
    button=' ';
    state=" ";
  }


  else if (button == 'r') {
    Serial.println("right");
    for (int i=0;i<20;i++){
      command[i]=predata[3][i];
    }
    response=false;
    button=' ';
    state=" ";
  }
    else if (button == 's') {
    Serial.println("right");
    for (int i=0;i<20;i++){
      command[i]=predata[4][i];
    }
    response=false;
    button=' ';
    state=" ";
  }
  else {
    Serial.print("state value is: ");
    Serial.println(state);
    Serial.println("somethingelse");
  }
  
}



void setup() {
  Serial.begin(9600);
  while (!Serial) {};

  Serial.println("Starting Arduino BLE Client application...");

  //setting button callbacks


  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);


  attachInterrupt(digitalPinToInterrupt(WIO_5S_LEFT),    switchLEFT, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_5S_RIGHT),  switchRIGHT, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_5S_UP),    switchUP, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_5S_DOWN),  switchDOWN, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIO_5S_PRESS),  switchPRESS, FALLING);

  //setting BLE

  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

} 
// End of setup.
// This is the Arduino main loop function.
void loop() {
  if (connected) {
    if (state == "data") {
      Serial.print("entered");
      state = "";
       //Serial.println("dataStream:");
      //for (int i=0;i<20;i++){
       // Serial.print(dataStream[i]);}
        Serial.println("dataSt:");
        for (int i=0;i<20;i++){
        Serial.print(dataSt[i],HEX);
        Serial.print(",");
      }
      
      
    }
    else if (state =="buttonPressed") {
      buildCommand();
      pRemoteCharacteristic->writeValue(command,20);//response);
      
      
      Serial.print("okay");
      if(response){
       // delay(5000);
      //std::string value = pRemoteCharacteristic->readValue();
      //Serial.print("The characteristic value was: ");
      //Serial.println(value.c_str());
      }
    }
  }
  else {
    if (doConnect == true) {
      if (connectToServer()) {
        Serial.println("We are now connected to the BLE Server.");
      } else {
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      }
      doConnect = false;
      state = " ";
    }
  }
  Serial.printf(".");
  delay(1000);
} // End of loop
