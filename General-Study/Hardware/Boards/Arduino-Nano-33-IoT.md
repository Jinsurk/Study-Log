# Arduino Nano 33 IoT


> **Nano 33 IoT는 센서 읽기·입출력 제어·무선 통신을 수행하는 마이크로컨트롤러 보드다.** 중심은 SAMD21 MCU이며, Wi-Fi·Bluetooth 모듈과 6축 IMU가 함께 탑재되어 있다.

학습일: 2026-10-03 · 하드웨어 없이 작동 원리를 공부하는 노트

## 1. 보드 내부 구성

| 구성 | 사양 또는 기능 | 역할 |
| --- | --- | --- |
| MCU | SAMD21G18A, ARM Cortex-M0+, 32비트, 최대 48 MHz | 명령 실행 및 주변장치 제어 |
| Flash | 256 KB | 전원을 꺼도 프로그램 보관 |
| SRAM | 32 KB | 실행 중 변수·배열·스택 등 저장 |
| NINA-W102 | ESP32 기반 통신 모듈 | Wi-Fi·Bluetooth 통신 |
| 6축 IMU | 3축 가속도계 + 3축 자이로 | 가속도·기울기 관련 정보·회전 속도 측정 |
| ATECC608A | 암호화 보안 칩 | 암호 키 저장 및 보안 기능 지원 |

32비트는 처리 구조에 관한 표현이고, 48 MHz는 클록 주파수다. 둘 다 “프로그램을 초당 4,800만 번 완성한다”는 뜻은 아니다. 실제 처리 시간은 명령·메모리 접근·주변장치 대기 등에 달려 있다.
사양 출처: 사용자가 제공한 5쪽 PDF “Arduino 33 iot datasheet.pdf”, 1–3쪽. [Arduino 공식 제품 설명](https://docs.arduino.cc/hardware/nano-33-iot)

## 2. 프로그램이 하드웨어를 움직이는 과정

```mermaid
flowchart LR
    A["PC에서 코드 작성"] --> B["보드용으로 컴파일"]
    B --> C["USB로 펌웨어 업로드"]
    C --> D["Flash에 저장"]
    D --> E["MCU가 명령 실행"]
    E --> F["GPIO·ADC·타이머·통신 주변장치 동작"]

```
Jetson에서는 Linux 위에서 프로그램을 실행한다. Arduino의 일반적인 스케치는 Linux 없이 MCU에서 실행되며, Arduino 코어가 시작 코드와 주변장치 API를 제공한다.
변수와 배열은 MCU의 SRAM을 사용한다. 큰 영상 처리와 작은 센서 제어는 필요한 계산량과 메모리 규모가 다르므로, 로봇에서는 보드별 역할을 나누는 것이 유용하다.

## 3. 주요 입출력 기능

- **GPIO:** 핀을 디지털 입력 또는 출력으로 사용한다. 버튼 상태 읽기, 외부 드라이버에 제어 신호 전달 등에 쓴다.

- **ADC:** 아날로그 전압을 숫자로 변환한다. 전압이 곧 물리량은 아니므로 센서 변환식과 보정이 필요하다.

- **PWM:** 일정한 주기로 HIGH·LOW를 반복하고 HIGH 비율을 조절한다. 그 자체는 연속적인 아날로그 전압이 아니다.

- **DAC:** A0/DAC0에서 디지털 값을 아날로그 전압으로 변환한다. ADC와 역할이 반대다.

- **I²C:** SDA·SCL을 사용해 센서와 통신한다. 이 보드의 내장 IMU도 I²C로 연결된다.

- **SPI:** 클록과 데이터 선으로 통신한다. MCU와 NINA 모듈 사이에도 사용된다.

- **UART:** TX·RX를 사용하는 직렬 통신이다. 외부 장치와 데이터를 주고받을 수 있다.
GPIO는 모터에 전력을 공급하는 출력이 아니라 제어 신호를 내는 핀이다. 모터 구동에는 부하에 맞는 드라이버와 전원이 필요하다.

## 4. Arduino 코드의 setup과 loop

Arduino 스케치는 C++로 컴파일되며, C에서 배운 변수·배열·조건문·반복문·함수 개념을 그대로 활용할 수 있다. Arduino 코어가 main 흐름을 제공하므로 일반 스케치에서는 setup과 loop를 작성한다.

```c++
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);

    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}

```
setup은 시작 또는 리셋 후 한 번 실행된다. loop는 반복 호출된다. 이 예는 내장 LED를 0.5초 켜고 0.5초 끄는 흐름을 설명한다. 현재 보드에서 업로드·실행한 결과는 아니다.
delay는 해당 코드 흐름을 기다리게 한다. 여러 센서와 통신을 동시에 처리하려면 이후 타이머·인터럽트 또는 millis 기반의 시간 관리를 배운다.
Nano 33 IoT에는 SAMD 보드 패키지가 필요하다. UNO의 AVR 패키지와 구분한다. [Arduino 보드 패키지 설명](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-board-manager)

## 5. IMU는 무엇을 측정하는가?

- 가속도계는 x·y·z 방향의 specific force를 측정한다. 정지 상태에서는 중력에 따른 성분으로 기울기를 추정할 수 있지만, 움직이는 동안의 가속도와 구분해야 한다.

- 자이로는 x·y·z축 주위의 각속도를 측정한다. 각속도를 시간에 따라 누적하면 각도 변화를 추정할 수 있지만 오차가 쌓인다.

- “6축”은 가속도 3축 + 각속도 3축이다. 6개의 위치 좌표나 절대 위치를 직접 제공한다는 뜻이 아니다.

- 이 구성에는 자기계가 포함되지 않는다. 절대 방위와 이동 위치를 얻으려면 추가 정보·보정·센서 융합이 필요하다.
**문서 간 표기 차이:** 사용자 제공 PDF는 IMU를 LSM6DSL로 적지만 공식 제품 페이지와 핀아웃은 LSM6DS3로 안내한다. 라이브러리·레지스터 실습 전 실제 보드 리비전과 회로도를 확인한다. 여기서는 두 경우에 공통인 6축 IMU 역할까지만 확정한다. [공식 제품 페이지](https://docs.arduino.cc/hardware/nano-33-iot) · [공식 핀아웃](https://docs.arduino.cc/resources/pinouts/ABX00027-full-pinout.pdf)

## 6. Jetson과 역할을 나누는 예시

| Jetson Orin Nano | Nano 33 IoT |
| --- | --- |
| RGB·Depth 영상 처리 | 센서·스위치·IMU 읽기 |
| 객체 인식, 지도·경로 관련 계산 | 타이머·PWM·입출력 처리 |
| 상위 동작 명령 생성 | 모터 드라이버 등에 명령 전달 |
| 센서 데이터를 수신해 기록·판단 | 측정값과 상태를 Jetson에 전송 |

이는 설계 예시이며 실제 제어 구성을 확인한 결과는 아니다. 처음에는 USB 직렬 통신으로 작은 센서 데이터를 주고받는 방식부터 검증할 수 있다. Wi-Fi·BLE 탑재 여부만으로 지연 시간이나 제어 안정성이 보장되지는 않는다.
관련 센서 노트: [2D LiDAR](../Sensors/2D-LiDAR.md) · [Depth Camera](../Sensors/Depth-Camera.md)

## 7. UNO와의 차이 및 전기적 조건

Nano 33 IoT의 GPIO는 **3.3V이며 5V tolerant가 아니다.** 핀 배열이 기존 Nano와 비슷해도 5V 신호를 직접 넣으면 안 된다.
VUSB/5V 핀은 기본적으로 연결되지 않으며, 제공 PDF는 USB 전원과 VUSB 납땜 브리지가 모두 있어야 USB의 5V가 해당 핀에 전달된다고 설명한다. VIN 전원에서 규제된 5V 출력이 자동으로 생기는 구조가 아니다.
기존 커리큘럼의 UNO는 ATmega328P 기반이다. SAMD21의 레지스터·타이머·인터럽트 구조가 다르므로 DDRB·PORTB 같은 AVR 코드를 그대로 가져올 수 없다. C 개념은 이어지지만 하드웨어 제어 코드는 각 MCU 문서에 맞춰 작성한다.

## 이해 확인

- [ ] Flash와 SRAM의 역할을 구분한다.

- [ ] MCU와 Wi-Fi 모듈의 역할을 구분한다.

- [ ] PWM과 아날로그 전압의 차이를 설명한다.

- [ ] 6축 IMU가 측정하는 양을 설명한다.

- [ ] Jetson과 Arduino 사이에 어떤 데이터를 주고받을지 정한다.
다음 학습: GPIO가 HIGH·LOW를 만드는 원리 → ADC로 전압 읽기 → IMU 데이터 의미 → USB 직렬 통신.

---

원본: [Notion — Arduino Nano 33 IoT](https://app.notion.com/p/3ee9b0b79c1480e983d1c4cc39931cca)

GitHub 정리일: 2026-10-05. 이 문서는 Notion 노트의 수동 스냅샷이며 자동 동기화되지 않습니다.
