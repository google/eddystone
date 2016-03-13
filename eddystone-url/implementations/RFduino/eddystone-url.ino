#include <RFduinoBLE.h>

uint8_t advdata[] =
{
  0x03,  // Length
  0x03,  // Param: Service List
  0xAA, 0xFE,  // Eddystone ID
  0x0A,  // Length
  0x16,  // Service Data
  0xAA, 0xFE, // Eddystone ID
  0x10,  // URL flag
  0xEB,  // Power
  0x02,  // http://
  'a',
  'b',
  'c',
  0x07,  // .com
};

void setup() {
  RFduinoBLE_advdata = advdata;
  RFduinoBLE_advdata_len = sizeof(advdata);
  RFduinoBLE.advertisementInterval = 700; // Advertise every 700ms
  RFduinoBLE.connectable = false;
  RFduinoBLE.begin();
}

void loop() {
  RFduino_ULPDelay(INFINITE);   // Switch to lower power mode
}