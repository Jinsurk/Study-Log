#include <Arduino_LSM6DS3.h>
#include <ArduinoBLE.h>
#include <math.h>
#include "accel_multi_calibration.h"

// Stationary gyro bias from latest capture (999 valid rows). Validate on a new capture. Units: deg/s.
constexpr float GYRO_BIAS[3] = {-0.01709710f, -0.63518519f, 0.37389389f};
constexpr unsigned long OUTPUT_PERIOD_MS = 100;  // 10 BLE updates per second
BLEService imuService("7f510001-1b15-4b3b-8a54-73251f861234");
BLEStringCharacteristic anglesCharacteristic(
  "7f510002-1b15-4b3b-8a54-73251f861234", BLERead | BLENotify, 20);

float accelRaw[3], gyroRaw[3];
bool haveAccel = false, haveGyro = false;
unsigned long lastOutputMs = 0;
unsigned long lastFilterUs = 0;
float rollFiltered = 0, pitchFiltered = 0;
bool filterReady = false;
constexpr float FILTER_TAU_S = 0.5f;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  if (!IMU.begin()) {
    Serial.println("ERROR: IMU initialization failed");
    while (true) { delay(100); }
  }
  if (!BLE.begin()) {
    Serial.println("ERROR: BLE initialization failed");
    while (true) { digitalWrite(LED_BUILTIN, HIGH); delay(200);
                   digitalWrite(LED_BUILTIN, LOW); delay(200); }
  }
  BLE.setLocalName("Nano33-IMU");
  BLE.setAdvertisedService(imuService);
  imuService.addCharacteristic(anglesCharacteristic);
  BLE.addService(imuService);
  anglesCharacteristic.writeValue("R:0.00,P:0.00");
  BLE.advertise();
  Serial.println("BLE ready: Nano33-IMU");
  delay(3000);
}

void loop() {
  BLE.poll();
  digitalWrite(LED_BUILTIN, BLE.connected() ? HIGH : LOW);
  // Keep the latest reading from each sensor. These are not hardware-synchronized.
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(accelRaw[0], accelRaw[1], accelRaw[2]);
    haveAccel = true;
  }
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gyroRaw[0], gyroRaw[1], gyroRaw[2]);
    haveGyro = true;
  }
  const unsigned long now = millis();
  if (!haveAccel || !haveGyro) return;

  haveAccel = haveGyro = false;

  float centered[3], accelCal[3] = {0, 0, 0}, gyroCal[3];
  for (int i = 0; i < 3; ++i) {
    centered[i] = accelRaw[i] - ACCEL_MULTI_OFFSET[i];
    gyroCal[i] = gyroRaw[i] - GYRO_BIAS[i];
  }
  // a_cal = C * (a_raw - offset): row-by-column multiplication.
  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 3; ++col) {
      accelCal[row] += ACCEL_MULTI_C[row][col] * centered[col];
    }
  }
  const float norm = sqrtf(accelCal[0] * accelCal[0]
                         + accelCal[1] * accelCal[1]
                         + accelCal[2] * accelCal[2]);
  // Accelerometer-only tilt convention: +Z upward gives roll=pitch=0.
  // Roll: rotation about X, Pitch: rotation about Y (degrees).
  // Accurate for static/slow motion; linear acceleration disturbs these angles.
  constexpr float RAD_TO_DEGREES = 57.2957795131f;
  const float rollDeg = atan2f(accelCal[1], accelCal[2]) * RAD_TO_DEGREES;
  const float pitchDeg = atan2f(-accelCal[0],
      sqrtf(accelCal[1] * accelCal[1] + accelCal[2] * accelCal[2])) * RAD_TO_DEGREES;
  // Update at sensor reading rate, independently of the 20 Hz CSV output.
  const unsigned long filterUs = micros();
  if (!filterReady) {
    rollFiltered = rollDeg;
    pitchFiltered = pitchDeg;
    filterReady = true;
  } else {
    const float dt = (filterUs - lastFilterUs) * 1.0e-6f;
    const float alpha = FILTER_TAU_S / (FILTER_TAU_S + dt);
    // Small-tilt approximation: Euler angle rates approximately equal gx, gy.
    const float rollPrediction = rollFiltered + gyroCal[0] * dt;
    const float pitchPrediction = pitchFiltered + gyroCal[1] * dt;
    float rollError = rollDeg - rollPrediction;
    while (rollError > 180.0f) rollError -= 360.0f;
    while (rollError < -180.0f) rollError += 360.0f;
    rollFiltered = rollPrediction + (1.0f - alpha) * rollError;
    while (rollFiltered > 180.0f) rollFiltered -= 360.0f;
    while (rollFiltered < -180.0f) rollFiltered += 360.0f;
    pitchFiltered = alpha * pitchPrediction + (1.0f - alpha) * pitchDeg;
  }
  lastFilterUs = filterUs;
  if (now - lastOutputMs < OUTPUT_PERIOD_MS) return;
  lastOutputMs = now;
  // ASCII text fits within a 20-byte BLE notification: R=roll, P=pitch, degrees.
  String payload = "R:" + String(rollFiltered, 2) + ",P:" + String(pitchFiltered, 2);
  anglesCharacteristic.writeValue(payload);
  if (Serial) Serial.println(payload);
}
