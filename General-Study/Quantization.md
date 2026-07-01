# 📉 Deep Learning Model Quantization (양자화)

## 1. 정의 및 필요성
* **개념:** 딥러닝 모델의 **가중치(Weight)**와 **활성값(Activation)**을 부동소수점(FP32/FP16) 대신 **저정밀 정수(INT8, INT4)** 또는 **특수 저비트 부동소수점(NF4)**으로 근사하여 모델 크기를 줄이고 추론 속도를 개선하는 기법이다.
* **장점:**
    * **메모리/저장소 절감:** FP32 ➔ INT8 변환 시 대략 4배 이상 크기가 축소되어 온디바이스 및 엣지 배포 가능성이 확대됨.
    * **추론 속도 향상:** 정수 연산 선호 하드웨어(CPU, 모바일, 엣지)에서 지연 시간(Latency) 감소.
    * **전력 소비 절감:** 배터리 기반 장치에서의 효율성 증대.

---

## 2. 기본 원리 및 수식
연속적인 실수 값을 제한된 개수의 정수로 변환하는 매핑 과정이다. (예: 실수 범위를 256개의 정수 구간인 INT8로 근사)

### 📌 핵심 매개변수
* **스케일 ($S$):** "실수 세계의 한 단위가 정수 세계에서 얼마나 큰가"를 나타내는 배율 계수. 스케일이 크면 넓은 범위를 표현하지만 정밀도가 낮아지고, 작으면 좁은 범위를 정밀하게 표현함.
* **영점 ($Z$):** "실수 0이 정수 세계에서 어떤 값에 대응되는가"를 나타내는 편차(Offset). 비대칭 양자화에서 평형 이동을 통해 음수/양수를 모두 표현하며, 패딩 및 합성곱 연산 시 0의 정확한 표현이 중요함.

### 📐 연산 수식
* **양자화 (Quantization):**
$$x_q = \text{clip}\left(\text{round}\left(\frac{x}{S}\right) + Z, \, q_{min}, \, q_{max}\right)$$

* **역양자화 (Dequantization):**
$$\hat{x} = S(x_q - Z)$$

> 💡 `clip` 함수는 변환된 값이 표현 가능한 범위($q_{min} \sim q_{max}$)를 벗어나면 경계값으로 조정해 준다. (예: 계산 결과가 130이면 INT8의 최대값인 127로 제한)

---

## 3. 양자화 유형 및 분류

### 🌓 대칭 vs 비대칭 (Symmetric vs Asymmetric)
* **대칭 (Symmetric):** $Z = 0$으로 설정하여 범위를 0 중심으로 대칭 설정. 주로 **가중치(Weight)**에 사용.
* **비대칭 (Asymmetric):** $Z \neq 0$으로 설정하여 실제 데이터 분포에 맞춰 영점을 이동. 주로 **활성값(Activation)**에 사용.

### 🪓 세분화 단위 (Granularity)
* **Per-Tensor:** 텐서 전체에 단일한 ($S, Z$)를 적용. 구현이 단순하지만 정확도 손실 가능성이 큼.
* **Per-Channel:** 각 채널별로 독립적인 ($S, Z$)를 적용. 계산 비용은 증가하나 정확도 보전에 유리 (특히 LLM 가중치에 필수적).

---

## 4. 양자화 진행 절차 (Three-Step)
1. **교정 (Calibration):** 모델이 실제로 사용할 값의 범위를 파악하는 단계. 대표 데이터(예: 100장의 샘플 이미지)를 모델에 통과시켜 각 층의 가중치와 활성값이 어떤 범위($x_{min} \sim x_{max}$)에 분포하는지 관찰하고, 이를 기반으로 스케일과 영점을 산정한다.
$$S = \frac{x_{max} - x_{min}}{q_{max} - q_{min}}, \quad Z = q_{min} - \frac{x_{min}}{S}$$

2. **양자화 (Quantization):** 계산된 스케일과 영점을 사용해 실수를 정수로 변환한다.

3. **역양자화 (Dequantization):** 추론 시 필요에 따라 정수를 다시 실수 근사치($\hat{x}$)로 복원하여 연산을 수행한다. 완벽한 복원은 불가능하지만, 적절한 스케일과 영점을 사용하면 정확도 손실을 최소화할 수 있다.

---

## 5. 주요 방법론 및 최신 기법

### 1) PTQ (Post-Training Quantization)
학습이 완료된 모델을 교정(Calibration)만으로 양자화하는 기법이다. LLM에서도 8비트(W8A8)나 저비트 가중치 전용 등 다양한 변형이 실용화되었다.
* **SmoothQuant:** 활성값의 Outlier(이상치)를 부드럽게 만들어 W8A8 양자화에서 정확도와 하드웨어 효율을 동시 확보.
* **AWQ (Activation-aware Weight Quantization):** 활성 통계를 기반으로 중요한 채널만 보호/스케일링하여 4비트에서도 성능을 보전.
* **GPTQ:** 2차 근사 정보를 활용한 원샷(One-shot) 가중치 양자화로 3~4비트까지 정밀도 유지.

### 2) QAT (Quantization-Aware Training)
* 훈련 과정 중에 양자화 효과를 모사(Fake Quantization)하여 학습함으로써 정확도 손실을 최소화하는 방식이다. CNN부터 모바일 배포까지 폭넓게 활용된다.

### 3) LLM 특화 기법
* **QLoRA:** NF4(정규분포 가정 최적 데이터 타입) + Double Quantization + LoRA 미세조정을 결합하여 메모리를 극적으로 절감하고, 4비트 미세조정을 실용화한 기법이다.

---

## ⚠️ 한계점 및 주의사항
* **정확도 저하 위험:** 활성값의 Outlier, LayerNorm/어텐션 분포 등에 민감하므로 채널별 스케일링이나 Outlier 처리 등 기법별 보정이 필수적이다.
* **하드웨어 제약:** 연산자별 INT 지원 여부, 커널 구현 상태, 혼합정밀도(Mixed-Precision) 경계에서의 전환 비용을 고려해야 한다.

---

## 6. 실전 적용 기록: DWT-CAT 모델의 Jetson Orin Nano TensorRT 배포 준비

### 목표
DWT-CAT 기반 이미지 복원 모델을 Jetson Orin Nano Developer Kit에서 TensorRT로 실행하기 위한 배포 환경을 준비했다. 현재 단계의 목표는 INT8 양자화가 아니라, 먼저 **FP16 TensorRT engine**을 안정적으로 생성할 수 있는 ONNX 모델과 빌드 스크립트를 준비하는 것이다.

### 원본 프로젝트 및 배포 폴더
원본 프로젝트는 GPU 서버의 다음 경로에 있다.

```text
/data2/jinseok/projects/xavis_dwt_swinir
```

배포 준비용 폴더는 원본 프로젝트와 분리했다.

```text
/home/jinseok/08_quantization_deploy
```

원본 학습/실험 폴더는 수정하지 않고, 필요한 파일만 별도 deploy 폴더로 복사하는 방식으로 진행했다.

### 선택 모델
배포 후보로 선택한 모델은 다음과 같다.

```text
DWT-CAT B2-C156
```

원본 실험 경로는 다음과 같다.

```text
/data2/jinseok/projects/xavis_dwt_swinir/experiments/efficient_cat/cat_b2_c156_n1-6_n2-2_c156
```

복사한 핵심 파일은 다음과 같다.

```text
best_model.pth
-> /home/jinseok/08_quantization_deploy/checkpoints/best_model.pth

resolved_config.yaml
-> /home/jinseok/08_quantization_deploy/configs/dwt_cat_config.yaml
```

성능 기록 및 로그도 참고용으로 복사했다.

```text
test_summary.txt -> reports/source_test_summary.txt
test_metrics.csv -> reports/source_test_metrics.csv
train.log -> reports/source_train.log
training_log.txt -> reports/source_training_log.txt
```

### Python 환경
기존 conda 환경 `torch`를 사용했다. 새 VSCode venv는 만들지 않았다.

```text
Python: /home/jinseok/miniconda3/envs/torch/bin/python
PyTorch: 2.2.2+cu118
ONNX: 1.19.1
ONNXRuntime: 1.19.2
```

### 입력/출력 기준
처음에는 fallback config 때문에 RGB 입력 `(1, 3, 64, 64)`로 잘못 진행될 위험이 있었다. 그러나 실제 DWT-CAT 모델은 RGB image-space가 아니라 DWT-domain 4채널 입력을 사용한다.

따라서 deploy script는 DWT 4채널 입력만 허용하도록 수정했다.

```text
input shape:  (1, 4, 64, 64)
output shape: (1, 4, 64, 64)
```

즉, 이 모델에서 calibration이나 TensorRT 변환을 진행할 때도 원본 RGB 이미지가 아니라 실제 모델 입력인 DWT 4채널 tensor 분포를 기준으로 생각해야 한다.

### ONNX export 검증
실행한 검증 명령은 다음과 같다.

```bash
/home/jinseok/miniconda3/envs/torch/bin/python scripts/inspect_model.py --height 64 --width 64 --strict

/home/jinseok/miniconda3/envs/torch/bin/python scripts/export_onnx.py \
  --height 64 \
  --width 64 \
  --opset 17 \
  --output models/dwt_cat_b2_c156_fp32.onnx

/home/jinseok/miniconda3/envs/torch/bin/python scripts/infer_onnx.py \
  --onnx models/dwt_cat_b2_c156_fp32.onnx \
  --height 64 \
  --width 64
```

검증 결과는 다음과 같다.

```text
checkpoint strict load: 성공
PyTorch forward: 성공
ONNX export: 성공
ONNXRuntime 비교: 성공
```

생성된 ONNX 파일은 다음 위치에 있다.

```text
/home/jinseok/08_quantization_deploy/models/dwt_cat_b2_c156_fp32.onnx
```

PyTorch와 ONNXRuntime 출력 차이는 매우 작았다.

```text
mean abs error: 0.00000035
max abs error: 0.00000429
```

따라서 ONNX 변환은 Jetson FP16 TensorRT build로 넘어갈 수 있는 수준으로 통과했다고 판단했다.

### Jetson FP16 TensorRT build 계획
TensorRT engine은 GPU architecture와 TensorRT version에 의존하므로 서버에서 만들지 않고 Jetson에서 직접 생성해야 한다.

Jetson에서 실행할 예정 명령은 다음과 같다.

```bash
cd /home/jinseok/08_quantization_deploy

bash scripts/build_fp16_engine.sh \
  models/dwt_cat_b2_c156_fp32.onnx \
  engines/dwt_cat_b2_c156_fp16.engine \
  reports/trt_fp16_build_log.txt
```

build script의 핵심 설정은 다음과 같다.

```text
trtexec --fp16
static shape: input:1x4x64x64
--verbose
--dumpLayerInfo
--profilingVerbosity=detailed
```

### Jetson에 옮길 최소 파일
FP16 engine build에 필요한 최소 파일은 다음과 같다.

```text
models/dwt_cat_b2_c156_fp32.onnx
scripts/build_fp16_engine.sh
configs/dwt_cat_config.yaml
reports/onnx_export_report.md
README.md
```

선택적으로 옮길 파일은 다음과 같다.

```text
reports/source_test_summary.txt
reports/source_test_metrics.csv
reports/trt_fp16_build_plan.md
```

처음 FP16 build에는 다음 파일들이 필수는 아니다.

```text
checkpoints/best_model.pth
전체 dataset
전체 experiments 폴더
서버에서 만든 TensorRT engine
```

### 현재 상태
현재까지의 진행 상태는 다음과 같다.

```text
GPU 서버 deploy 환경 구성: 완료
PyTorch forward: 완료
ONNX export: 완료
ONNXRuntime 검증: 완료
Jetson 전송: 아직 안 함
Jetson FP16 TensorRT build: 아직 안 함
INT8 PTQ: 아직 시작 안 함
```

### 다음 단계
1. Jetson에서 `trtexec`와 TensorRT 설치 상태를 확인한다.
2. 최소 deploy package를 Jetson으로 전송한다.
3. Jetson에서 FP16 TensorRT engine을 생성한다.
4. binding 정보를 확인한다.

```text
input binding: input
input shape: 1x4x64x64
output binding: output
expected output shape: 1x4x64x64
```

5. FP16 engine inference 결과를 검증한다.
6. FP16 경로가 성공한 뒤에 INT8 PTQ calibration을 준비한다.

### 정리
DWT-CAT 모델의 양자화는 바로 INT8부터 시작하지 않고, 다음 순서로 진행하는 것이 안전하다.

```text
PyTorch checkpoint 검증
-> ONNX export
-> ONNXRuntime 비교
-> Jetson FP16 TensorRT engine build
-> FP16 inference 검증
-> INT8 PTQ calibration
-> 필요 시 mixed precision 또는 QAT 검토
```

특히 이 모델은 DWT 4채널 입력을 사용하므로, 이후 INT8 calibration도 RGB 이미지가 아니라 실제 모델 입력인 DWT-domain tensor를 기준으로 진행해야 한다.
