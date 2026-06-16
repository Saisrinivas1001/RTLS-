#include <WiFi.h>
#include <esp_now.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define SCAN_TIME 1   // seconds
#define ANCHOR_ID 3   // change per anchor

BLEScan* pBLEScan;

/* ===== PU MAC (CHANGE IF NEEDED) ===== */
uint8_t PU_MAC[] = {0xBC, 0xDD, 0xC2, 0xCD, 0x42, 0x68};

/* ===== ESP-NOW PACKET (MINIMAL) ===== */
typedef struct {
  uint8_t anchorId;
  char tagName[8];
  int8_t rssi;
} Packet;

/* ================= BLE CALLBACK ================= */
class TagScanCallback : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice dev) {

    if (dev.haveName()) {

      String name = dev.getName().c_str();

      if (name.startsWith("TAG_")) {

        int8_t rssi = dev.getRSSI();

        // YOUR EXISTING SERIAL PRINT (UNCHANGED)
        Serial.print(name);
        Serial.print("  RSSI: ");
        Serial.println(rssi);

        // ===== ESP-NOW SEND (ONLY ADDITION) =====
        Packet pkt;
        pkt.anchorId = ANCHOR_ID;
        strncpy(pkt.tagName, name.c_str(), 7);
        pkt.tagName[7] = '\0';
        pkt.rssi = rssi;

        esp_now_send(PU_MAC, (uint8_t*)&pkt, sizeof(pkt));
      }
    }
  }
};

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("BLE TAG SCAN STARTED");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // ===== ESP-NOW INIT (MINIMAL) =====
  esp_now_init();

  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, PU_MAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  // ===== BLE (UNCHANGED) =====
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();

  pBLEScan->setAdvertisedDeviceCallbacks(
    new TagScanCallback(),
    true   // allow duplicates
  );

  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

/* ================= LOOP ================= */
void loop() {

  pBLEScan->start(SCAN_TIME, false);
  pBLEScan->clearResults();

  delay(50);
}
