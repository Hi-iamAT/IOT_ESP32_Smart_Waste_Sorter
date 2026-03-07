#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <CheapStepper.h>

// ===================== ENUM =====================
enum TargetBin {
  BIN_WET,
  BIN_DRY,
  BIN_METAL
};

// ===================== WiFi + ThingSpeak =====================
const char* ssid     = "Redowan";
const char* password = "01610142404";

// FIX #3: Use correct ThingSpeak HTTP endpoint (port 80, not HTTPS)
const char* tsServer = "http://api.thingspeak.com/update";
String tsApiKey      = "M039Y3AXLFZT2PD8";

// ===================== PIN DEFINITIONS =====================
// FIX #5/#6: GPIO 2 is a boot-strapping pin (onboard LED on most dev boards).
// Using it as a stepper output can prevent boot if pulled LOW.
// Changed stepper pins to safe GPIOs: 16, 17, 5, 15.
// - GPIO 16 / 17: safe general-purpose outputs
// - GPIO  5: has internal pull-up at boot, stepper coil state is fine
// - GPIO 15: has internal pull-down at boot, stepper coil state is fine
// Verify these don't conflict with your wiring.
#define STEPPER_IN1 4
#define STEPPER_IN2 2
#define STEPPER_IN3 18
#define STEPPER_IN4 19

// Servo for gate
#define PIN_SERVO_GATE 23

// Sensors
#define PIN_IR_MAIN    32   // IR sensor — LOW = object detected
#define PIN_METAL      13   // Metal sensor — LOW = metal detected
#define PIN_RAIN_ANALOG 35  // Rain/moisture AO → ADC1_CH7 (input-only, correct)

// Buzzer (active-low logic: LOW = ON, HIGH = OFF)
// FIX #8: If your buzzer is active-high, swap HIGH/LOW in buzzerBeep().
#define PIN_BUZZER 21

// Ultrasonic sensors — 3 bins
#define TRIG_WET 26
#define ECHO_WET 12

#define TRIG_DRY 25
#define ECHO_DRY 14

#define TRIG_MET 33
#define ECHO_MET 27

// ===================== TUNING CONSTANTS =====================
const int BIN_DEPTH_CM = 8;   // Real bin depth in cm — calibrate!
const int BIN_EMPTY_CM = 7;  // ← add this line
const int FULL_THRESHOLD_PCT    = 70;    // Alarm threshold %
const unsigned long UPLOAD_INTERVAL_MS = 15000UL;

// Rain/moisture hysteresis thresholds (lower AO value = wetter on most modules)
const int RAIN_WET_ON       = 1800;  // Becomes wet when value drops to/below this
const int RAIN_WET_OFF      = 2200;  // Becomes dry when value rises to/above this
const int RAIN_INVALID_LOW  = 20;
const int RAIN_INVALID_HIGH = 4090;

// ===================== STEPPER DEGREES =====================
const int DEG_TO_WET   = 120;
const int DEG_TO_METAL = 240;
const int STEPPER_RPM  = 17;

// ===================== TIMING =====================
const unsigned long DROP_OPEN_TIME_MS  = 1200UL;
const unsigned long SERVO_SETTLE_MS    = 700UL;
const unsigned long STEPPER_SETTLE_MS  = 500UL;

// ===================== SERVO ANGLES =====================
const int SERVO_MIN_US      = 500;
const int SERVO_MAX_US      = 2400;
const int GATE_CLOSED_ANGLE = 10;
const int GATE_OPEN_ANGLE   = 95;

// ===================== OBJECTS / GLOBALS =====================
CheapStepper stepper(STEPPER_IN1, STEPPER_IN2, STEPPER_IN3, STEPPER_IN4);
Servo gateServo;

unsigned long lastUploadMs  = 0;
bool initialUploadDone      = false; // FIX #9: skip duplicate first upload in loop

int pctWet = 0, pctDry = 0, pctMet = 0;
bool rainIsWet = false;

// FIX #12: WiFi reconnect cooldown
unsigned long lastWifiAttemptMs = 0;
const unsigned long WIFI_RETRY_COOLDOWN_MS = 20000UL; // Don't retry more than every 20s

// ===================== PROTOTYPES =====================
void buzzerBeep(int times, int onMs = 120, int offMs = 120);
bool wifiEnsureConnected();
void readAllBinLevels();
void uploadThingSpeak();
long readUltrasonicCM(int trigPin, int echoPin);
int  distanceToPercent(long distanceCm);
int  median3(int a, int b, int c);
int  readBinPercentMedian3(int trigPin, int echoPin);
int  readRainFiltered();
void updateRainState();
TargetBin classifyWaste();
void moveSorterTo(TargetBin bin);
void returnSorterToDry(TargetBin bin);
void dropOneItem();

// ===================== HELPERS =====================

void buzzerBeep(int times, int onMs, int offMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(PIN_BUZZER, LOW);   // Active-low ON
    delay(onMs);
    digitalWrite(PIN_BUZZER, HIGH);  // Active-low OFF
    if (offMs > 0) delay(offMs);
  }
}

// FIX #3 + #12: Use WiFiClient for HTTP, add reconnect cooldown
bool wifiEnsureConnected() {
  if (WiFi.status() == WL_CONNECTED) return true;

  unsigned long now = millis();
  if (now - lastWifiAttemptMs < WIFI_RETRY_COOLDOWN_MS && lastWifiAttemptMs != 0) {
    Serial.println("WiFi cooldown — skipping reconnect attempt");
    return false;
  }
  lastWifiAttemptMs = now;

  Serial.println("WiFi not connected, attempting reconnect...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 12000UL) {
    delay(250);
  }

  bool ok = (WiFi.status() == WL_CONNECTED);
  Serial.println(ok ? "WiFi connected!" : "WiFi failed.");
  return ok;
}

// Ultrasonic: returns distance in cm, or -1 on timeout
long readUltrasonicCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000UL);
  if (duration == 0) return -1;
  return (long)(duration * 0.034 / 2.0);
}

int distanceToPercent(long distanceCm) {
  if (distanceCm < 0) return -1;
  if (distanceCm >= BIN_EMPTY_CM) return 0;  // ← was BIN_DEPTH_CM
  if (distanceCm <= 0) return 100;

  int pct = (int)(((BIN_EMPTY_CM - distanceCm) * 100.0 / BIN_EMPTY_CM) + 0.5);  // ← both changed
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return pct;
}

int median3(int a, int b, int c) {
  if (a > b) { int t = a; a = b; b = t; }
  if (b > c) { int t = b; b = c; c = t; }
  if (a > b) { int t = a; a = b; b = t; }
  return b;
}

// FIX #11: Handles all-invalid case: returns -1 so caller skips update.
// If only some readings are -1, substitutes with the best available value.
int readBinPercentMedian3(int trigPin, int echoPin) {
  int p1 = distanceToPercent(readUltrasonicCM(trigPin, echoPin)); delay(30);
  int p2 = distanceToPercent(readUltrasonicCM(trigPin, echoPin)); delay(30);
  int p3 = distanceToPercent(readUltrasonicCM(trigPin, echoPin));

  // All invalid
  if (p1 < 0 && p2 < 0 && p3 < 0) return -1;

  // Substitute invalid readings with a valid one
  if (p2 < 0) p2 = (p1 >= 0) ? p1 : p3;
  if (p1 < 0) p1 = p2;
  if (p3 < 0) p3 = p2;

  return median3(p1, p2, p3);
}

void readAllBinLevels() {
  int d = readBinPercentMedian3(TRIG_DRY, ECHO_DRY);
  int w = readBinPercentMedian3(TRIG_WET, ECHO_WET);
  int m = readBinPercentMedian3(TRIG_MET, ECHO_MET);


  if (d >= 0) pctDry = d;
  if (w >= 0) pctWet = w;
  if (m >= 0) pctMet = m;
}

// FIX #3: Use WiFiClient with HTTPClient for ESP32 core 2.x compatibility
void uploadThingSpeak() {
  if (!wifiEnsureConnected()) {
    Serial.println("Upload skipped — no WiFi");
    return;
  }

  int alarm = (pctWet >= FULL_THRESHOLD_PCT ||
               pctDry >= FULL_THRESHOLD_PCT ||
               pctMet >= FULL_THRESHOLD_PCT) ? 1 : 0;

  WiFiClient client;
  HTTPClient http;

  String url = String(tsServer)
               + "?api_key=" + tsApiKey
               + "&field1=" + String(pctDry)
               + "&field2=" + String(pctWet)
               + "&field3=" + String(pctMet)
               + "&field4=" + String(alarm);

  http.begin(client, url);  // FIX: pass WiFiClient explicitly
  int httpCode = http.GET();
  http.end();

  Serial.print("ThingSpeak response: ");
  Serial.println(httpCode);
}

// ===================== RAIN/MOISTURE =====================
int readRainFiltered() {
  int a = analogRead(PIN_RAIN_ANALOG); delay(5);
  int b = analogRead(PIN_RAIN_ANALOG); delay(5);
  int c = analogRead(PIN_RAIN_ANALOG);
  return median3(a, b, c);
}

void updateRainState() {
  int v = readRainFiltered();

  if (v <= RAIN_INVALID_LOW || v >= RAIN_INVALID_HIGH) {
    rainIsWet = false;
    Serial.print("RAIN invalid -> DRY. v="); Serial.println(v);
    return;
  }

  // Hysteresis
  if (!rainIsWet) {
    if (v <= RAIN_WET_ON)  rainIsWet = true;
  } else {
    if (v >= RAIN_WET_OFF) rainIsWet = false;
  }

  Serial.print("RAIN v="); Serial.print(v);
  Serial.print(" wet="); Serial.println(rainIsWet);
}

// ===================== CLASSIFICATION =====================
TargetBin classifyWaste() {
 if (digitalRead(PIN_METAL) == LOW) return BIN_METAL;  // Metal = highest priority
  updateRainState();
  if (rainIsWet) return BIN_WET;
  return BIN_DRY;
}

// ===================== STEPPER MOVEMENT =====================
void moveSorterTo(TargetBin bin) {
  int deg = 0;
  if (bin == BIN_WET)   deg = DEG_TO_WET;
  else if (bin == BIN_METAL) deg = DEG_TO_METAL;

  if (deg > 0) {
    stepper.moveDegreesCW(deg);
    delay(STEPPER_SETTLE_MS);
  }
}

void returnSorterToDry(TargetBin bin) {
  int deg = 0;
  if (bin == BIN_WET)   deg = DEG_TO_WET;
  else if (bin == BIN_METAL) deg = DEG_TO_METAL;

  if (deg > 0) {
    stepper.moveDegreesCCW(deg);
    delay(STEPPER_SETTLE_MS);
  }
}

// ===================== GATE DROP =====================
void dropOneItem() {
  gateServo.write(GATE_OPEN_ANGLE);
  delay(DROP_OPEN_TIME_MS);
  gateServo.write(GATE_CLOSED_ANGLE);
  delay(SERVO_SETTLE_MS);
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n===== ESP32 WASTE SORTER START =====");
  Serial.flush();

  // ADC attenuation for GPIO35 rain sensor (0–3.3V range)
  analogSetAttenuation(ADC_11db);
  Serial.println("ADC attenuation set");

  // Input pins
  pinMode(PIN_IR_MAIN, INPUT_PULLUP);
  pinMode(PIN_METAL,   INPUT_PULLUP);

  // Output pins
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, HIGH); // Buzzer OFF (active-low)

  // Ultrasonic pins
  pinMode(TRIG_WET, OUTPUT); pinMode(ECHO_WET, INPUT);
  pinMode(TRIG_DRY, OUTPUT); pinMode(ECHO_DRY, INPUT);
  pinMode(TRIG_MET, OUTPUT); pinMode(ECHO_MET, INPUT);

  // Gate servo
  gateServo.setPeriodHertz(50);
  gateServo.attach(PIN_SERVO_GATE, SERVO_MIN_US, SERVO_MAX_US);
  gateServo.write(GATE_CLOSED_ANGLE);
  delay(300);
  Serial.println("Gate servo attached and closed");

  // Stepper
  stepper.setRpm(STEPPER_RPM);
  Serial.println("Stepper RPM set");

  // WiFi
  wifiEnsureConnected();

  // Initial bin levels + upload
  readAllBinLevels();
  Serial.print("Initial bins — Dry="); Serial.print(pctDry);
  Serial.print("% Wet="); Serial.print(pctWet);
  Serial.print("% Metal="); Serial.print(pctMet); Serial.println("%");
  uploadThingSpeak();
  initialUploadDone = true; // FIX #9: mark so loop doesn't double-upload immediately

  lastUploadMs = millis(); // FIX #9: align timer so next upload is 15s from now
// ────────────────────────────────────────────────
//     TEMPORARY STEPPER + ULN2003 LED TEST
//     (runs only once at startup)
// ────────────────────────────────────────────────

Serial.println("\n=== STEPPER & ULN2003 LED TEST START ===");

Serial.println("Setting stepper pins as output (just in case)...");
pinMode(STEPPER_IN1, OUTPUT);
pinMode(STEPPER_IN2, OUTPUT);
pinMode(STEPPER_IN3, OUTPUT);
pinMode(STEPPER_IN4, OUTPUT);

// Step 1: Test LEDs one by one (no motor movement yet)
Serial.println("Testing LEDs one by one (0.6 sec each)...");
digitalWrite(STEPPER_IN1, HIGH); delay(600); digitalWrite(STEPPER_IN1, LOW); Serial.println("→ LED A should have lit");
digitalWrite(STEPPER_IN2, HIGH); delay(600); digitalWrite(STEPPER_IN2, LOW); Serial.println("→ LED B should have lit");
digitalWrite(STEPPER_IN3, HIGH); delay(600); digitalWrite(STEPPER_IN3, LOW); Serial.println("→ LED C should have lit");
digitalWrite(STEPPER_IN4, HIGH); delay(600); digitalWrite(STEPPER_IN4, LOW); Serial.println("→ LED D should have lit");

// Step 2: Test actual movement (CheapStepper way)
Serial.println("Testing full movement: 120° CW then back CCW");
Serial.println("Watch LEDs: they should chase in sequence");

stepper.setRpm(10);          // slower = easier to see LEDs
stepper.moveDegreesCW(120);
delay(4000);                 // give time to finish + observe

stepper.moveDegreesCCW(120);
delay(4000);

Serial.println("Stepper movement test finished");
Serial.println("If motor didn't move but LEDs lit → check motor power / wiring");
Serial.println("=== STEPPER TEST END ===\n");
  buzzerBeep(2, 80, 80);
  Serial.println("===== SETUP COMPLETE =====");
  Serial.flush();
}

// ===================== LOOP =====================
void loop() {



  // Periodic ThingSpeak upload
  if (millis() - lastUploadMs >= UPLOAD_INTERVAL_MS) {
    lastUploadMs = millis();
    Serial.println("Periodic upload triggered");
    readAllBinLevels();
    Serial.print("Bins: Dry="); Serial.print(pctDry);
    Serial.print("% Wet="); Serial.print(pctWet);
    Serial.print("% Metal="); Serial.print(pctMet); Serial.println("%");
    uploadThingSpeak();
  }

  // IR trigger: object detected
  if (digitalRead(PIN_IR_MAIN) == LOW) {
    delay(60); // Debounce
    if (digitalRead(PIN_IR_MAIN) == LOW) {

      TargetBin bin = classifyWaste();

      Serial.print("Object detected! Bin: ");
      Serial.println(bin == BIN_WET ? "WET" : bin == BIN_METAL ? "METAL" : "DRY");

      moveSorterTo(bin);
      dropOneItem();
      returnSorterToDry(bin);

      buzzerBeep(1, 60, 0);

      // Wait for object to clear IR sensor (max 1.5s)
      unsigned long t0 = millis();
      while (millis() - t0 < 1500UL) {
        if (digitalRead(PIN_IR_MAIN) == HIGH) break;
        delay(20);
      }
    }
  }

  delay(10);
}