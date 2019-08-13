


void neopixelTimerCallback(TimerHandle_t _handle){
  loopCycles++;
  int frame = loopCycles % FRAME_COUNT;

  neopixel.setBrightness(brightness);
  if (mode == 1){
    // proximity mode
//    displayProximityCount(1, frame);
    displayIdleRainbow(frame);
  } else if (mode == 3){
    // range test mode
    displayRangeTestRainbow(frame);
  }
}

void batteryCheckCallback(TimerHandle_t _handle){
  ada_callback_fromISR(NULL, 0, batteryCheckISRCallback); // Queue up a task with no extra variables and no arguments.
}

void batteryCheckISRCallback(){
  // Get a raw ADC reading
  float vbat_mv = readVBAT();

  // Convert from raw mv to percentage (based on LIPO chemistry)
  uint8_t vbat_per = mvToPercent(vbat_mv);

  // Display the results
  Serial.print("LIPO = ");
  Serial.print(vbat_mv);
  Serial.print(" mV (");
  Serial.print(vbat_per);
  Serial.println("%)");

  if (vbat_per <= LOW_BATTERY_PERCENTAGE){
    int flash_delay = vbat_per * 40; // if it's 25 percent, this is a 1000 second delay between flashes because 25 * 40 = 1000.
    int flash_count = vbat_per < 10 ? 10 : 5;
    for(int i = 0; i < flash_count; i++){
      set_low_battery_led(true);
      delay(25);
      set_low_battery_led(false);
      delay(flash_delay);
    }
  } else {
    set_low_battery_led(false);
  }

  printTemp();
}
