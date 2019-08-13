

void setModeBasedOnNumberOfPresses(int pressCount){
  if (pressCount == 1) {
    mode = 1;
    enterProximityMode();
  } else if (pressCount == 2) {
    mode = 2;
    exitProximityMode();
    enterHeadlampMode();
  }

}


void enterSleepMode(){
  Serial.println("enterSleepMode()");
  
  neopixelTimer.stop();
  turnOffAll();
  
  digitalWrite(XENON_BLUE_LED_PIN, LOW);

  Bluefruit.Scanner.stop();

  Bluefruit.Advertising.stop();
}

void enterProximityMode(){
  Serial.println("exitSleepMode()");
  
  neopixelTimer.start();

  // start scanning
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

  // // start advertising
  Bluefruit.Advertising.start(0);
}

void exitProximityMode(){
  neopixelTimer.stop();

  // start scanning
  Bluefruit.Scanner.stop();                   // 0 = Don't stop scanning after n seconds

  // // start advertising
  Bluefruit.Advertising.stop();
}

void enterHeadlampMode(){  
  turnOffAll();
  neopixel.setBrightness(100);
  for(int i = 0; i < STRIP_LED_COUNT; i ++){
    neopixel.setPixelColor(i, neopixel.Color(255,255,255));
  }
  neopixel.show();
}
