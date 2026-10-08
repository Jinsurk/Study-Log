#include <Arduino_LSM6DS3.h>

const int SAMPLE_COUNT = 1000;

void setup()
{
    Serial.begin(115200);
    while (!Serial) {}

    if (!IMU.begin()) {
        Serial.println("ERROR: IMU initialization failed");
        while (true) {}
    }

    // PC requests a fresh capture by sending S.
    while (Serial.read() != 'S') {}
    delay(5000);  // Keep the board still during this interval.

    Serial.println("sample,t_us,gx_deg_s,gy_deg_s,gz_deg_s");
    int count = 0;
    unsigned long startTime = micros();

    while (count < SAMPLE_COUNT) {
        if (IMU.gyroscopeAvailable()) {
            unsigned long timestamp = micros() - startTime;
            float gx, gy, gz;
            IMU.readGyroscope(gx, gy, gz);
            count++;

            Serial.print(count);
            Serial.print(',');
            Serial.print(timestamp);
            Serial.print(',');
            Serial.print(gx, 6);
            Serial.print(',');
            Serial.print(gy, 6);
            Serial.print(',');
            Serial.println(gz, 6);
        }
    }

    Serial.println("DONE");
}

void loop()
{
    // One capture per reset; no further sensor reads.
}
