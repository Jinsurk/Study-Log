#include <Arduino_LSM6DS3.h>
#include <math.h>
#include "accel_multi_calibration.h"

// Stationary gyro bias from latest capture (999 valid rows). Validate on a new capture. Units: deg/s.
constexpr float GYRO_BIAS[3] = {-0.01709710f, -0.63518519f, 0.37389389f};
constexpr unsigned long OUTPUT_PERIOD_MS = 50;  // 20 CSV rows per second
constexpr unsigned int SAMPLE_COUNT = 1000;
unsigned int printedSamples = 0;
float accelRaw[3], gyroRaw[3];
bool haveAccel = false, haveGyro = false;
unsigned long lastOutputMs = 0;
unsigned long lastFilterUs = 0;
float rollFiltered = 0, pitchFiltered = 0;
bool filterReady = false;
constexpr float FILTER_TAU_S = 0.5f;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // Open Serial Monitor to start.
  if (!IMU.begin()) {
    Serial.println("ERROR: IMU initialization failed");
    while (true) { delay(100); }
  }
  delay(3000);  // Wait 3 seconds after Serial Monitor connects.
}

void loop() {
  if (printedSamples >= SAMPLE_COUNT) return;  // Stop reading and printing.
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
  Serial.print(now);
  for (int i = 0; i < 3; ++i) { Serial.print(','); Serial.print(accelRaw[i], 6); }
  for (int i = 0; i < 3; ++i) { Serial.print(','); Serial.print(accelCal[i], 6); }
  Serial.print(','); Serial.print(norm, 6);
  for (int i = 0; i < 3; ++i) { Serial.print(','); Serial.print(gyroRaw[i], 6); }
  for (int i = 0; i < 3; ++i) { Serial.print(','); Serial.print(gyroCal[i], 6); }
  Serial.print(','); Serial.print(rollDeg, 3);
  Serial.print(','); Serial.print(pitchDeg, 3);
  Serial.print(','); Serial.print(rollFiltered, 3);
  Serial.print(','); Serial.print(pitchFiltered, 3);
  Serial.println();
  ++printedSamples;
}
