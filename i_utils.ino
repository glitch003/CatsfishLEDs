int countDevicesInRange(){
  // count how many devices we've seen recently
  int devicesInRange = 0;
  for(int i = 0; i < seenDevicesCount; i++){
    SeenDevice s = seenDevices[i];
    if ((millis() - s.lastSeenAt) < PROXIMITY_TIMEOUT){ // timeout
      devicesInRange++;
    }
  }
  return devicesInRange;
}


int floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint8_t rssiToBrightness(int8_t rssi){

  // rssi range is about -40 to -100
  int8_t maxSignal = -40;
  int8_t minSignal = -100;

  if (rssi > maxSignal){
    return 255;
  }

  int range = -1 * (maxSignal - minSignal);
  rssi = rssi - maxSignal; // make rssi start at 0 and go to

//  Serial.print("non inverted percent: ");
//  Serial.println((float)rssi / (float)range);

  float percent = 1 - ((float)rssi / (float)range);

//  Serial.print("final inverted percent: ");
//  Serial.println(percent);



  uint8_t finalValue = (uint8_t)(round(percent * (float)255));

//  Serial.print("final value of led count: ");
//  Serial.println(finalValue);

    if (rssi < minSignal || finalValue == 0) {
      return 1; // never show 0 brightness.  the device is in range, it should always show at least 1.
    }

  return finalValue;


}


int rssiToNextHeartbeatFrame(int8_t rssi){

  // rssi range is about -40 to -100
  int8_t maxSignal = -40;
  int8_t minSignal = -100;

  if (rssi > maxSignal){
    return 5;
  }

  int range = -1 * (maxSignal - minSignal);
  rssi = rssi - maxSignal; // make rssi start at 0 and go to

//  Serial.print("non inverted percent: ");
//  Serial.println((float)rssi / (float)range);

  float percent = 1 - ((float)rssi / (float)range);

//  Serial.print("final inverted percent: ");
//  Serial.println(percent);



  int finalValue = round(percent * (float)100);

//  Serial.print("final value of led count: ");
//  Serial.println(finalValue);

    if (rssi < minSignal || finalValue == 0) {
      return 100; // never show 0 brightness.  the device is in range, it should always show at least 1.
    }

  return finalValue;


}


void printOwnMacAddress(){
  uint8_t addr[6];
  Bluefruit.getAddr(addr);
  Serial.print("Address: ");
  Serial.printBufferReverse(addr, 6, ':');
  Serial.println();
}

bool isMacAddressACatsfish(uint8_t addr[6]){
//  Serial.print("isMacAddressACatsfish: ");
//  Serial.printBufferReverse(addr, 6, ':');
//  Serial.println();
  for(int i = 0; i < ALL_ADDRESSES_LENGTH; i++){
    if (memcmp(allAddresses[i], addr, 6) == 0){
      return true;
    }
  }
  return false;
}
