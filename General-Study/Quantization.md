# 📉 Deep Learning Model Quantization (경량화 및 양자화)

딥러닝 모델의 연산 속도를 높이고 메모리 사용량을 줄이기 위해 high-precision 포맷(FP32)을 저정밀도 포맷(INT8 등)으로 변환하는 모델 경량화(Model Lightweighting) 기법에 대한 정리입니다.

---

## 1. 양자화(Quantization)의 필요성
* **메모리 대역폭 한계 극복:** 딥러닝 모델(특히 Transformer 계열)은 파라미터가 매우 많아 임베디드 장치나 제한된 자원을 가진 하드웨어에서 구동 시 DRAM 대역폭 병목이 발생함.
* **연산 속도 향상:** FP32(Floating Point 32-bit) 연산보다 INT8(Integer 8-bit) 연산이 하드웨어 레벨에서 연산 장치(ALU)의 면적을 적게 차지하고 소모 전력 및 지연 시간(Latency)이 훨씬 낮음.

---

## 2. 양자화의 기본 원리 (Mapping)
연속적인 실수 공간의 가중치(Weights)와 활성화 함수 출력값(Activation)을 이산적인 정수 공간으로 매핑하는 선형 매핑(Linear Mapping) 수식:

$$Q(x) = \text{round}\left(\frac{x}{S}\right) + Z$$

* **$S$ (Scale Factor):** 실수를 정수 범위로 축소하는 비율을 결정하는 실수 값.
* **$Z$ (Zero-Point):** 실수 공간의 `0`이 정수 공간에서 어떤 값에 매핑되는지 나타내는 편차(Offset).

---

## 3. 대표적인 양자화 방법론 분류

### ① PTQ (Post-Training Quantization, 학습 후 양자화)
* **특징:** 이미 학습이 완료된 모델의 FP32 가중치를 그대로 가져와서 INT8로 변환하는 방식.
* **장점:** 추가적인 재학습(Retraining)이 필요 없어 구현이 빠르고 비용이 적게 듦.
* **단점:** 정밀하게 튜닝되지 않으면 모델의 정확도(Accuracy)나 화질 지표(PSNR/SSIM 등)가 크게 저하될 수 있음. 미세한 주파수 성분이 중요한 영상 복원 모델에서는 성능 하락폭이 커질 수 있어 주의 필요.

### ② QAT (Quantization-Aware Training, 양자화 인식 학습)
* **특징:** 순방향 연산(Forward pass)에서는 양자화 손실(Quantization noise)을 모사하고, 역방향 연산(Backward pass)에서 이 손실을 감안하여 가중치를 학습시키는 방식.
* **장점:** 변환 후에도 모델의 원래 성능을 거의 그대로 보존할 수 있음. 성능 드롭에 민감한 고정밀 태스크에 필수적임.
* **단점:** 모델을 처음부터 다시 학습시키거나 파인튜닝해야 하므로 컴퓨팅 자원과 시간이 많이 소모됨.

---

## 📌 연구 및 실험 적용을 위한 Takeaway
* 이미지 복원(Image Restoration)이나 불량 검사 시스템과 같이 픽셀 단위의 정밀도가 중요한 도메인에서는 단순 PTQ 적용 시 Artifact(격자 무늬 등)가 발생할 가능성이 높음.
* 따라서 하드웨어 타겟팅 시 **QAT**를 도입하거나, 중요 레이어는 FP16(반정밀도)으로 남겨두는 **Mixed-Precision** 전략을 고려해야 함.
