/*

  lunarGateway.cpp - Library for connecting the ESP32 with the Lunar 2021 scale

  Created by Frowin Ellermann, 2022.

  Released under the MIT license.

  https://github.com/frowin/LunarGateway

*/

#ifndef LUNAR_GATEWAY_H
#define LUNAR_GATEWAY_H
#include <Arduino.h>
#include "BLEDevice.h"
class lunarGateway {
public:
  lunarGateway();
  void init(BLERemoteCharacteristic* commandChar, BLERemoteCharacteristic* weightChar);
  void sendId();
  void sendHeartBeat();
  void doTare();
  void startTimer();
  void notificationRequest();
  void stopTimer();
  void resetTimer();
  void getSettings();
  void decodeMessage(char* message);
  


  float weight;
  int battery;
private:
  int _test;
  int _mac;
  BLERemoteCharacteristic* _commandChar;
  BLERemoteCharacteristic* _weightChar;
  uint8_t* _prepareRequest(int msgtype,uint8_t * message, size_t size);
  char* _substr(const char* src, int start_index, int end_index);
};

#endif
