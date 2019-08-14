int countDevicesInRange(){
  // count how many devices we've seen recently
  int devicesInRange = 0;
  for(int i = 0; i < seenDevicesCount; i++){
    SeenDevice s = seenDevices[i];
    if ((millis() - s.lastSeenAt) < 5000){ // timeout
      devicesInRange++;
    }
  }
  return devicesInRange;
}
