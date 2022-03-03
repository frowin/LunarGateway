/*

  lunarGateway.cpp - Library for connecting the ESP32 with the Lunar 2021 scale

  Created by Frowin Ellermann, 2022.

  Released under the MIT license.

  https://github.com/frowin/LunarGateway

  Does anybody read the comments in the beginning? If you read this, please text me on twitter @frowin :)

  Don't forget to set BLE_MAC to your Acaia Lunar 2021's BLE MAC address!


  
  To send some commands set Serial monitor to 115200 and "new line"

  command | action
  --------------------------
  t       | tare
  s       | start timer
  h       | stop  timer
  r       | reset timer
  w       | get weight
  b       | get battery status
  m       | activate/deactivate debugging ouput of incoming BLE payloads
  n       | subscribe to notification (should not be necessary)



*/

#include <BLEDevice.h>
#include <stdio.h>
#include "lunarGateway.h"

static BLEUUID   serviceUUID("49535343-FE7D-4AE5-8FA9-9FAFD205E455"); // service id
static BLEUUID      charUUID("49535343-8841-43f4-a8d4-ecbe34729bb3"); // characteristic id for sending commands 
static BLEUUID    weightUUID("49535343-1e4d-4bd9-ba61-23c647249616"); // characteristic id for receiving data as e.g. weight

static std::string BLE_MAC = "34:81:f4:e0:1b:ff"; // BLE MAC address of your Lunar 2021 scale

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* commandChar;
static BLERemoteCharacteristic* weightChar;

static BLEAdvertisedDevice* myDevice;

long lastHeartBeat = 0;
long lastConnected = 0;
bool subscribeToCallbackMessages = false;


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getAddress().toString() == BLE_MAC) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  } 
}; 

lunarGateway lunar; // create object with lunarGateway class

void setup() {

  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  
  // setting up the BLE stack
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(1, false);
} 

void loop() {
  handleSerial(); // check if there is any incoming serial communication and handles it if needed
  
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // check if last data set received from scale is older than 250ms ago
  if((millis()-lastConnected)>250){
    Serial.println("No connection");
  }

  if (connected) {
    doScan = false;
    
    if((millis()-lastHeartBeat)>2750){ // send heartbeat every 2750ms,
                                        // after 3000ms without heartbeat or communication, the scale disconnects
      lunar.notificationRequest();
      delay(200);
      lunar.sendHeartBeat();
      lastHeartBeat = millis();
    }    
  
  }else if(doScan){
    Serial.println("No connection");
    BLEDevice::getScan()->start(0); 
  }else{
    Serial.println("No connection"); 
    doScan = true;
  }
} 

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {    
    char* pDataChar = (char*)pData;
    if(subscribeToCallbackMessages)
    printHEXPointer(pDataChar); // print every received package to serial
    lastConnected = millis();
    lunar.decodeMessage(pDataChar); // decode received message 
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

    // Connect to the lunar scale
    pClient->connect(myDevice); 
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // Obtain a reference to the service we are after in the scale
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    commandChar = pRemoteService->getCharacteristic(charUUID);
    weightChar = pRemoteService->getCharacteristic(weightUUID);
    lunar.init(commandChar, weightChar); // feed characteristic data into lunar object
    
    if (commandChar == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    if(commandChar->canRead()) {
      std::string value = commandChar->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }else{
      Serial.println("Characteristic is not readable...");
    }

    if(commandChar->canNotify())
      commandChar->registerForNotify(notifyCallback);

    if (weightChar == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(weightUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    if(weightChar->canRead()) {
      std::string value = weightChar->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }else{
      Serial.println("Characteristic is not readable...");
    }

    if(weightChar->canNotify())
      weightChar->registerForNotify(notifyCallback);
    connected = true;
    return true;
}

void printHEXPointer(char* message){
  for (int cnt = 0; cnt < strlen(message); cnt++){
    Serial.print(message[cnt], HEX); Serial.print(" ");
  }
  Serial.println("");
}

void handleSerial(){
  String incomingString;
  if (Serial.available() > 0) {
        incomingString = Serial.readStringUntil('\n');
    if(incomingString == "t"){
      lunar.doTare();
    }else if(incomingString == "s"){
      lunar.startTimer();
    }else if(incomingString == "h"){
      lunar.stopTimer();
    }else if(incomingString == "r"){
      lunar.resetTimer();
    }else if(incomingString == "w"){
      Serial.println(String(lunar.weight)+" g");
    }else if(incomingString == "b"){
      Serial.println(String(lunar.battery)+" %");
    }else if(incomingString == "m"){
      subscribeToCallbackMessages = !subscribeToCallbackMessages;
    }else if(incomingString == "n"){
      lunar.notificationRequest();
    }
  }
}