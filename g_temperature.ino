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
