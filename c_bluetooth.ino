
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



uint8_t rssiToLedCount(int8_t rssi){

  // rssi range is about -40 to -100
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



  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  Bluefruit.Scanner.resume();
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
