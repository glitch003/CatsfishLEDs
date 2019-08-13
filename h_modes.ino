

void enterSleepMode(){
  Serial.println("enterSleepMode()");
  
  neopixelTimer.stop();
  turnOffAll();

  Bluefruit.Scanner.stop();

  Bluefruit.Advertising.stop();
}

void exitSleepMode(){
  Serial.println("exitSleepMode()");
  
  neopixelTimer.start();

  // start scanning
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

  // // start advertising
  Bluefruit.Advertising.start(0);
}

void enterHeadlampMode(){
  neopixel.setBrightness(255);
  for(int i = 0; i < STRIP_LED_COUNT; i ++){
    neopixel.setPixelColor(i, neopixel.Color(255,255,255));
  }
  neopixel.show();
}

void exitHeadlampMode(){
  turnOffAll();
}
