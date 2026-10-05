#include <iostream>
#include <iomanip>
#include <memory>
#include <thread>
#include <chrono>

#include "sl_lidar.h"
#include "sl_lidar_driver.h"

int main() {
    std::unique_ptr<sl::IChannel> channel(
        *sl::createSerialPortChannel("COM4", 256000)
    );
    std::unique_ptr<sl::ILidarDriver> lidar(
        *sl::createLidarDriver()
    );

    if (!channel || !lidar)
        return 1;

    auto result = lidar->connect(channel.get());

    if (SL_IS_FAIL(result)) {
        std::cerr << "Cannot open COM4. Close RoboStudio and check USB.\n";
        return 1;
    }

    sl_lidar_response_device_info_t info{};
    sl_lidar_response_device_health_t health{};

    if (SL_IS_FAIL(lidar->getDeviceInfo(info))
        || SL_IS_FAIL(lidar->getHealth(health))
        || health.status == SL_LIDAR_STATUS_ERROR) {
        std::cerr << "Device information/health check failed.\n";
        lidar->disconnect();
        return 1;
    }

    std::cout << "Connected: COM4, 256000 bps; health="
              << int(health.status) << '\n';

    result = lidar->setMotorSpeed();

    if (SL_IS_OK(result))
        result = lidar->startScan(false, true);

    int exit_code = 0;

    if (SL_IS_FAIL(result)) {
        std::cerr << "Motor or scan start failed.\n";
        exit_code = 1;
    } else {
        // 반복문 밖에서 한 번 초기화
        auto last_print = std::chrono::steady_clock::now();

        bool found = false;
        float min_distance = 0.0f;

        for (int scan = 0; scan < 10; ++scan) {
            sl_lidar_response_measurement_node_hq_t nodes[8192];
            size_t count = 8192;

            result = lidar->grabScanDataHq(nodes, count, 3000);

            if (SL_IS_FAIL(result)) {
                std::cerr << "Scan timeout/error.\n";
                exit_code = 1;
                break;
            }

            // 출력 구간 동안 최솟값 누적
            

            for (size_t i = 0; i < count; ++i) {
                if (!nodes[i].dist_mm_q2 || !nodes[i].quality)
                    continue;

                const float distance_mm = nodes[i].dist_mm_q2 / 4.0f;

                if (!found || distance_mm < min_distance) {
                    min_distance = distance_mm;
                    found = true;
                }
            }

            

            // 계산은 매 스캔 수행하고, 출력만 시간으로 제한
            auto now = std::chrono::steady_clock::now();

            if (now - last_print >= std::chrono::milliseconds(500)) {
                if (found) {
                    std::cout << std::fixed << std::setprecision(2)
                              << "Scan " << scan + 1
                              << " | Minimum distance: "
                              << min_distance << " mm\n";
                } else {
                    std::cerr << "No valid distances in interval.\n";
                    exit_code = 1;
                }

                last_print = now;
                found = false;
                min_distance = 0.0f;

                // 실제로 출력한 경우에만 갱신
            }
        }
    }

    lidar->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    const auto stop_result = lidar->setMotorSpeed(0);

    if (SL_IS_FAIL(stop_result)) {
        std::cerr << "Motor stop failed.\n";
        exit_code = 1;
    }

    lidar->disconnect();
    std::cout << "Finished; motor stop requested.\n";

    return exit_code;
}