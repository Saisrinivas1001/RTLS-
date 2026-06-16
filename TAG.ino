#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>

#define TAG_ID "TAG_1"  // Change for each tag

void setup() {
  Serial.begin(115200);
  BLEDevice::init(TAG_ID);
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();
  Serial.println("TAG " + String(TAG_ID) + " advertising...");
}

void loop() {
  delay(10);
}
