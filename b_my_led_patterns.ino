void displayProximityCountHeartbeat(int frame){
  // turn off all
  for(int i = 0; i < STRIP_LED_COUNT; i++){
    neopixel.setPixelColor(i, neopixel.Color(0,0,0));
  }

  int pixelOnFrames = 20;
  
  if (frame < pixelOnFrames){
    // first beat
    int inRange = countDevicesInRange();
    for(int i = 0; i < inRange; i++){
      // set ledIndex based on how many are in range
      int ledIndex = (STRIP_LED_COUNT / inRange) * i;
//      // modify that based on the frame to make it spin
//      ledIndex = (ledIndex + (loopCycles/200)) % STRIP_LED_COUNT;
      
      neopixel.setPixelColor(ledIndex, neopixel.gamma32(neopixel.ColorHSV(20000)));
    }
  }else{
    // second beat

    int8_t rssiForDevices[128];

    // count how many devices we've seen recently and collect their rssi's
    int inRange = 0;
    for(int i = 0; i < seenDevicesCount; i++){
      SeenDevice s = seenDevices[i];
      if ((millis() - s.lastSeenAt) < PROXIMITY_TIMEOUT + (frame * NEOPIXEL_TIMER_SPEED)){ // timeout.  compensate for time since the first heartbeat.
        rssiForDevices[inRange] = s.rssi;
        inRange++;
      }
    }

    // count how many devices we've seen recently and collect their rssi's
    for(int i = 0; i < inRange; i++){
      if (frame > rssiToNextHeartbeatFrame(rssiForDevices[i]) + pixelOnFrames && frame < rssiToNextHeartbeatFrame(rssiForDevices[i]) + (pixelOnFrames * 2)){
        int ledIndex = (STRIP_LED_COUNT / inRange) * i;
        neopixel.setPixelColor(ledIndex, neopixel.gamma32(neopixel.ColorHSV(20000)));
      }
    }

    
  }
  neopixel.show();
}


void displayProximityCountWithRssiBrightnessSingleColor(){  
  // turn off all
  for(int i = 0; i < STRIP_LED_COUNT; i++){
    neopixel.setPixelColor(i, neopixel.Color(0,0,0));
  }

  int rssiForDevices[128];

  // count how many devices we've seen recently and collect their rssi's
  int inRange = 0;
  for(int i = 0; i < seenDevicesCount; i++){
    SeenDevice s = seenDevices[i];
    if ((millis() - s.lastSeenAt) < PROXIMITY_TIMEOUT){ // timeout
      rssiForDevices[inRange] = s.rssi;
      inRange++;
    }
  }


  long firstPixelHue = 10 * 256;

  for(int i = 0; i < inRange; i++){
    // use this if you want different colors for each led
//    int pixelHue = firstPixelHue + (i * 65536L / inRange);
    
    int pixelHue = firstPixelHue;

    // set ledIndex based on how many are in range
    int ledIndex = (STRIP_LED_COUNT / inRange) * i;
    // modify that based on the frame to make it spin
    ledIndex = (ledIndex + (loopCycles/200)) % STRIP_LED_COUNT;

    int brightnessValue = rssiToBrightness(rssiForDevices[i]);
    
    neopixel.setPixelColor(ledIndex, neopixel.gamma32(neopixel.ColorHSV(pixelHue, 255, brightnessValue)));
  }
  neopixel.show();
}


void displayProximityCountWithRssiBrightness(int frame){  
  // turn off all
  for(int i = 0; i < STRIP_LED_COUNT; i++){
    neopixel.setPixelColor(i, neopixel.Color(0,0,0));
  }

  int rssiForDevices[128];

  // count how many devices we've seen recently and collect their rssi's
  int inRange = 0;
  for(int i = 0; i < seenDevicesCount; i++){
    SeenDevice s = seenDevices[i];
    if ((millis() - s.lastSeenAt) < PROXIMITY_TIMEOUT){ // timeout
      rssiForDevices[inRange] = s.rssi;
      inRange++;
    }
  }


  long firstPixelHue = frame * 256;

  for(int i = 0; i < inRange; i++){
    // use this if you want different colors for each led
//    int pixelHue = firstPixelHue + (i * 65536L / inRange);
    
    int pixelHue = firstPixelHue;

    // set ledIndex based on how many are in range
    int ledIndex = (STRIP_LED_COUNT / inRange) * i;
    // modify that based on the frame to make it spin
    ledIndex = (ledIndex + (loopCycles/200)) % STRIP_LED_COUNT;

    int brightnessValue = rssiToBrightness(rssiForDevices[i]);
    
    neopixel.setPixelColor(ledIndex, neopixel.gamma32(neopixel.ColorHSV(pixelHue, 255, brightnessValue)));
  }
  neopixel.show();
}


void displayProximityCount(int inRange, int frame){  
  // turn off all
  for(int i = 0; i < STRIP_LED_COUNT; i++){
    neopixel.setPixelColor(i, neopixel.Color(0,0,0));
  }


  long firstPixelHue = frame * 256;

  for(int i = 0; i < inRange; i++){
    // use this if you want different colors for each led
//    int pixelHue = firstPixelHue + (i * 65536L / inRange);
    
    int pixelHue = firstPixelHue;

    // set ledIndex based on how many are in range
    int ledIndex = (STRIP_LED_COUNT / inRange) * i;
    // modify that based on the frame to make it spin
    ledIndex = (ledIndex + (loopCycles/200)) % STRIP_LED_COUNT;
    
    neopixel.setPixelColor(ledIndex, neopixel.gamma32(neopixel.ColorHSV(pixelHue)));
  }
  neopixel.show();
}

void displayIdleRainbow(int frame){
  long firstPixelHue = frame * 256;
  for(int i=0; i<STRIP_LED_COUNT; i++) { // For each pixel in strip...
    // Offset pixel hue by an amount to make one full revolution of the
    // color wheel (range of 65536) along the length of the strip
    // (strip.numPixels() steps):
    int pixelHue = firstPixelHue + (i * 65536L / STRIP_LED_COUNT);
    // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
    // optionally add saturation and value (brightness) (each 0 to 255).
    // Here we're using just the single-argument hue variant. The result
    // is passed through strip.gamma32() to provide 'truer' colors
    // before assigning to each pixel:
    neopixel.setPixelColor(i, neopixel.gamma32(neopixel.ColorHSV(pixelHue)));
  }
  neopixel.show(); // Update strip with new contents
}

void displayRangeTestRainbow(int frame){
  long firstPixelHue = frame * 256;
  for(int i=0; i<ledsToShowBasedOnRssi; i++) { // For each pixel in strip...
    // Offset pixel hue by an amount to make one full revolution of the
    // color wheel (range of 65536) along the length of the strip
    // (strip.numPixels() steps):
    int pixelHue = firstPixelHue + (i * 65536L / ledsToShowBasedOnRssi);
    // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
    // optionally add saturation and value (brightness) (each 0 to 255).
    // Here we're using just the single-argument hue variant. The result
    // is passed through strip.gamma32() to provide 'truer' colors
    // before assigning to each pixel:
    neopixel.setPixelColor(i, neopixel.gamma32(neopixel.ColorHSV(pixelHue)));
  }

  // turn off rest
  for(int i = ledsToShowBasedOnRssi; i < STRIP_LED_COUNT; i++){
    neopixel.setPixelColor(i, neopixel.Color(0,0,0));
  }

  neopixel.show(); // Update strip with new contents
}


void turnOffAll(){
  for(int i = 0; i < STRIP_LED_COUNT; i++){
    neopixel.setPixelColor(i, neopixel.Color(0,0,0));
  }
  neopixel.show();
}






void loopThroughAllPatterns(int ledMode){
    if (ledMode == 0) {one_color_all(0,0,0);}                 //---STRIP OFF - "0"
  else if (ledMode == 1) {one_color_all(255,255,255);}      //---STRIP SOLID WHITE
  else if (ledMode == 2) {rainbow_fade(20);}                //---STRIP RAINBOW FADE
  else if (ledMode == 3) {rainbow_loop(10, 20);}            //---RAINBOW LOOP
  else if (ledMode == 4) {random_burst(20);}                //---RANDOM
  else if (ledMode == 5) {color_bounce(20);}                //---CYLON v1
  else if (ledMode == 6) {color_bounceFADE(20);}            //---CYLON v2
  else if (ledMode == 7) {police_lightsONE(40);}            //---POLICE SINGLE
  else if (ledMode == 8) {police_lightsALL(40);}            //---POLICE SOLID
  else if (ledMode == 9) {flicker(200,255);}                //---STRIP FLICKER 
  else if (ledMode == 10) {pulse_one_color_all(0, 10);}     //--- PULSE COLOR BRIGHTNESS
  else if (ledMode == 11) {pulse_one_color_all_rev(0, 10);} //--- PULSE COLOR SATURATION   
  else if (ledMode == 12) {fade_vertical(240, 60);}         //--- VERTICAL SOMETHING
//  if (ledMode == 13) {rule30(100);}                       //--- CELL AUTO - RULE 30 (RED)
  else if (ledMode == 14) {random_march(100);}              //--- MARCH RANDOM COLORS
  else if (ledMode == 15) {rwb_march(100);}                 //--- MARCH RWB COLORS
  else if (ledMode == 16) {radiation(120, 60);}             //--- RADIATION SYMBOL (OR SOME APPROXIMATION)
  else if (ledMode == 17) {color_loop_vardelay();}          //--- VARIABLE DELAY LOOP
  else if (ledMode == 18) {white_temps();}                  //--- WHITE TEMPERATURES
  else if (ledMode == 19) {sin_bright_wave(240, 35);}       //--- SIN WAVE BRIGHTNESS
  else if (ledMode == 20) {pop_horizontal(300, 100);}       //--- POP LEFT/RIGHT
  else if (ledMode == 21) {quad_bright_curve(240, 100);}    //--- QUADRATIC BRIGHTNESS CURVE  
  else if (ledMode == 22) {flame();}                        //--- FLAME-ISH EFFECT
  else if (ledMode == 23) {rainbow_vertical(10, 20);}       //--- VERITCAL RAINBOW
  else if (ledMode == 24) {pacman(100);}                    //--- PACMAN
  else if (ledMode == 25) {color_loop_randdelay();}         //--- VARIABLE DELAY LOOP
  else if (ledMode == 26) {swiss_cross(100);}               //--- SWISS CROSS
  else if (ledMode == 27) {germany(100);}                   //--- GERMANY FLAG

}
