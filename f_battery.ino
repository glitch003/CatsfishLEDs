void set_low_battery_led(bool on_or_off){
  if (on_or_off){
    digitalWrite(RED_LED_PIN, LED_ON);
  } else {
    digitalWrite(RED_LED_PIN, LED_OFF);
  }
}


float readVBAT(void) {
  float raw;

  // Set the analog reference to 3.0V (default = 3.6V)
  analogReference(AR_INTERNAL_3_0);

  // Set the resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  // Let the ADC settle
  delay(1);

  // Get the raw 12-bit, 0..3000mV ADC value
  raw = analogRead(vbat_pin);

  // Set the ADC back to the default settings
  analogReference(AR_DEFAULT);
  analogReadResolution(10);

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3000mV and resolution is 12-bit (0..4095)
  return raw * REAL_VBAT_MV_PER_LSB;
}

uint8_t mvToPercent(float mvolts) {
  if(mvolts<3300)
    return 0;

  if(mvolts <3600) {
    mvolts -= 3300;
    return mvolts/30;
  }

  mvolts -= 3600;
  return 10 + (mvolts * 0.15F );  // thats mvolts /6.66666666
}


void displayBatteryLevel(){
  turnOffAll();
  neopixel.setBrightness(brightness);

  // Get a raw ADC reading
  float vbat_mv = readVBAT();

  // Convert from raw mv to percentage (based on LIPO chemistry)
  uint8_t vbat_per = mvToPercent(vbat_mv);

  int ledLevel = STRIP_LED_COUNT * (vbat_per / 100.0F);
  uint32_t pixelColor = neopixel.Color(0,255,0);
  if (vbat_per < LOW_BATTERY_PERCENTAGE){
    pixelColor = neopixel.Color(0,255,0);
  } else if(vbat_per < 60){
    pixelColor = neopixel.Color(255,255,0);
  }

  for(int i = 0; i < ledLevel; i++){
    neopixel.setPixelColor(i,pixelColor);
    delay(100);
    neopixel.show();
  }
  delay(2000);
  for(int i = ledLevel - 1; i >= 0; i--){
    neopixel.setPixelColor(i,0,0,0);
    delay(100);
    neopixel.show();
  }
  
}
