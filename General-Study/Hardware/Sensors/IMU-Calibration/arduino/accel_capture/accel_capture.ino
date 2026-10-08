#include <Arduino_LSM6DS3.h>

const int SAMPLE_COUNT = 1000;

void sendLine(const String& text)
{
    String frame = text + "\n";
    const uint8_t* data = (const uint8_t*)frame.c_str();
    size_t sent = 0;
    while (sent < frame.length()) {
        size_t written = Serial.write(data + sent, frame.length() - sent);
        sent += written;
        if (written == 0) { delay(1); }
    }
    Serial.flush();
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) {}
    if (!IMU.begin()) {
        sendLine("ERROR: IMU initialization failed");
        while (true) {}
    }

    while (Serial.read() != 'S') {}
    delay(5000);
    sendLine("sample,t_us,ax_g,ay_g,az_g");
    int count = 0;
    unsigned long startTime = micros();
    while (count < SAMPLE_COUNT) {
        if (IMU.accelerationAvailable()) {
            unsigned long timestamp = micros() - startTime;
            float ax, ay, az;
            IMU.readAcceleration(ax, ay, az);
            String row;
            row.reserve(100);
            row += String(++count);
            row += ',';
            row += String(timestamp);
            row += ',';
            row += String(ax, 6);
            row += ',';
            row += String(ay, 6);
            row += ',';
            row += String(az, 6);
            sendLine(row);
            // This static calibration does not require full-rate capture.
            delay(20);
        }
    }
    sendLine("DONE");
}

void loop() {}
