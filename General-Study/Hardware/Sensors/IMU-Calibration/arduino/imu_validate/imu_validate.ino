#include <Arduino_LSM6DS3.h>

const int SAMPLE_COUNT = 1000;
// Fixed biases from gyro_20261007_135515_052.csv; do not refit on validation data.
const float BIAS_X = -0.00721f;
const float BIAS_Y = -0.50085f;
const float BIAS_Z =  0.07413f;

void setup()
{
    Serial.begin(115200);
    while (!Serial) {}
    if (!IMU.begin()) {
        Serial.println("ERROR: IMU initialization failed");
        while (true) {}
    }
    while (Serial.read() != 'S') {}
    delay(5000);
    Serial.println("sample,t_us,gx_deg_s,gy_deg_s,gz_deg_s,gx_corrected_deg_s,gy_corrected_deg_s,gz_corrected_deg_s");
    int count = 0;
    unsigned long startTime = micros();
    while (count < SAMPLE_COUNT) {
        if (IMU.gyroscopeAvailable()) {
            unsigned long timestamp = micros() - startTime;
            float gx, gy, gz;
            IMU.readGyroscope(gx, gy, gz);
            Serial.print(++count);
            Serial.print(',');
            Serial.print(timestamp);
            Serial.print(',');
            Serial.print(gx, 6);
            Serial.print(',');
            Serial.print(gy, 6);
            Serial.print(',');
            Serial.print(gz, 6);
            Serial.print(',');
            Serial.print(gx - BIAS_X, 6);
            Serial.print(',');
            Serial.print(gy - BIAS_Y, 6);
            Serial.print(',');
            Serial.println(gz - BIAS_Z, 6);
        }
    }
    Serial.println("DONE");
}

void loop() {}
