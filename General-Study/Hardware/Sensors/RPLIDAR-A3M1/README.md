# RPLIDAR A3M1: Windows에서 Jetson까지 C++ 데이터 취득

학습 기간: 2026-10-01~2026-10-05. 사용자 실행 로그와 실제 Windows 소스를 기준으로 정리했다. 개인 계정명, 장치 일련번호, 개인 PC 경로, 인증 정보와 실행 파일은 제외했다.

## 목차

- [실행 환경과 확인 수준](#실행-환경과-확인-수준)
- [Windows 연결과 C++ 실행](#windows-연결과-c-실행)
- [Jetson Ubuntu 24.04 연결](#jetson-ubuntu-2404-연결)
- [핵심 코드와 개념](#핵심-코드와-개념)
- [실행 결과](#실행-결과)
- [오류와 주의점](#오류와-주의점)
- [다음 할 일](#다음-할-일)
- [참고 자료](#참고-자료)

## 실행 환경과 확인 수준

| 항목 | 상태와 근거 |
| --- | --- |
| Windows + VS Code + MSYS2 UCRT64 g++ 15.2.0 | 실제 컴파일·실행 확인 |
| A3M1 + CP210x USB UART 어댑터 | Windows COM4, Jetson `/dev/ttyUSB0` 연결 확인 |
| Jetson Orin / Ubuntu 24.04 LTS | 사용자가 밝힌 환경. OS 릴리스·보드 SKU 출력은 별도 확인하지 않음 |
| ROS 2 sllidar_ros2 | 기존 설치로 실행 확인. 로그의 패키지 SDK 1.0.1 / LIDAR SDK 2.1.0. ROS 배포판 출력은 미확인 |
| Jetson 독립 C++ | 연결·상태·모터 제어·1회 및 10회 데이터 수신·최소 거리와 각도 출력 확인 |
| 출력 구간별 최솟값 / 약 2 Hz | Windows 실행 확인. Jetson에는 아직 이 기능을 적용·검증하지 않음 |
| CSV 저장 | 파일 열기와 헤더 쓰기 코드까지 안내. 최종 소스·파일 생성·10행 저장은 미확인 |

6축 센서 도착 전 보유한 라이다로 시작했다. UNO를 거치지 않고 USB UART 어댑터를 통해 호스트가 직접 데이터를 받는다. 카메라·깊이 카메라 연동 및 IMU 데이터 취득은 이 실습에서 수행하지 않았다.

6축은 가속도 3축 + 자이로 3축, 9축은 여기에 자기계 3축이 추가된다. 기초 데이터 취득에는 6축으로 충분하며, 자기계는 방위 보정에 도움이 되지만 주변 자기장 영향을 고려해야 한다. 기존 [IMU 노트](../IMU.md)와 연결한다.

## Windows 연결과 C++ 실행

연결: 랩탑 USB-C → USB 독 → USB-A/5핀 케이블 → 라이다 어댑터.

1. CP2102 USB UART 장치가 감지됐지만 드라이버 오류 코드 28, COM 포트 없음.
2. Silicon Labs 공식 CP210x Universal Windows Driver 설치.
3. 관리자 PowerShell의 `pnputil /add-driver <압축을-푼-드라이버-폴더>/silabser.inf /install`로 적용.
4. COM4 / 상태 OK 확인. RoboStudio에서 A3M1 연결 및 측정점 표시 확인.
5. RoboStudio를 종료한 뒤 독립 C++ 프로그램에서 COM4 / 256000 bps 연결.

명령의 `<...>`는 환경에 맞게 교체할 자리표시자다. 드라이버 설치는 관리자 권한이 필요했다. COM 번호는 다른 PC에서 달라질 수 있다. 어댑터에 속도 선택 스위치가 있는 경우 모델에 맞는 설정을 확인한다.

### 실제 소스

- [Windows lidar_read.cpp](windows/lidar_read.cpp): 현재 로컬 파일의 실제 코드. 잘못 남아 있던 주석만 동작에 맞게 정리했다.
- [mingw_compat.h](windows/mingw_compat.h): g++ 호환용 별도 헤더. SDK 원본은 수정하지 않았다.

Windows 소스는 10회 스캔을 하며 출력 구간 동안 최소 거리를 누적한다. 마지막 출력 뒤 남은 짧은 구간은 출력 없이 종료할 수 있다. 각도 저장 기능은 이 Windows 파일에는 없다.

Windows 빌드 구성은 g++ C++17, SDK `include`/`src` 헤더 경로, 최상위·`arch/win32`·`hal/thread.cpp`·`dataunpacker` 소스, `ws2_32`와 `winmm` 링크를 사용했다. `-UWIN32`, 강제 포함한 `mingw_compat.h`, 정적 GCC/C++ 런타임 옵션으로 빌드했다. 개인 PC 절대 경로가 들어간 빌드 스크립트와 VS Code 설정은 업로드하지 않았다.

F5의 “현재 파일만 빌드” 대신 SDK를 함께 빌드하는 사용자 작업을 사용했다. 소스 수정 뒤에는 재빌드해야 실행 파일에 반영된다.

## Jetson Ubuntu 24.04 연결

아래 경로는 개인 계정명을 제외한 재사용 가능한 표기다. 기존 ROS 작업 공간 위치가 다르면 변경한다.

```bash
lsusb
ls -l /dev/ttyUSB* /dev/ttyACM*
id -nG
command -v g++ make git
```

확인 결과:

- `10c4:ea60 Silicon Labs CP210x UART Bridge` 인식.
- `/dev/ttyUSB0`, 소유 그룹 `dialout`, 권한 `crw-rw----`.
- 사용자 계정이 `dialout`에 속해 sudo 없이 접근 가능.
- g++, make, git 존재. `ttyACM`이 없는 것은 이번 연결의 오류가 아님.
- 기존 `~/ros2_ws/src/sllidar_ros2/sdk/include/sl_lidar_driver.h` 발견.
- 기존 설치에 `sllidar_node`, `sllidar_client` 실행 파일 존재. 별도 SDK 재설치하지 않음.

### ROS로 신호 검증

확인한 A3 launch 기본값: serial, `/dev/ttyUSB0`, 256000, frame `laser`, `Sensitivity`, angle compensation 활성화.

```bash
source ~/ros2_ws/install/setup.bash
ros2 launch sllidar_ros2 sllidar_a3_launch.py
```

새 터미널에서:

```bash
source ~/ros2_ws/install/setup.bash
ros2 topic hz /scan
# 측정만 Ctrl+C로 종료; 라이다 launch 터미널은 유지
ros2 node list
ros2 node info /sllidar_node
ros2 topic echo /scan sensor_msgs/msg/LaserScan --once --qos-reliability best_effort
```

`echo /scan --once`가 한 번 자료형 발견에 실패했다. 노드 정보에서 `/scan: sensor_msgs/msg/LaserScan` 발행자를 확인하고 위 명령으로 실제 수신했다. 명시적 자료형과 QoS를 함께 바꿨으므로 실패 원인을 QoS로 단정하지 않는다.

로그의 설정은 scan frequency 10.0 Hz, sample rate 16 kHz, 최대 거리 25 m였지만 `/scan` 실제 수신은 약 12.32 Hz였다. 설정값·샘플링 속도·토픽 수신율을 구분한다. 한 메시지의 `scan_time=0.072548...`도 관측됐으며 이를 장시간 평균 수신율과 동일시하지 않는다.

LaserScan 예: `frame_id=laser`, 각도 약 −π~π rad, increment 0.0034926 rad, range_min 약 0.05 m, range_max 25 m, ranges 첫 값 약 5.056 m. `.inf`는 유효한 유한 거리로 계산하지 않는다. range_min은 드라이버 메시지 값이며 센서의 물리 사양을 새로 검증한 값이 아니다. 반복된 거리값은 원시 SDK 점과 각도 보상된 ROS 배열을 구분해서 해석한다.

### 독립 C++ 빌드

ROS launch 터미널에서 Ctrl+C로 노드를 종료하고 같은 시리얼 포트를 해제한다. 모터가 실제 멈췄는지도 확인한다.

```bash
cd ~/C++/proj
vi lidar_check.cpp
```

실제 사용한 SDK 소스 구성에 따라 빌드:

```bash
SDK_PATH="$HOME/ros2_ws/src/sllidar_ros2/sdk"

g++ -std=c++14 -pthread \
  -I"$SDK_PATH/include" \
  -I"$SDK_PATH/src" \
  lidar_check.cpp \
  "$SDK_PATH"/src/*.cpp \
  "$SDK_PATH"/src/arch/linux/*.cpp \
  "$SDK_PATH"/src/hal/*.cpp \
  "$SDK_PATH"/src/dataunpacker/*.cpp \
  "$SDK_PATH"/src/dataunpacker/unpacker/*.cpp \
  -o lidar_check && ./lidar_check
```

`-I`는 헤더 검색 경로, `-pthread`는 스레드 지원, `&&`는 빌드 성공 시 실행이다. `gcc lidar_check.cpp -o lidar_check`만으로는 SDK 헤더·구현과 C++ 링크 구성이 빠진다.

Jetson 최종 `lidar_check.cpp` 파일은 이 기록 작성 환경에서 직접 확보하지 못했다. 따라서 완성된 파일을 추정해 추가하지 않았다. 다음 절의 코드는 사용자가 제공한 실제 소스와 실행 확인된 변경 부분이다. CSV 코드가 반영된 최종 파일도 아직 확보하지 않았다.

## 핵심 코드와 개념

### 연결과 결과 검사

```cpp
std::unique_ptr<sl::IChannel> channel(
    *sl::createSerialPortChannel("/dev/ttyUSB0", 256000)
);
std::unique_ptr<sl::ILidarDriver> lidar(*sl::createLidarDriver());
auto result = lidar->connect(channel.get());
```

통신 객체 생성과 실제 연결은 별개다. `unique_ptr`는 소유한 객체를 자동 정리하고 `.get()`은 주소를 꺼낸다. `result`는 선언 후 재대입한다. 연결·상태 확인 전에 모터 명령을 실행하지 않는다.

`health{}`의 0은 초기값이다. `getHealth()` 성공을 확인한 후에만 장치 상태로 해석한다. `return 1`은 main을 종료하며 실패를 알리고, 오류 메시지 출력 자체는 실행을 중단하지 않는다.

### 수신과 단위 변환

```cpp
sl_lidar_response_measurement_node_hq_t nodes[8192];
size_t count = 8192;
result = lidar->grabScanDataHq(nodes, count, 3000);
```

count는 호출 전 수용 개수, 성공 후 실제 노드 개수다. 1274개면 유효 인덱스는 0~1273. 3000은 대기 제한 ms다.

```cpp
if (!nodes[i].dist_mm_q2 || !nodes[i].quality)
    continue;
const float distance_mm = nodes[i].dist_mm_q2 / 4.0f;
const float angle_deg = nodes[i].angle_z_q14 * 90.0f / 16384.0f;
```

거리 Q2는 정수값을 4로 나눠 mm로 복원한다. 8003 → 2000.75 mm. 0.25 mm 표현 간격이 실제 정확도를 뜻하지 않는다. 각도 32768 → 180°. SDK 각도는 센서 기준이며 북쪽 방위가 아니다. ROS LaserScan의 단위(m, rad)와 SDK 변환 단위(mm, deg)를 혼동하지 않는다.

### 스캔 전체의 최소 거리와 같은 점의 각도

사용자가 작성한 코드의 핵심:

```cpp
bool found = false;
float min_distance = 0.0f;
float min_angle = 0.0f;

for (size_t i = 0; i < count; ++i) {
    if (!nodes[i].dist_mm_q2 || !nodes[i].quality)
        continue;
    const float distance_mm = nodes[i].dist_mm_q2 / 4.0f;
    const float angle_deg = nodes[i].angle_z_q14 * 90.0f / 16384.0f;
    if (!found || distance_mm < min_distance) {
        min_distance = distance_mm;
        min_angle = angle_deg;
        found = true;
    }
}

if (found) {
    std::cout << "Minimum distance: " << min_distance
              << " mm | Angle: " << min_angle << " deg\n";
} else {
    std::cerr << "No valid distances.\n";
    exit_code = 1;
}
```

첫 유효 거리를 초기값으로 저장한다. `!found`는 반대 값을 계산할 뿐 변수를 변경하지 않는다. `||`는 첫 저장 또는 더 작은 거리라는 두 경우를 연결한다. `&&`로 바꾸면 초기값 0보다 작은 양수 거리를 요구하게 된다. 논리 연산은 단락 평가된다.

`<`는 동일한 최소 거리에서 앞 점을 유지하고, `<=`는 같은 거리의 뒤 점으로 갱신한다. 각도를 조건 밖에서 갱신하면 거리와 다른 점의 각도가 저장된다.

10회 스캔은 `for (int scan = 0; scan < 10; ++scan)`으로 수신·비교·출력을 감싼다. 모터와 측정은 한 번 시작하고 반복 종료 후 정지한다. 매 스캔의 최솟값은 found와 최소 거리·각도를 반복문 안에서 초기화한다.

### 수신·계산과 출력 빈도 분리

`i < count && printed < 10`은 스캔당 유효 점 10개 출력 제한이다. 전체 최솟값은 `i < count`로 모든 점을 확인해야 한다. continue는 이번 점을 건너뛰고, break는 가장 가까운 반복문을 종료한다.

Windows에서는 `steady_clock`으로 마지막 출력 이후 500 ms 이상 경과했는지 검사했다. 출력하지 않는 스캔도 계산한다. 출력 구간 전체의 최솟값은 found와 최솟값을 스캔 반복문 밖에 두고, 출력 직후 초기화한다. 매 스캔 초기화하면 최신 스캔 한 회의 최솟값이다.

```cpp
last_print = now;
found = false;
min_distance = 0.0f;
```

스캔 완료 시 확인하므로 엄밀한 2 Hz나 정확한 고정 500 ms 시간 창은 아니다. 첫 수신 대기가 500 ms를 넘으면 첫 스캔부터 출력할 수 있다. 종료 시 남은 짧은 구간 출력은 미구현이다.

### 종료

```cpp
lidar->stop();
std::this_thread::sleep_for(std::chrono::milliseconds(200));
result = lidar->setMotorSpeed(0);
lidar->disconnect();
```

측정 중지와 모터 정지는 별개다. 연결 해제 전에 정지를 요청한다. 200 ms는 제조사 예제 값을 따른 것이며 필수 최소 시간이나 산정 근거는 확인하지 않았다. 대기 자체가 정지 완료 확인은 아니다. 명령 성공 로그와 물리적 정지는 구분한다. 강제 종료·SIGINT 시 정지 처리는 미구현이다.

## 실행 결과

| 단계 | 확인한 결과 |
| --- | --- |
| Windows 5회 수신 | 1335~1387점, 각도·거리 표시, 종료 코드 0 |
| Windows 10회 최솟값 | 338~344 mm, 전체 비교와 표시 10개의 구분 확인 |
| Windows 구간 출력 | Scan 1: 339 mm, Scan 7: 336 mm, 정지 요청 |
| Jetson 상태·모터 | health=0, 시작·정지 요청 성공 |
| Jetson 1회 수신 | 1388점, 352.359° / 2080 mm 등 10점 표시 |
| Jetson 최소 점 | 1363점, 227 mm / 352.639°, 잘못 남은 printed 검사 제거 후 실행 |
| Jetson 10회 | 1305~1391점, 최소 227~229 mm, 약 139.5~140.2° |

최소 거리는 같은데 각도가 달라지면 동일 거리의 여러 점, 배열 순서, 설치 변화 등을 구분해야 한다. 로그만으로 같은 물체라고 단정하지 않는다. 기준 거리와의 정확도 비교는 미실시다.

## 오류와 주의점

- `sl_lidar.h: No such file`: SDK 헤더 경로를 지정하고 구현 소스도 함께 빌드한다.
- `result was not declared`: 모터 코드를 result 선언·연결·상태 확인 뒤로 이동한다.
- 세미콜론 누락은 다음 for 행에도 연쇄 오류를 낸다. else에는 조건을 직접 붙이지 않는다.
- 최솟값 뒤 No valid distances가 출력됨: 더 이상 증가시키지 않는 printed==0 검사가 남아 있었다. found로 통일했다.
- Windows SDK의 NOMINMAX 재정의·NULL 변환 경고가 있었다. 경고와 컴파일 실패를 구분한다.
- ROS·독립 프로그램·RoboStudio가 같은 시리얼 포트를 동시에 사용하지 않도록 한다.
- 지도 생성, SLAM, 카메라 융합은 이번 거리 취득과 별도 단계다.

## 다음 할 일

CSV 도입으로 아래 코드까지 안내했지만 사용자 반영과 파일 생성은 아직 확인하지 않았다.

```cpp
#include <fstream>
// main 내부, 모터 시작 전
std::ofstream csv("lidar_min.csv");
if (!csv.is_open()) {
    std::cerr << "Cannot open CSV file.\n";
    lidar->disconnect();
    return 1;
}
csv << "scan,min_distance_mm,min_angle_deg\n";
```

- [ ] Jetson의 실제 최종 lidar_check.cpp를 확보해 소스 파일로 추가.
- [ ] found가 참이면 스캔 번호·최소 거리·각도를 CSV로 기록.
- [ ] 헤더와 결과 10행, 단위, 쓰기 성공 확인. ofstream 기본 모드는 재실행 시 덮어쓴다.
- [ ] 긴 빌드 명령을 대체할 빌드 설정 작성·검증.
- [ ] Jetson에 출력 구간 최솟값과 표시 제한 적용·검증.
- [ ] 종료 시 남은 구간 출력, Ctrl+C 정지, 데이터 누락 처리를 검토.
- [ ] IMU·웹캠·깊이 카메라 연동은 모델과 SDK 확인 후 별도 착수.

## 참고 자료

- [SLAMTEC A3 제품 설명](https://www.slamtec.com/en/lidar/a3/)
- [제조사 자료·SDK·RoboStudio](https://www.slamtec.com/en/support#rplidar-a-series)
- [RPLIDAR C++ SDK](https://github.com/Slamtec/rplidar_sdk)
- [제조사 ultra_simple 예제](https://github.com/Slamtec/rplidar_sdk/blob/master/app/ultra_simple/main.cpp)
- [sllidar_ros2](https://github.com/Slamtec/sllidar_ros2)
- [Silicon Labs CP210x 드라이버](https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers)
- [ROS LaserScan 정의](https://github.com/ros2/common_interfaces/blob/jazzy/sensor_msgs/msg/LaserScan.msg)
- 소유 자료: A3M1 datasheet rev.1.10, Arduino UNO R3 자료. 개인 경로와 PDF 자체는 게시하지 않는다.
- [기존 2D LiDAR 노트](../2D-LiDAR.md)

SDK는 BSD 2-clause, 공식 데모는 GPLv3로 README에서 구분한다. SDK와 실행 파일 일체는 이 기록에 복사하지 않았다.
