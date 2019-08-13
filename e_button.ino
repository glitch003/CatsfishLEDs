
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
        // button was pressed down
        if ((millis() - lastPressTime) < maxTimeInbetweenMultipleClicks){
          pressCount++;
        } else {
          pressCount = 1;
        }

        lastPressTime = millis();
      } else {
        // button was released
        if (mode != 0 && pressCount == 1){
          // turn off if we are doing anything and the user presses the button
          enterSleepMode();
          mode = 0;
        } else {
          // wake up!
          digitalWrite(XENON_BLUE_LED_PIN, HIGH);
          setModeBasedOnNumberOfPresses(pressCount);
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
