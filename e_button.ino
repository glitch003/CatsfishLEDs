
void buttonPressed(){
//  Serial.println("buttonPressed()");
  // read the state of the pushbutton value:
  int reading = digitalRead(BUTTON_PIN);
//  Serial.print("reading: ");
//  Serial.println(reading);

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
//    Serial.println("((millis() - lastDebounceTime) > debounceDelay)");
    // if the button state has changed:
    if (reading != buttonState) {
//      Serial.println("(reading != buttonState)");
      buttonState = reading;

      if (buttonState == LOW) {
//         volatile unsigned long lastPressTime = 0;
// const unsigned long maxTimeInbetweenMultipleClicks = 1000;
// volatile int pressCount = 0;
        if ((millis() - lastPressTime) < maxTimeInbetweenMultipleClicks){
          pressCount++;
        } else {
          pressCount = 1;
        }

        lastPressTime = millis();
      } else {
        if (pressCount == 1) {
          ledsOn = !ledsOn;
          digitalWrite(XENON_BLUE_LED_PIN, ledsOn);

          if (ledsOn){
            exitSleepMode();
          } else {
            enterSleepMode();
          }
        } else if (pressCount == 2) {
          // if LEDS are on, headlamp mode will not work, because the LEDs will be brighter.  so turn them off.
          if (ledsOn){
            enterSleepMode();
            ledsOn = LOW;
            digitalWrite(XENON_BLUE_LED_PIN, ledsOn);
          }

          enterHeadlampMode();
        }
      }
    }
  }



  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

   // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;


}
