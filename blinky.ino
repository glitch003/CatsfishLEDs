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

#define DEBUG_LED_PATTERNS
#define DEBUG_BLE
#define DEBUG_BAT_AND_TEMP

#define CATSFISH_VERSION_STRING "CATSFISH v1.0"

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

#define LOW_BATTERY_PERCENTAGE 20




#define NEOPIXEL_VERSION_STRING "Neopixel v2.0"

#ifdef XENON
  #define STRIP_PIN                     33   /* pin D2 on the Xenon */
#else
  #define STRIP_PIN                     19   /* pin 19 on the sparkfun board */
#endif


#define STRIP_LED_COUNT 16
#define FRAME_COUNT 1280
uint8_t *pixelBuffer = NULL;
uint8_t brightness = 100;

unsigned long loopCycles = 0;

unsigned long deviceLastSeen = 0;
unsigned long devicesInRange = 0;

struct SeenDevice {
  unsigned long lastSeenAt;
  int8_t rssi;
  uint8_t addr[6];
};

SeenDevice seenDevices[128];
uint8_t seenDevicesCount = 0;
#define PROXIMITY_TIMEOUT 5000

uint8_t ledsToShowBasedOnRssi = 0;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

int mode = 1;
// modes are:
// 0 = off
// 1 = proximity mode
// 2 = headlamp mode
// 3 = range test mode (disbled for now)

// more mode ideas - 
// super bright mode (for daytime)
// darker mode (chillaxing)


Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(STRIP_LED_COUNT, STRIP_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

#define NEOPIXEL_TIMER_SPEED 500


#define BUTTON_PIN 11
volatile int buttonState = 1;
volatile int lastButtonState = HIGH;


// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
volatile unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
const unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
volatile unsigned long lastPressTime = 0;
const unsigned long maxTimeInbetweenMultipleClicks = 2000;
volatile int pressCount = 0;

SoftwareTimer neopixelTimer, batteryCheckTimer;

#define ALL_ADDRESSES_LENGTH 32
uint8_t allAddresses[ALL_ADDRESSES_LENGTH][6] = {{78, 217, 29, 122, 65, 254, }, {157, 17, 106, 128, 197, 249, }, {33, 245, 240, 96, 82, 225, }, {182, 222, 19, 122, 133, 251, }, {131, 102, 142, 100, 127, 195, }, {10, 75, 68, 187, 141, 252, }, {189, 75, 5, 243, 35, 246, }, {63, 229, 53, 97, 191, 224, }, {218, 53, 29, 220, 208, 193, }, {180, 11, 24, 157, 76, 216, }, {66, 36, 181, 165, 240, 221, }, {92, 151, 168, 197, 32, 226, }, {94, 206, 72, 192, 112, 221, }, {172, 139, 121, 241, 235, 233, }, {37, 218, 52, 80, 125, 207, }, {218, 70, 135, 96, 40, 211, }, {63, 140, 70, 99, 103, 228, }, {41, 113, 1, 161, 254, 198, }, {178, 128, 192, 156, 58, 211, }, {139, 100, 22, 160, 163, 245, }, {95, 120, 176, 203, 119, 237, }, {28, 183, 110, 3, 151, 209, }, {3, 226, 173, 34, 46, 232, }, {250, 159, 236, 49, 150, 209, }, {157, 17, 106, 128, 197, 249, }, {113, 98, 105, 130, 64, 220, }, {71, 176, 112, 165, 81, 222, }, {162, 129, 19, 149, 12, 253, }, {132, 185, 233, 235, 9, 231, }, {160, 115, 52, 217, 215, 229, }, {8, 30, 168, 65, 71, 237, }, {159, 98, 232, 173, 150, 206, }, };

void setup() {
  // Initialize hardware:
  Serial.begin(115200); // Serial is the USB serial port

#ifdef XENON
  // the particle xenon has 2 user controllable LEDS - one RGB and one Blue.
  pinMode(XENON_BLUE_LED_PIN, OUTPUT);
  digitalWrite(XENON_BLUE_LED_PIN, HIGH);
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
  neopixelTimer.begin(NEOPIXEL_TIMER_SPEED, neopixelTimerCallback);

  batteryCheckTimer.begin(15000, batteryCheckCallback);
  batteryCheckTimer.start();

  enterProximityMode();

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
