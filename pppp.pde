import processing.serial.*;

/* ================= SERIAL ================= */
Serial esp;
String portName = "COM4";   // <<< CHANGE THIS
int baudRate = 115200;

/* ================= MAP CONFIG ================= */
float roomSize = 4.0;      // meters
float margin = 80;

/* ================= TAG CONFIG ================= */
int MAX_TAGS = 5;
String[] tagNames = new String[MAX_TAGS];

float[] tx = new float[MAX_TAGS];   // raw from PU
float[] ty = new float[MAX_TAGS];
float[] sx = new float[MAX_TAGS];   // smoothed
float[] sy = new float[MAX_TAGS];

boolean[] active = new boolean[MAX_TAGS];

float smoothFactor = 0.15;

/* ================= COLORS ================= */
color[] tagColors = {
  color(0, 200, 0),
  color(0, 120, 255),
  color(255, 160, 0),
  color(180, 0, 255),
  color(255, 0, 100)
};

void setup() {
  size(600, 600);
  background(255);

  esp = new Serial(this, portName, baudRate);
  esp.bufferUntil('\n');

  textAlign(CENTER, CENTER);
  ellipseMode(CENTER);

  // init center
  for (int i = 0; i < MAX_TAGS; i++) {
    tx[i] = ty[i] = sx[i] = sy[i] = 2;
    active[i] = false;
  }
}

void draw() {
  background(245);

  drawGrid();
  drawAnchors();

  // draw all active tags
  for (int i = 0; i < MAX_TAGS; i++) {
    if (!active[i]) continue;

    sx[i] = lerp(sx[i], tx[i], smoothFactor);
    sy[i] = lerp(sy[i], ty[i], smoothFactor);

    drawTag(sx[i], sy[i], tagNames[i], tagColors[i]);
  }
}

/* ================= SERIAL EVENT ================= */
void serialEvent(Serial p) {
  String line = p.readStringUntil('\n');
  if (line == null) return;

  line = trim(line);

  // Expected: TAG_1 → X:1.92 Y:2.08
  if (!line.startsWith("TAG_")) return;

  try {
    String[] parts = split(line, ' ');
    String tag = parts[0];          // TAG_1
    float x = float(parts[2].substring(2)); // after X:
    float y = float(parts[3].substring(2)); // after Y:

    int idx = getTagIndex(tag);
    if (idx < 0) return;

    tx[idx] = x;
    ty[idx] = y;
    active[idx] = true;
  }
  catch (Exception e) {
    // ignore malformed line
  }
}

/* ================= TAG MANAGEMENT ================= */
int getTagIndex(String name) {
  for (int i = 0; i < MAX_TAGS; i++) {
    if (active[i] && tagNames[i].equals(name))
      return i;
  }

  for (int i = 0; i < MAX_TAGS; i++) {
    if (!active[i]) {
      tagNames[i] = name;
      return i;
    }
  }
  return -1;
}

/* ================= DRAWING ================= */
void drawGrid() {
  stroke(200);
  fill(0);

  for (int i = 0; i <= roomSize; i++) {
    float x = map(i, 0, roomSize, margin, width - margin);
    float y = map(i, 0, roomSize, height - margin, margin);

    line(x, margin, x, height - margin);
    line(margin, y, width - margin, y);

    text(i + " m", x, height - margin + 20);
    text(i + " m", margin - 25, y);
  }
}

void drawAnchors() {
  fill(255, 0, 0);
  stroke(0);

  drawAnchor(0, 0, "A1 (0,0)");
  drawAnchor(4, 0, "A2 (4,0)");
  drawAnchor(4, 4, "A3 (4,4)");
  drawAnchor(0, 4, "A4 (0,4)");
}

void drawAnchor(float x, float y, String label) {
  float px = map(x, 0, roomSize, margin, width - margin);
  float py = map(y, 0, roomSize, height - margin, margin);

  ellipse(px, py, 14, 14);
  text(label, px, py - 18);
}

void drawTag(float x, float y, String name, color c) {
  float px = map(x, 0, roomSize, margin, width - margin);
  float py = map(y, 0, roomSize, height - margin, margin);

  fill(c);
  stroke(0);
  ellipse(px, py, 20, 20);

  fill(0);
  text(name, px, py - 30);
  text(nf(x, 2, 2) + ", " + nf(y, 2, 2), px, py + 28);
}
