#include <WiFi.h>
#include <esp_now.h>
#include <math.h>

#define NUM_ANCHORS 4
#define MAX_TAGS    5

/* ===== ANCHOR POSITIONS (meters) ===== */
float ax[NUM_ANCHORS] = {0.0, 4.0, 4.0, 0.0};
float ay[NUM_ANCHORS] = {0.0, 0.0, 4.0, 4.0};

/* ===== RSSI MODEL ===== */
float RSSI0 = -59.0;   // RSSI at 1m (calibrate later)
float PATH_LOSS = 2.2;

/* ===== PACKET ===== */
typedef struct {
  uint8_t anchorId;
  char tagName[8];
  int8_t rssi;
} Packet;

/* ===== 1D KALMAN (RSSI) ===== */
struct Kalman1D {
  float x = -70;
  float p = 1;
  float q = 0.5;
  float r = 6;

  float update(float z) {
    p += q;
    float k = p / (p + r);
    x += k * (z - x);
    p *= (1 - k);
    return x;
  }
};

/* ===== 2D KALMAN (POSITION) ===== */
struct Kalman2D {
  float x = 2, y = 2;
  float p = 1;
  float q = 0.05;
  float r = 0.4;

  void update(float mx, float my) {
    p += q;
    float k = p / (p + r);
    x += k * (mx - x);
    y += k * (my - y);
    p *= (1 - k);
  }
};

/* ===== TAG STORAGE ===== */
struct TagData {
  bool active;
  char name[8];

  bool got[NUM_ANCHORS];
  float filtRssi[NUM_ANCHORS];

  Kalman1D kfRssi[NUM_ANCHORS];
  Kalman2D kfPos;
};

TagData tags[MAX_TAGS];

/* ===== UTIL ===== */
float rssiToDistance(float rssi) {
  return pow(10.0, (RSSI0 - rssi) / (10.0 * PATH_LOSS));
}

/* ===== TAG LOOKUP ===== */
int getTagIndex(const char* name) {
  for (int i = 0; i < MAX_TAGS; i++) {
    if (tags[i].active && strcmp(tags[i].name, name) == 0)
      return i;
  }

  for (int i = 0; i < MAX_TAGS; i++) {
    if (!tags[i].active) {
      tags[i].active = true;
      strcpy(tags[i].name, name);
      memset(tags[i].got, 0, sizeof(tags[i].got));
      return i;
    }
  }
  return -1;
}

/* ===== WNLLS POSITION SOLVER ===== */
void computePosition(float &x, float &y, float rssi[]) {

  x = 2.0;
  y = 2.0;

  for (int iter = 0; iter < 6; iter++) {
    float Jx = 0, Jy = 0;
    float Hxx = 0, Hyy = 0;

    for (int i = 0; i < NUM_ANCHORS; i++) {
      float d = rssiToDistance(rssi[i]);
      float dx = x - ax[i];
      float dy = y - ay[i];
      float r = sqrt(dx*dx + dy*dy) + 0.001;

      float w = 1.0 / (d * d);

      Jx  += w * (dx - d * dx / r);
      Jy  += w * (dy - d * dy / r);
      Hxx += w;
      Hyy += w;
    }

    x -= Jx / Hxx;
    y -= Jy / Hyy;
  }
}

/* ===== ESP-NOW RX ===== */
void onDataRecv(const uint8_t* mac,
                const uint8_t* data,
                int len) {

  if (len != sizeof(Packet)) return;

  Packet pkt;
  memcpy(&pkt, data, sizeof(pkt));

  int anchor = pkt.anchorId - 1;
  if (anchor < 0 || anchor >= NUM_ANCHORS) return;

  int t = getTagIndex(pkt.tagName);
  if (t < 0) return;

  // 1D Kalman on RSSI
  tags[t].filtRssi[anchor] =
    tags[t].kfRssi[anchor].update(pkt.rssi);

  tags[t].got[anchor] = true;

  /* ===== CHECK A1–A4 ===== */
  bool ready = true;
  for (int i = 0; i < NUM_ANCHORS; i++)
    if (!tags[t].got[i]) ready = false;

  if (!ready) return;

  /* ===== WNLLS ===== */
  float mx, my;
  computePosition(mx, my, tags[t].filtRssi);

  /* ===== 2D KALMAN ===== */
  tags[t].kfPos.update(mx, my);

  /* ===== OUTPUT ===== */
  Serial.print(tags[t].name);
  Serial.print(" → X:");
  Serial.print(tags[t].kfPos.x, 2);
  Serial.print(" Y:");
  Serial.println(tags[t].kfPos.y, 2);

  /* ===== RESET ===== */
  memset(tags[t].got, 0, sizeof(tags[t].got));
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_now_init();

  esp_now_peer_info_t peer{};
  memset(peer.peer_addr, 0xFF, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  esp_now_register_recv_cb(onDataRecv);
}

/* ===== LOOP ===== */
void loop() {}
