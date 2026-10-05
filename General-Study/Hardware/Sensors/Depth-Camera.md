# Depth Camera


> **Depth Camera는 영상의 각 픽셀에 깊이값을 제공하는 카메라다.** RGB 영상이 색을 담는다면, 깊이 영상은 표면이 카메라로부터 얼마나 떨어져 있는지를 담는다. 보정된 깊이 영상과 카메라 내부 파라미터가 있으면 3차원 점군으로 변환할 수 있다.

학습일: 2026-10-03 · 목표: 기본 원리를 이해하고, 실제 깊이 영상에서 거리값을 검증한다.

## 1. RGB·IR·Depth는 무엇이 다른가?

| 데이터 | 픽셀의 의미 | 용도 |
| --- | --- | --- |
| RGB 영상 | 빨강·초록·파랑의 밝기 | 색·외형 확인, 객체 인식 |
| IR 영상 | 적외선 영상의 밝기 | 투사 패턴 또는 적외선 영상 관찰 |
| Depth 영상 | 각 픽셀 방향의 깊이값 | 거리 측정, 점군 생성, 장애물 인식 |

깊이 영상을 색으로 보여주는 화면은 거리값을 색에 대응시킨 시각화다. 화면의 빨강·파랑 자체가 물체의 실제 색이나 고정된 거리 단위를 뜻하지 않는다.

## 2. 깊이를 만드는 세 가지 대표 원리

### 구조광: 알려진 패턴을 투사

적외선 프로젝터가 점이나 무늬를 물체에 투사한다. IR 카메라는 그 패턴을 촬영하고, 보정된 기준 패턴과 관찰된 위치의 차이를 이용해 깊이를 계산한다. 프로젝터와 수광 카메라 사이의 기하 관계를 이용하는 삼각측량 계열이다.
Orbbec Astra 계열은 구조광 방식의 대표적인 예다. 다만 제품 이름만으로 해상도·거리 범위·SDK 호환성을 동일하게 가정하지 않는다. [Astra 계열 공식 설명](https://www.orbbec.com/products/structured-light-camera/astra-series/)

### 스테레오: 두 시점의 차이를 비교

간격이 떨어진 두 카메라에서 같은 점을 찾는다. 정렬된 두 영상에서 그 점의 가로 위치 차이를 시차 d라고 한다.
$$
Z = \frac{fB}{d}
$$
f는 픽셀 단위 초점거리, B는 두 카메라 사이 간격이다. 시차가 크면 가깝고, 작으면 멀다. 무늬가 부족한 표면은 대응점을 찾기 어려우며, 능동 스테레오는 투사 패턴으로 이를 보조한다. [Orbbec 기술 비교](https://www.orbbec.com/blog/decoding-depth-camera-performance-quantitative-evaluation-of-accuracy-and-precision/)

### ToF: 빛의 시간 또는 위상 차이를 측정

빛이 갔다 돌아오는 시간에 기반해 거리를 계산한다. 직접 ToF는 왕복 시간을, 간접 ToF는 변조된 빛의 위상 차이를 이용한다. 모든 Depth Camera가 ToF인 것은 아니다.
구조광은 패턴의 위치를, iToF는 시간에 관련된 위상을 활용한다. 강한 외부광·반사 특성·다중 반사 등에 대한 반응은 기술과 제품마다 다르다. [Orbbec 구조광·iToF 비교](https://www.orbbec.com/blog/structured-light-vs-itof-depth-cameras/)

## 3. 깊이 영상은 2차원 배열

영상 폭이 W, 높이가 H이면 H행 × W열의 깊이값이 있다. 픽셀 좌표를 (u, v)라고 할 때, 배열은 보통 행 v와 열 u 순서로 접근한다.

```c
/* 실제 카메라 데이터가 아닌 학습용 예시, 단위 mm */
uint16_t depth[2][3] = {
    {1000, 1100, 1200},
    {1300,    0, 1500}
};

```
이 예에서 depth\[0\]\[2\]는 1200 mm다. OpenNI 방식에서는 0을 유효하지 않은 깊이로 사용하므로 depth\[1\]\[1\]의 0을 “0 m에 물체가 있다”로 해석하면 안 된다.
ROS의 대표 규약은 32FC1일 때 m 단위 깊이, OpenNI의 16UC1일 때 mm 단위 깊이다. 원시 SDK 데이터는 별도의 scale을 사용할 수 있으므로 encoding과 드라이버 문서를 함께 확인한다. [ROS REP-118](https://github.com/ros-infrastructure/rep/blob/master/rep-0118.rst)

## 4. 픽셀을 3차원 좌표로 변환

렌즈 왜곡을 보정한 핀홀 모델에서:
$$
X = \frac{(u-c_x)Z}{f_x},\qquad
Y = \frac{(v-c_y)Z}{f_y}
$$
u·v는 픽셀 좌표, fx·fy는 초점거리, cx·cy는 주점이다. 사용할 깊이 영상의 해상도·보정 상태에 맞는 CameraInfo를 사용한다. 보정되지 않은 영상에는 먼저 왜곡 보정이 필요하다.
ROS 카메라 optical frame은 x가 오른쪽, y가 아래쪽, z가 정면이다. 로봇 좌표계와 축 방향이 달라 TF 변환이 필요하다. [ROS CameraInfo 정의](https://github.com/ros2/common_interfaces/blob/jazzy/sensor_msgs/msg/CameraInfo.msg)
REP-118의 깊이 Z는 광축 방향 거리다. 비스듬한 픽셀의 3차원 점까지 직선 거리는 √(X² + Y² + Z²)이므로 Z와 항상 같지는 않다.
RGB 카메라와 Depth 카메라는 광학 중심이 다를 수 있다. RGB에서 검출한 픽셀에 깊이값을 연결하려면 RGB-Depth 정렬 또는 두 카메라 사이 보정 변환이 필요하다. 별도의 USB 웹캠을 사용하면 그 웹캠과 깊이 카메라 사이의 외부 파라미터 및 시간 동기화도 확인한다.

## 5. 2D LiDAR와 비교

| 항목 | 2D LiDAR | Depth Camera |
| --- | --- | --- |
| 기본 데이터 | 각도별 거리 | 픽셀별 깊이 |
| 관측 형태 | 한 스캔 평면의 단면 | 카메라 시야 안의 표면 |
| 공간 표현 | 주로 평면 x·y | 보정 정보를 이용해 x·y·z |
| 관측 범위 | 제품별 각도 범위 | 수평·수직 FOV 안의 영역 |
| 로봇 활용 | 평면 장애물·지도·위치 추정 | 높이 있는 장애물·물체 위치·형상 |

Depth Camera도 물체의 뒤쪽이나 가려진 표면을 한 번에 보는 것은 아니다. 유리·반사체·범위 밖 영역 등에 구멍이나 잘못된 값이 생길 수 있다. 다른 센서와 결합할 때는 좌표계·시각·누락값을 함께 다룬다.
관련 노트: [2D LiDAR](2D-LiDAR.md)

## 6. Jetson에서 실제 사용하기

현재 이 노트는 원리와 확인 절차를 정리한 상태다. 연결 모델, 드라이버 실행, 깊이 단위, 실측 정확도는 아래 실습을 수행한 뒤 결과로 기록한다.

### 단계 1 — 장치 및 실행 환경 확인

Jetson에 카메라를 USB로 연결하고 다음 읽기 전용 명령을 실행한다.

```bash
lsusb
printenv ROS_DISTRO
ros2 pkg list | grep -Ei 'astra|orbbec|openni'
ros2 topic list -t

```
제품 라벨의 정확한 모델명도 확인한다. Astra·Astra 2·Astra+ 등은 드라이버 지원을 구분해야 한다. Orbbec SDK v2는 기존 OpenNI 장치를 지원하지 않으므로 실제 모델과 펌웨어를 기준으로 SDK·ROS 드라이버를 선택한다. [Orbbec SDK 지원표](https://github.com/orbbec/OrbbecSDK) · [ROS 2 래퍼 지원표](https://github.com/orbbec/OrbbecSDK_ROS2)

### 단계 2 — Depth 및 CameraInfo 확인

드라이버 실행 후 실제 토픽 이름을 확인한다. 다음은 이름 예시이며, 목록에 나온 실제 이름으로 바꾼다.

```bash
ros2 topic hz /camera/depth/image_raw
ros2 topic echo /camera/depth/image_raw --once --field encoding
ros2 topic echo /camera/depth/camera_info --once

```
확인 항목은 해상도, 프레임 주기, encoding, frame_id, 내부 파라미터다. RViz2의 Image로 깊이 영상을 확인한다. PointCloud2는 해당 토픽이 실제로 발행될 때 사용한다.

### 단계 3 — 줄자로 거리 검증

카메라 앞에 무광의 평평한 판을 놓는다. 카메라가 측정 가능한 범위 안에서 여러 거리를 선택해 중앙 영역의 깊이와 줄자 측정을 비교한다.
단일 픽셀만 보지 말고 작은 영역의 유효값 중앙값을 사용해 노이즈와 경계 혼합의 영향을 줄인다. 측정 기준점은 Depth 광학 중심으로 잡고, 모델의 기구 도면을 확인해 기준 오차를 기록한다.

| 실측 거리 | 깊이 측정값 | 차이 | 메모 |
| --- | --- | --- | --- |
| 미측정 | 미측정 | 미계산 | 실습 후 기록 |

- [ ] 정확한 카메라 모델·USB ID·SDK/드라이버 확인

- [ ] 깊이 영상과 CameraInfo 수신 확인

- [ ] 깊이 단위와 유효하지 않은 값 확인

- [ ] 실측 거리와 중앙 영역 깊이 비교

- [ ] 픽셀을 3차원 좌표로 변환

- [ ] RGB 객체 검출과 Depth 연결 전 정렬·보정 확인

---

원본: [Notion — Depth Camera](https://app.notion.com/p/3ee9b0b79c14801c895ceec388cd2244)

GitHub 정리일: 2026-10-05. 이 문서는 Notion 노트의 수동 스냅샷이며 자동 동기화되지 않습니다.
