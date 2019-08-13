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

  // turn off rest
  for(int i = ledsToShowBasedOnRssi; i < STRIP_LED_COUNT; i++){
    neopixel.setPixelColor(i, neopixel.Color(0,0,0));
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
