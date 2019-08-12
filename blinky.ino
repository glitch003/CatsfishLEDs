/* BLE Example for SparkFun Pro nRF52840 Mini
 *
 *  This example demonstrates how to use the Bluefruit
 *  library to both send and receive data to the
 *  nRF52840 via BLE.
 *
 *  Using a BLE development app like Nordic's nRF Connect
 *  https://www.nordicsemi.com/eng/Products/Nordic-mobile-Apps/nRF-Connect-for-Mobile
 *  The BLE UART service can be written to to turn the
 *  on-board LED on/off, or read from to monitor the
 *  status of the button.
 *
 *  See the tutorial for more information:
 *  https://learn.sparkfun.com/tutorials/nrf52840-development-with-arduino-and-circuitpython#arduino-examples
*/
#include <Arduino.h>
#include <bluefruit.h>
#include <Adafruit_NeoPixel.h>

//#include <FastLED.h>


//BLEUart bleuart; // uart over ble

// master external antenna switch.  comment this out to use internal antenna.
//#define EXTERNAL_ANTENNA

// master XENON flag.  turn this off if not using particle XENON
#define XENON


#ifdef XENON
  // pin 15 for xenon
  const int LED_PIN = 14;
  #define LED_OFF HIGH
  #define LED_ON LOW
  #define XENON_BLUE_LED_PIN 44
#else
  // pin 7 for sparkfun
  const int LED_PIN = 7;
  #define LED_OFF LOW
  #define LED_ON HIGH
#endif

const int RED_LED_PIN = 13;




// battery stuff
uint32_t vbat_pin = 5;             // A7 for feather nRF52832, A6 for nRF52840
#define VBAT_MV_PER_LSB   (0.73242188F)   // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096

#define VBAT_DIVIDER      (0.71275837F)   // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.403F)        // Compensation factor for the VBAT divider

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)




#define NEOPIXEL_VERSION_STRING "Neopixel v2.0"

#ifdef XENON
  #define STRIP_PIN                     33   /* pin D2 on the Xenon */
#else
  #define STRIP_PIN                     19   /* pin 19 on the sparkfun board */
#endif


#define STRIP_LED_COUNT 16
#define FRAME_COUNT 1280
uint8_t *pixelBuffer = NULL;
uint8_t brightness = 10;

unsigned long loopCycles = 0;

unsigned long deviceLastSeen = 0;

uint8_t ledsToShowBasedOnRssi = STRIP_LED_COUNT;

//CRGB leds[STRIP_LED_COUNT];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns


Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(STRIP_LED_COUNT, STRIP_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


#define BUTTON_PIN 11
volatile int buttonState = 0;
volatile byte ledsOn = HIGH;
volatile int lastButtonState = LOW;


// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
volatile unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
const unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
volatile unsigned long lastPressTime = 0;
const unsigned long maxTimeInbetweenMultipleClicks = 1000;
volatile int pressCount = 0;

SoftwareTimer neopixelTimer, batteryCheckTimer;

void setup() {
  // Initialize hardware:
  Serial.begin(115200); // Serial is the USB serial port

#ifdef XENON
  // the particle xenon has 2 user controllable LEDS - one RGB and one Blue.  We kill the blue here.
  pinMode(XENON_BLUE_LED_PIN, OUTPUT);
//  digitalWrite(XENON_BLUE_LED_PIN, LOW);
#endif

  // antenna stuff
  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);
  #ifdef EXTERNAL_ANTENNA
    digitalWrite(24,LOW);
    digitalWrite(25,HIGH);
  #else
    digitalWrite(24,HIGH);
    digitalWrite(25,LOW);
  #endif


  pinMode(LED_PIN, OUTPUT); // Turn on-board LED off
  digitalWrite(LED_PIN, LED_OFF);


  // button interrupt
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressed, ISR_DEFERRED | CHANGE);

//  while(!Serial) delay(10);

   // Config Neopixels
   neopixel.begin();
   pixelBuffer = new uint8_t[STRIP_LED_COUNT*3];
   neopixel.setBrightness(brightness);

//  FastLED.addLeds<NEOPIXEL, STRIP_PIN>(leds, STRIP_LED_COUNT).setCorrection(TypicalSMD5050);
//  FastLED.setBrightness(BRIGHTNESS);


  initBluetooth();


  // Get a single ADC sample and throw it away
  readVBAT();

  // turn off all leds
  turnOffAll();

  // create timers
  neopixelTimer.begin(10, neopixelTimerCallback);

  batteryCheckTimer.begin(15000, batteryCheckCallback);
  batteryCheckTimer.start();

  exitSleepMode();

  suspendLoop();
}

void loop(){}

//void loop() {
//  if (ledsOn){
//    rainbow_loop(10, 20);
//  } else {
//    turnOffAll();
//    delay(100);
//  }
//}

//void loop() {
//  loopCycles++;
//  int frame = loopCycles % FRAME_COUNT;
//
//  if (loopCycles % 1000 == 0) {
//    printTemp();
//    printBatteryLevel();
//  }
//
////  if (frame == 0) { loopCycles = -1; }
//
////  // If data has come in via BLE:
////  if (bleuart.available()) {
////    uint8_t c;
////    // use bleuart.read() to read a character sent over BLE
////    c = (uint8_t) bleuart.read();
////    // Print out the character for debug purposes:
////    Serial.write(c);
////
////    // If the character is one of our expected values,
////    // do something:
////    switch (c) {
////      // 0 number or character, turn the LED off:
////      case 0:
////      case '0':
////        digitalWrite(LED_PIN, LED_OFF);
////        break;
////      // 1 number or character, turn the LED on:
////      case 1:
////      case '1':
////        digitalWrite(LED_PIN, LED_ON);
////        break;
////      default:
////        break;
////    }
////  }
//
//  //checkButtonState();
//
//  if (millis() - deviceLastSeen < 1000 && ledsOn == HIGH){
//    digitalWrite(LED_PIN, LED_ON);
////    Serial.println("yes");
//    showInRange(frame);
//  } else {
//    digitalWrite(LED_PIN, LED_OFF);
////    Serial.println("no");
//    turnOffAll();
//  }
//
//  delay(10);
//}

void initBluetooth(){
  // Uncomment the code below to disable sharing
  // the connection LED on pin 7.
  Bluefruit.autoConnLed(false);

  // Initialize Bluetooth:
  Bluefruit.begin();

  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4, 8
  Bluefruit.setTxPower(8);
  Bluefruit.setName("Catsfish");
//  bleuart.begin();

  // Start advertising device and bleuart services
//  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
//  Bluefruit.Advertising.addTxPower();
//  Bluefruit.Advertising.addService(bleuart);


//  // set bluetooth 5 phy to coded long range mode
  bool phyWasSet = Bluefruit.Advertising.setPhy(BLE_GAP_PHY_CODED);
  if(phyWasSet){
    Serial.println("phy was set!!");
  }else{
    Serial.println("phy was not set");
  }

//  Bluefruit.ScanResponse.addName();


  Bluefruit.Advertising.restartOnDisconnect(true);


  // adafruit lib default
//  // Set advertising interval (in unit of 0.625ms):
//  Bluefruit.Advertising.setInterval(32, 244);
//  // number of seconds in fast mode:
//  Bluefruit.Advertising.setFastTimeout(30);

  // use this if you want really fast advertising
   // Set advertising interval (in unit of 0.625ms):
  Bluefruit.Advertising.setInterval(32, 32);
  // apple intervals are 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms - remember to divide by 0.625!
  // number of seconds in fast mode:
  Bluefruit.Advertising.setFastTimeout(0); // stay in fast mode forever


  // use this for less power usage
//  /// Set advertising interval (in unit of 0.625ms):
//  Bluefruit.Advertising.setInterval(338, 338);
//  // apple intervals are 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms - remember to divide by 0.625!
//  // number of seconds in fast mode:
//  Bluefruit.Advertising.setFastTimeout(0); // stay in fast mode forever










  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Filter for devices with a min RSSI of -80 dBm
   * - Interval = 100 ms, window = 50 ms
   * - Use active scan (requests the optional scan response packet)
   * - Start(0) = will scan forever since no timeout is given
   */
  Bluefruit.Scanner.setPhy(BLE_GAP_PHY_CODED);
  Bluefruit.Scanner.setRxCallback(basic_scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
////  Bluefruit.Scanner.filterRssi(-60);            // Only invoke callback when RSSI >= -80 dBm

// adafruit lib default
//  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms

// use this if you want really fast scanning
// apple settings for foreground scanning.  scan interval of 40ms and scan window of 30ms
//  Bluefruit.Scanner.setInterval(64, 48); // in units of 0.625 ms

// use this for less power usage
// apple settings for background scanning.  300ms scanInterval and 30ms scanWindow
  Bluefruit.Scanner.setInterval(480, 48); // in units of 0.625 ms


//  Bluefruit.Scanner.useActiveScan(true);        // Request scan response data
}

void neopixelTimerCallback(TimerHandle_t _handle){
  loopCycles++;
  int frame = loopCycles % FRAME_COUNT;

  neopixel.setBrightness(brightness);
  showInRange(frame);
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

  if (vbat_per <= 25){
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



// void checkButtonState(){
//   // read the state of the pushbutton value:
//   int reading = digitalRead(BUTTON_PIN);

//   // If the switch changed, due to noise or pressing:
//   if (reading != lastButtonState) {
//     // reset the debouncing timer
//     lastDebounceTime = millis();
//   }

//   if ((millis() - lastDebounceTime) > debounceDelay) {
//     // whatever the reading is at, it's been there for longer
//     // than the debounce delay, so take it as the actual current state:
// //    Serial.println("((millis() - lastDebounceTime) > debounceDelay)");
//     // if the button state has changed:
//     if (reading != buttonState) {
// //      Serial.println("(reading != buttonState)");
//       buttonState = reading;

//       // only toggle the LED if the new button state is HIGH
//       if (buttonState == HIGH) {
// //        Serial.println("(buttonState == HIGH)");
//         ledsOn = !ledsOn;
//       }
//     }
//   }

//   // save the reading.  Next time through the loop,
//   // it'll be the lastButtonState:
//   lastButtonState = reading;
// }

uint8_t rssiToLedCount(int8_t rssi){
  return STRIP_LED_COUNT;
  // rssi range is about -33 to -100
  int8_t maxSignal = -40;
  int8_t minSignal = -100;

  if (rssi > maxSignal){
    return STRIP_LED_COUNT;
  }

  int range = -1 * (maxSignal - minSignal);
  rssi = rssi - maxSignal; // make rssi start at 0 and go to


  // non-float calculations
//  Serial.print("non inverted percent: ");
//  Serial.println((int)rssi * 100 / range);
//
//  int percent = 100 - ((int)rssi * 100 / range);
//
//  Serial.print("final inverted percent: ");
//  Serial.println(percent);
//
//
//
//  uint8_t finalValue = (uint8_t)(STRIP_LED_COUNT*1000/(100000 / percent));
//
//  Serial.print("final value of led count: ");
//  Serial.println(finalValue);
//
//    if (rssi < minSignal || finalValue == 0) {
//      return 1; // never show 0 LEDs.  the device is in range, it should always show at least 1.
//    }
//
//  return finalValue;


  // same calculations but using floats.
//  Serial.print("non inverted percent: ");
//  Serial.println((float)rssi / (float)range);

  float percent = 1 - ((float)rssi / (float)range);

//  Serial.print("final inverted percent: ");
//  Serial.println(percent);



  uint8_t finalValue = (uint8_t)(round(percent * (float)STRIP_LED_COUNT));

//  Serial.print("final value of led count: ");
//  Serial.println(finalValue);

    if (rssi < minSignal || finalValue == 0) {
      return 1; // never show 0 LEDs.  the device is in range, it should always show at least 1.
    }

  return finalValue;


}

void showInRange(int frame){
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


void basic_scan_callback(ble_gap_evt_adv_report_t* report)
{
  PRINT_LOCATION();
  uint8_t len = 0;
  uint8_t buffer[32];
  memset(buffer, 0, sizeof(buffer));

  Serial.print("Device ");
  Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
  Serial.print(" seen at ");
  Serial.print(millis() / 1000);
  Serial.print(" seconds with rssi ");
  Serial.printf("%14s %d dBm\n", "RSSI", report->rssi);

  deviceLastSeen = millis();
  ledsToShowBasedOnRssi = rssiToLedCount(report->rssi);

//  /* Complete Local Name */
//  if(Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, buffer, sizeof(buffer)))
//  {
//    String s = String((char*)buffer);
//
//    Serial.printf("%14s %d dBm\n", "RSSI", report->rssi);
//    Serial.printf("%14s %s\n", "COMPLETE NAME", buffer);
//    Serial.println();
//
//
//    if (s.startsWith("Catsfish")){
//      Serial.println("Device is in range");
////      Serial.printf("%14s %d dBm\n", "RSSI", report->rssi);
////      Serial.printf("%14s %s\n", "COMPLETE NAME", buffer);
////      Serial.println();
//
////      if (report->rssi > -60){
//        deviceLastSeen = millis();
////      }
//    }
//    memset(buffer, 0, sizeof(buffer));
//
//
//  }



  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  Bluefruit.Scanner.resume();
}


// print chip temp
void printTemp() {
  int32_t temperature;                                        // variable to hold temp reading
  sd_temp_get(&temperature);                            // get new temperature
  int16_t temp_new = (int16_t) temperature;                   // convert from int32_t to int16_t
  uint8_t integer = (uint8_t)((temp_new >> 2) & 0xFFUL); // Right-shift by two to remove decimal part
  uint8_t decimal = (uint8_t)((temp_new << 6) & 0xFFUL); // Left-shift 6 to get fractional part with 0.25 degrees C resolution

  Serial.print("temp = ");
  Serial.print(integer);
  Serial.print(".");
  Serial.print(decimal);
  Serial.println(" degrees");
}




// this prints everything that is scanned
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  PRINT_LOCATION();
  uint8_t len = 0;
  uint8_t buffer[32];
  memset(buffer, 0, sizeof(buffer));

  deviceLastSeen = millis();

  /* Display the timestamp and device address */
  if (report->type.scan_response)
  {
    Serial.printf("[SR%10d] Packet received from ", millis());
  }
  else
  {
    Serial.printf("[ADV%9d] Packet received from ", millis());
  }
  // MAC is in little endian --> print reverse
  Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
  Serial.print("\n");

  /* Raw buffer contents */
  Serial.printf("%14s %d bytes\n", "PAYLOAD", report->data.len);
  if (report->data.len)
  {
    Serial.printf("%15s", " ");
    Serial.printBuffer(report->data.p_data, report->data.len, '-');
    Serial.println();
  }

  /* RSSI value */
  Serial.printf("%14s %d dBm\n", "RSSI", report->rssi);

  /* Adv Type */
  Serial.printf("%14s ", "ADV TYPE");
  if ( report->type.connectable )
  {
    Serial.print("Connectable ");
  }else
  {
    Serial.print("Non-connectable ");
  }

  if ( report->type.directed )
  {
    Serial.println("directed");
  }else
  {
    Serial.println("undirected");
  }

  /* Shortened Local Name */
  if(Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, buffer, sizeof(buffer)))
  {
    Serial.printf("%14s %s\n", "SHORT NAME", buffer);
    memset(buffer, 0, sizeof(buffer));
  }

  /* Complete Local Name */
  if(Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, buffer, sizeof(buffer)))
  {
    Serial.printf("%14s %s\n", "COMPLETE NAME", buffer);
    memset(buffer, 0, sizeof(buffer));
  }

  /* TX Power Level */
  if (Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_TX_POWER_LEVEL, buffer, sizeof(buffer)))
  {
    Serial.printf("%14s %i\n", "TX PWR LEVEL", buffer[0]);
    memset(buffer, 0, sizeof(buffer));
  }

  /* Check for UUID16 Complete List */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE, buffer, sizeof(buffer));
  if ( len )
  {
    printUuid16List(buffer, len);
  }

  /* Check for UUID16 More Available List */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE, buffer, sizeof(buffer));
  if ( len )
  {
    printUuid16List(buffer, len);
  }

  /* Check for UUID128 Complete List */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE, buffer, sizeof(buffer));
  if ( len )
  {
    printUuid128List(buffer, len);
  }

  /* Check for UUID128 More Available List */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE, buffer, sizeof(buffer));
  if ( len )
  {
    printUuid128List(buffer, len);
  }

  /* Check for BLE UART UUID */
  if ( Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE) )
  {
    Serial.printf("%14s %s\n", "BLE UART", "UUID Found!");
  }

  /* Check for DIS UUID */
  if ( Bluefruit.Scanner.checkReportForUuid(report, UUID16_SVC_DEVICE_INFORMATION) )
  {
    Serial.printf("%14s %s\n", "DIS", "UUID Found!");
  }

  /* Check for Manufacturer Specific Data */
  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buffer, sizeof(buffer));
  if (len)
  {
    Serial.printf("%14s ", "MAN SPEC DATA");
    Serial.printBuffer(buffer, len, '-');
    Serial.println();
    memset(buffer, 0, sizeof(buffer));
  }

  Serial.println();

  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  Bluefruit.Scanner.resume();
}

void printUuid16List(uint8_t* buffer, uint8_t len)
{
  Serial.printf("%14s %s", "16-Bit UUID");
  for(int i=0; i<len; i+=2)
  {
    uint16_t uuid16;
    memcpy(&uuid16, buffer+i, 2);
    Serial.printf("%04X ", uuid16);
  }
  Serial.println();
}

void printUuid128List(uint8_t* buffer, uint8_t len)
{
  (void) len;
  Serial.printf("%14s %s", "128-Bit UUID");

  // Print reversed order
  for(int i=0; i<16; i++)
  {
    const char* fm = (i==4 || i==6 || i==8 || i==10) ? "-%02X" : "%02X";
    Serial.printf(fm, buffer[15-i]);
  }

  Serial.println();
}







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
