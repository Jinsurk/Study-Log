# Hardware 학습 노트

센서가 측정하는 물리량, 데이터의 의미, 로봇에서의 활용과 한계를 정리한다.

## Sensors

- [2D LiDAR](Sensors/2D-LiDAR.md) — ToF·삼각측량, 거리와 각도, LaserScan
- [Depth Camera](Sensors/Depth-Camera.md) — 구조광·스테레오·ToF, 깊이 배열, 좌표 변환
- [IMU](Sensors/IMU.md) — MEMS 가속도계·자이로, 드리프트, 영점 보정

## Boards

- [Arduino Nano 33 IoT](Boards/Arduino-Nano-33-IoT.md) — MCU, 메모리, 입출력, 전기적 조건

## 기록 원칙

- 센서 개념 문서는 원리 중심이며 실기 기록은 확인 결과와 미확인을 구분한다. 제안된 절차를 실행 완료로 해석하지 않는다.
- 사양과 SDK 지원은 실제 모델·리비전을 기준으로 다시 확인한다.
- 측정 결과는 장비·환경·단위·명령·출력을 함께 기록한다.
- 그림이 필요하면 각 분야의 `assets/`에 추가하고 문서에서 상대 경로로 연결한다.
- Notion → GitHub는 현재 수동 정리 방식이다. 원본 링크는 각 문서 하단에 있다.

## 실기·코드 실습

- [RPLIDAR A3M1 C++ 데이터 취득](Sensors/RPLIDAR-A3M1/README.md) — 연결, 수신, 최소 거리·각도, CSV 다음 단계
- [Nano 33 IoT IMU 보정](Sensors/IMU-Calibration/README.md) — Windows CSV 수집, 보정 모델, 실제 검증 결과
