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

BLEUart bleuart; // uart over ble

// Define hardware: LED and Button pins and states
const int LED_PIN = 7;
#define LED_OFF LOW
#define LED_ON HIGH

#define NEOPIXEL_VERSION_STRING "Neopixel v2.0"
#define STRIP_PIN                     19   /* Pin used to drive the NeoPixels */
#define STRIP_LED_COUNT 10
#define FRAME_COUNT 10
uint8_t *pixelBuffer = NULL;

const int BUTTON_PIN = 13;
#define BUTTON_ACTIVE LOW
int lastButtonState = -1;
unsigned int loopCycles = -1;

long deviceInRange = 0;

Adafruit_NeoPixel neopixel = Adafruit_NeoPixel();



void setup() {
  // Initialize hardware:
  Serial.begin(9600); // Serial is the USB serial port
  pinMode(LED_PIN, OUTPUT); // Turn on-board blue LED off
  digitalWrite(LED_PIN, LED_OFF);
  pinMode(BUTTON_PIN, INPUT);

   // Config Neopixels
   neopixel.begin();
   neopixel.updateLength(STRIP_LED_COUNT);
   neopixel.setPin(STRIP_PIN);
   pixelBuffer = new uint8_t[STRIP_LED_COUNT*3];
   neopixel.setBrightness(100);

  // Uncomment the code below to disable sharing
  // the connection LED on pin 7.
  //Bluefruit.autoConnLed(false);

  // Initialize Bluetooth:
  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(8);
  Bluefruit.setName("CatsfishSF");
  bleuart.begin();

  // Start advertising device and bleuart services
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  // Set advertising interval (in unit of 0.625ms):
  Bluefruit.Advertising.setInterval(32, 244);
  // number of seconds in fast mode:
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);  

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Filter for devices with a min RSSI of -80 dBm
   * - Interval = 100 ms, window = 50 ms
   * - Use active scan (requests the optional scan response packet)
   * - Start(0) = will scan forever since no timeout is given
   */
  Bluefruit.Scanner.setRxCallback(basic_scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
//  Bluefruit.Scanner.filterRssi(-60);            // Only invoke callback when RSSI >= -80 dBm
  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(true);        // Request scan response data
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

}

void loop() {
  loopCycles++;
  const int numFrames = 10;
  const int cyclesPerFrame = 10;
  int frame = (loopCycles / cyclesPerFrame) % numFrames;
//  if (frame == 0) { loopCycles = -1; }
  
  // If data has come in via BLE:
  if (bleuart.available()) {
    uint8_t c;
    // use bleuart.read() to read a character sent over BLE
    c = (uint8_t) bleuart.read();
    // Print out the character for debug purposes:
    Serial.write(c);

    // If the character is one of our expected values,
    // do something:
    switch (c) {
      // 0 number or character, turn the LED off:
      case 0:
      case '0':
        digitalWrite(LED_PIN, LED_OFF);
        break;
      // 1 number or character, turn the LED on:
      case 1:
      case '1':
        digitalWrite(LED_PIN, LED_ON);
        break;
      default:
        break;
    }
  }

  // If our button state has changed:
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState != lastButtonState) {
    lastButtonState = buttonState;
    // Write the new button state to the bleuart TX char
    bleuart.write(!buttonState);
  }

  if (millis() - deviceInRange < 5000){
    digitalWrite(LED_PIN, LED_ON);
    showInRange(frame);
  } else {
    digitalWrite(LED_PIN, LED_OFF);
    turn_off();
  }

  delay(10);
}


void showInRange(int frame){
  int p[3];
  for(int i = 0; i < STRIP_LED_COUNT; i++){
    if(i == frame) {
      p[0] = 255; p[1] = 0; p[2] = 255;
    } else if(i == (FRAME_COUNT - 1 - frame)) {
      p[0] = 255; p[1] = 0; p[2] = 0;      
    } else {
      p[0] = 0; p[1] = 0; p[2] = 0;
    }
    for(int j = 0; j < 3; j++) {
      pixelBuffer[i*3 + j] = p[j];
    }
  }
  swapBuffers();
}

void turn_off(){
  for(int i = 0; i < STRIP_LED_COUNT; i++){
    pixelBuffer[i*3] = 0;
    pixelBuffer[i*3+1] = 0;
    pixelBuffer[i*3+2] = 0;      
  }
  swapBuffers();
}

void swapBuffers()
{
  uint8_t *base_addr = pixelBuffer;
  int pixelIndex = 0;

  for (int i = 0; i < STRIP_LED_COUNT; i++) {
    neopixel.setPixelColor(pixelIndex, neopixel.Color(*base_addr, *(base_addr+1), *(base_addr+2)));
    base_addr+=3;
    pixelIndex++;
  }
  neopixel.show();
}

void basic_scan_callback(ble_gap_evt_adv_report_t* report)
{
  PRINT_LOCATION();
  uint8_t len = 0;
  uint8_t buffer[32];
  memset(buffer, 0, sizeof(buffer));
  
 
  /* Complete Local Name */
  if(Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, buffer, sizeof(buffer)))
  {
    String s = String((char*)buffer);
    

    if (s.startsWith("Catsfish")){
      Serial.printf("%14s %d dBm\n", "RSSI", report->rssi);
      Serial.printf("%14s %s\n", "COMPLETE NAME", buffer);
      Serial.println();

//      if (report->rssi > -60){
        deviceInRange = millis();
//      }
    }
    memset(buffer, 0, sizeof(buffer));


  }



  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  Bluefruit.Scanner.resume();
}

//
//void scan_callback(ble_gap_evt_adv_report_t* report)
//{
//  PRINT_LOCATION();
//  uint8_t len = 0;
//  uint8_t buffer[32];
//  memset(buffer, 0, sizeof(buffer));
//  
//  /* Display the timestamp and device address */
//  if (report->type.scan_response)
//  {
//    Serial.printf("[SR%10d] Packet received from ", millis());
//  }
//  else
//  {
//    Serial.printf("[ADV%9d] Packet received from ", millis());
//  }
//  // MAC is in little endian --> print reverse
//  Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
//  Serial.print("\n");
//
//  /* Raw buffer contents */
//  Serial.printf("%14s %d bytes\n", "PAYLOAD", report->data.len);
//  if (report->data.len)
//  {
//    Serial.printf("%15s", " ");
//    Serial.printBuffer(report->data.p_data, report->data.len, '-');
//    Serial.println();
//  }
//
//  /* RSSI value */
//  Serial.printf("%14s %d dBm\n", "RSSI", report->rssi);
//
//  /* Adv Type */
//  Serial.printf("%14s ", "ADV TYPE");
//  if ( report->type.connectable ) 
//  {
//    Serial.print("Connectable ");
//  }else
//  {
//    Serial.print("Non-connectable ");
//  }
//  
//  if ( report->type.directed )
//  {
//    Serial.println("directed");
//  }else
//  {
//    Serial.println("undirected");
//  }
//
//  /* Shortened Local Name */
//  if(Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, buffer, sizeof(buffer)))
//  {
//    Serial.printf("%14s %s\n", "SHORT NAME", buffer);
//    memset(buffer, 0, sizeof(buffer));
//  }
//
//  /* Complete Local Name */
//  if(Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, buffer, sizeof(buffer)))
//  {
//    Serial.printf("%14s %s\n", "COMPLETE NAME", buffer);
//    memset(buffer, 0, sizeof(buffer));
//  }
//
//  /* TX Power Level */
//  if (Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_TX_POWER_LEVEL, buffer, sizeof(buffer)))
//  {
//    Serial.printf("%14s %i\n", "TX PWR LEVEL", buffer[0]);
//    memset(buffer, 0, sizeof(buffer));
//  }
//
//  /* Check for UUID16 Complete List */
//  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE, buffer, sizeof(buffer));
//  if ( len )
//  {
//    printUuid16List(buffer, len);
//  }
//
//  /* Check for UUID16 More Available List */
//  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE, buffer, sizeof(buffer));
//  if ( len )
//  {
//    printUuid16List(buffer, len);
//  }
//
//  /* Check for UUID128 Complete List */
//  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE, buffer, sizeof(buffer));
//  if ( len )
//  {
//    printUuid128List(buffer, len);
//  }
//
//  /* Check for UUID128 More Available List */
//  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE, buffer, sizeof(buffer));
//  if ( len )
//  {
//    printUuid128List(buffer, len);
//  }  
//
//  /* Check for BLE UART UUID */
//  if ( Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE) )
//  {
//    Serial.printf("%14s %s\n", "BLE UART", "UUID Found!");
//  }
//
//  /* Check for DIS UUID */
//  if ( Bluefruit.Scanner.checkReportForUuid(report, UUID16_SVC_DEVICE_INFORMATION) )
//  {
//    Serial.printf("%14s %s\n", "DIS", "UUID Found!");
//  }
//
//  /* Check for Manufacturer Specific Data */
//  len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buffer, sizeof(buffer));
//  if (len)
//  {
//    Serial.printf("%14s ", "MAN SPEC DATA");
//    Serial.printBuffer(buffer, len, '-');
//    Serial.println();
//    memset(buffer, 0, sizeof(buffer));
//  }  
//
//  Serial.println();
//
//  // For Softdevice v6: after received a report, scanner will be paused
//  // We need to call Scanner resume() to continue scanning
//  Bluefruit.Scanner.resume();
//}
//
//void printUuid16List(uint8_t* buffer, uint8_t len)
//{
//  Serial.printf("%14s %s", "16-Bit UUID");
//  for(int i=0; i<len; i+=2)
//  {
//    uint16_t uuid16;
//    memcpy(&uuid16, buffer+i, 2);
//    Serial.printf("%04X ", uuid16);
//  }
//  Serial.println();
//}
//
//void printUuid128List(uint8_t* buffer, uint8_t len)
//{
//  (void) len;
//  Serial.printf("%14s %s", "128-Bit UUID");
//
//  // Print reversed order
//  for(int i=0; i<16; i++)
//  {
//    const char* fm = (i==4 || i==6 || i==8 || i==10) ? "-%02X" : "%02X";
//    Serial.printf(fm, buffer[15-i]);
//  }
//
//  Serial.println();  
//}
