# EWT: Efficient Wavelet-Transformer for Single Image Denoising

## 논문 정보

- **저자:** Juncheng Li, Bodong Cheng, Ying Chen, Guangwei Gao, Tieyong Zeng
- **발표 연도 / 학회:** 2023년 4월 arXiv v1. 서지 검색에서 *Neural Networks* 177권(2024), 논문 번호 106378의 저널판이 확인되나, 저널판 본문은 검토하지 않았다(저자 구성 등 차이 미확인).
- **논문:** [arXiv:2304.06274](https://arxiv.org/abs/2304.06274)
- **공식 코드:** 미확인. 논문은 "All code will be public"이라고 적었으나 공개 저장소는 찾지 못했다(2026-11-16 검색 기준).
- **키워드:** Image Denoising, DWT, Wavelet, Transformer, CNN, Efficiency
- **리뷰 작성일:** 2026-11-03 (2026-11-16 보완)
- **검토한 논문 버전:** arXiv v1 (2023-04-13, 12쪽)

## 1. 한 줄 요약

DWT로 입력을 네 주파수 대역으로 분해해 해상도를 절반으로 줄이고, CNN과 윈도우 기반 Transformer를 병렬로 결합해 복원 성능과 연산 효율(시간·GPU 메모리)의 균형을 추구한 denoising 모델이다.

## 2. 연구 배경과 문제 정의

고해상도 영상에 Transformer를 적용하면 실행 시간과 GPU 메모리 부담이 크다. 저자들은 SwinIR처럼 Transformer 블록을 많이 쌓은 모델이 성능은 좋지만 자원 소모가 커서, 성능 향상과 자원 소비 사이의 균형을 이루지 못한다고 본다.

해상도를 줄이는 기존 방법(pooling, stride convolution, 크롭)은 정보 손실이나 전역 정보 손실을 일으킨다. 저자들은 가역적인 DWT를 다운샘플링으로, IWT를 업샘플링으로 사용해 이 문제를 피하려 한다. 또한 Transformer 비중이 너무 높으면 자원 대비 성능이 나빠진다고 보고, CNN 분기를 함께 쓰는 구조를 설계했다.

## 3. 핵심 아이디어 및 기여

논문이 주장하는 기여는 다음과 같다.

1. **DWT/IWT 샘플링:** 입력에 DWT를 적용하고 출력에서 IWT로 원래 해상도를 복원한다. 저자들은 이것을 "웨이블릿 영역에서 Transformer를 사용한 첫 시도"라고 주장한다(리뷰어가 검증하지 않은 주장).
2. **DFEB (Dual-stream Feature Extraction Block):** CNN 분기와 Transformer 분기를 병렬로 사용하고 출력을 결합해 특징을 융합한다.
3. **MFAM (Multi-level Feature Aggregation Module):** 여러 DFEB의 출력을 모두 더해 집계하고, 지역·전역 잔차 연결을 사용한다.
4. **효율:** 원래 Transformer 대비 속도 80% 이상 향상, GPU 메모리 60% 이상 절감을 주장한다(근거는 Table V).

## 4. 모델 구조와 동작 과정

```text
잡음 영상 (H×W×C)
  → DWT → LL, LH, HL, HH 결합 = Fe (H/2×W/2×4C)
  → 3×3 Conv → MFAM ─┬─ DFEB ×4 (각 출력을 모두 합산)
                     └─ ConvBlock → MFAM 입력과 잔차 합산
  → 3×3 Conv → Fe와 잔차 합산 (웨이블릿 계수 공간)
  → IWT → 복원 영상 (H×W×C)
```

### DWT/IWT

DWT는 영상을 저주파 LL과 세 방향 고주파 LH, HL, HH로 분해한다. 각 대역의 가로·세로 크기는 원본의 절반이며, 네 대역을 채널 방향으로 결합한다(식 1). 저자들은 장점으로 (1) 가역성, (2) 주파수·위치 정보 포착, (3) 해상도 축소에 따른 메모리 절감과 추가 파라미터 없음, (4) 상대적인 수용 영역 확대를 든다(3.B절).

- 사용한 웨이블릿 종류는 "single-scale wavelet"이라고만 적혀 있고 **Haar 등 필터 종류는 명시되지 않았다**(미확인).
- DWT 자체는 가역적이지만, 학습된 전체 네트워크의 완벽한 정보 보존을 보장하지는 않는다.

### MFAM과 DFEB

- **MFAM:** N개 DFEB 출력을 합산(Fd = ΣFi)하고 ConvBlock을 거친 뒤 MFAM 입력에 더한다(식 8–10).
- **DFEB:** 입력 특징을 두 그룹으로 나눈다.
  - **CNN 분기(surface information):** ConvBlock 1개. Conv–ReLU–Conv 출력에 λ = 0.1을 곱해 입력에 더한다.
  - **Transformer 분기(fine-grained information):** Swin의 윈도우 및 시프트 윈도우 attention((S)W-MSA) 블록 6개. 전체 영상에 전역 attention을 적용하지 않는다.
  - 두 분기 출력을 더하지 않고 **concat한 뒤 Conv로 융합**한다. 저자들은 두 분기의 작동 방식이 달라 단순 합산은 정보 혼동을 일으킨다고 설명한다.

### 최종 설정 (4.B절)

| 항목 | 값 |
|---|---|
| DWT 단계 | 1단계 |
| DFEB 개수 | 4 |
| DFEB당 Transformer 블록 | 6 (+ ConvBlock 1) |
| 임베딩 차원 | 180 |
| 윈도우 크기 / head | 8 / 6 |
| MLP 차원 | 임베딩의 2배 |
| 합성곱 커널 | 모두 3×3 |
| 손실 함수 | L1 (복원 영상 기준, 식 7) |
| 파라미터 | 약 11.8M |
| 학습 | 노이즈 패치 16개 무작위 선택, 회전·뒤집기 증강, Adam |

패치 크기, 학습률, 반복 횟수, 학습·측정 GPU 종류는 본문에 명시되지 않았다(미확인). 구성 요소 실험은 패치 크기 64를 사용했다.

## 5. 실험 설정 및 주요 결과

- **학습:** DIV2K 학습 영상 800장. AWGN을 더해 잡음 영상을 만든다.
- **평가:**
  - 회색조 Set12, BSD68, Urban100 (σ = 15, 25, 50)
  - 컬러 Kodak24, CBSD68, CUrban100 (σ = 10, 30, 50)
  - 실제 잡음 SIDD, RNI15
- **지표:** PSNR 중심. SSIM은 보고하지 않는다.

### 합성 Gaussian 잡음 (최종 11.8M 모델)

| 비교 | EWT | 비교 모델 | 출처 |
|---|---:|---:|---|
| 회색조 9개 조건 평균 | 30.22 dB | MWCNN 30.08 dB (19.2M) | Table I, XI |
| 컬러 9개 조건 평균 | 32.48 dB | RDN 32.45 dB (약 22M) | Table II |
| CBSD68 σ = 50 | 28.36 dB | DIDN 28.35 dB (165M) | Table X, Fig. 1 |

- MWCNN 대비 BSD68 σ = 25, 50에서는 EWT가 낮다(29.40 vs 29.41, 26.47 vs 26.53 dB). 저자들은 MWCNN이 학습 영상을 약 7배(5,744장) 더 썼다고 지적한다.
- Kodak24에서는 RDN, DHDN, DIDN보다 약간 낮다(Table II, X).

### SwinIR와의 효율 비교

| 모델 | 파라미터 | PSNR | 실행 시간 | GPU 메모리 |
|---|---:|---:|---:|---:|
| SwinIR* | 5.17M | 31.79 dB | 53.29초 | 18,432 MiB |
| DWT + SwinIR* | 5.20M | 31.57 dB | 12.52초 | 8,276 MiB |
| EWT* | 5.18M | 31.73 dB | 9.14초 | 6,347 MiB |

Kodak24, 컬러, Gaussian σ = 30, 패치 크기 56, 논문 Table V. 별표 모델은 비교를 위해 임베딩 차원을 180에서 120으로 줄이고 같은 데이터·설정으로 재학습한 모델이다. EWT*는 최종 11.8M 모델이 아니다.

- EWT*는 SwinIR* 대비 PSNR이 0.06 dB 낮고, 실행 시간은 약 83%, GPU 메모리는 약 66% 적다.
- 논문은 GPU 수치를 학습 시 사용량으로, 시간은 평균 실행 시간으로 설명한다. 측정 장비는 명시되지 않았다.
- **DWT를 SwinIR에 그대로 붙이면 PSNR이 0.22 dB 낮아졌다.** 저자들은 이를 구조 최적화 부재 때문으로 해석한다.

### Uformer·Restormer 비교 (Table IX, σ = 30, 같은 데이터로 재학습)

| 모델 | 파라미터 | GPU | Kodak24 | CBSD68 | CUrban100 |
|---|---:|---:|---:|---:|---:|
| SwinIR | 5.17M | 18,432 MiB | 31.79 dB | 30.64 dB | 31.36 dB |
| Uformer | 5.28M | 6,875 MiB | 31.57 dB | 30.07 dB | 30.82 dB |
| Restormer | 12.47M | 21,894 MiB | 31.62 dB | 30.51 dB | 31.16 dB |
| EWT | 5.18M | 6,347 MiB | 31.73 dB | 30.60 dB | 31.35 dB |

CUrban100 실행 시간은 SwinIR 232.46초, Uformer 44.50초, Restormer 215.02초, EWT 43.77초다. 세 데이터셋 모두 SwinIR가 PSNR이 가장 높다.

### 기타 잡음

- **Poisson·Speckle (Table III):** SwinIR, Uformer, Restormer와 비교해 6개 조건 중 4개에서 가장 높거나 같다. Urban100에서는 Restormer나 SwinIR가 더 높다. 잡음 강도 설정은 본문에 명시되지 않았다(미확인).
- **실제 잡음 SIDD (Table IV):** 39.52 dB로, 표의 CycleISP(39.52 dB)와 같다. 추가 학습 데이터 없이 얻은 결과라고 설명한다. 어떤 데이터로 실제 잡음 모델을 학습했는지는 명시되지 않았다(미확인).

### 구성 요소 실험 (Kodak24, 컬러, σ = 30)

- **DWT 단계 (Table VI, 패치 64):**

  | 단계 | 시간 | GPU | FLOPs | PSNR |
  |---|---:|---:|---:|---:|
  | 1 | 11.96초 | 7,636 MiB | 17.82G | 31.78 dB |
  | 2 | 3.09초 | 3,658 MiB | 4.50G | 31.62 dB |
  | 3 | 1.91초 | 2,758 MiB | 1.18G | 27.94 dB |

  단계가 늘면 자원은 크게 줄지만, 3단계에서는 성능이 크게 떨어진다.
- **분기 조합 (Table VII, DFEB 2개):**
  - PSNR: CNN+CNN 31.12, Transformer+Transformer 31.66, CNN+Transformer 31.72 dB
  - CNN+CNN이 가장 빠르고 메모리 사용량이 적다(2.44초, 1,459 MiB). 반면 파라미터와 FLOPs는 가장 많다.
- **DFEB 개수 (Table VIII):**
  - PSNR: 1/2/3/4개에서 31.55/31.72/31.75/31.78 dB
  - 시간: 3.01초에서 11.96초로 증가
  - 2개 이후로는 PSNR 향상 폭이 작다.

## 6. 강점과 한계

### 강점

- 해상도 축소와 웨이블릿 대역 활용을 결합해 Transformer의 자원 부담을 줄이려는 설계가 명확하다.
- PSNR과 함께 파라미터, 실행 시간, GPU 메모리, FLOPs를 비교한다.
- DWT 단계, 분기 조합, DFEB 개수에 대한 ablation을 제공한다.
- SwinIR, Uformer, Restormer를 같은 데이터로 재학습해 비교한 표(Table V, IX)가 있다.

### 한계 및 해석 시 주의

- **효율 수치의 조건:** 특정 모델 크기와 실험 조건의 값이며, 측정 장비가 명시되지 않았다. 추론 환경에서는 별도 측정이 필요하다.
- **DWT 효과의 분리:** DWT+SwinIR과 EWT는 구조도 다르므로 결과 차이를 한 요소의 효과로만 단정할 수 없다. 해상도만 줄이는 대조군(예: PixelUnshuffle, stride conv)이 없어서 웨이블릿 대역 분해 자체의 효과와 해상도 축소 효과를 구분하지 못한다.
- **가역성의 의미:** 변환의 가역성이 네트워크 전체의 완벽한 복원이나 미세 구조 보존을 보장하지 않는다.
- **정성 비교 조건:** "SwinIR보다 경계가 선명하다"는 주장(Fig. 10, 11)은 그림에 근거한다. 비교한 SwinIR는 학습 데이터가 약 10배 많은 공식 사전학습 모델이며, PSNR은 SwinIR가 더 높다.
- **통계:** SSIM 등 다른 지표, 반복 학습(seed)에 따른 편차, 신뢰구간을 보고하지 않는다.
- **재현 정보:** 웨이블릿 종류, 학습 패치 크기와 학습률, 실제 잡음 학습 데이터 등 재현에 필요한 정보 일부가 빠져 있다.
- **X-ray 영상:** 논문에서 검증하지 않았다.

## 7. 내 연구에 적용할 점

> 아래는 논문 결과가 아닌 연구 적용 아이디어다.

- **적용 아이디어:**
  - **CAT vs. DWT+CAT:** 같은 데이터와 평가 조건에서 DWT 적용 전후의 복원 품질과 시간·메모리를 비교한다.
  - **대조군:** EWT에 없는 PixelUnshuffle 대조군을 두어, 해상도 축소 효과와 웨이블릿 대역 효과를 분리한다.
  - **구조 비교:** 비슷한 모델 규모와 학습 조건에서 DWT+SwinIR 또는 EWT 계열을 DWT+CAT과 비교해 CAT의 기여를 살펴본다.
  - **CNN 분기:** DFEB처럼 CAT 블록에 병렬 CNN 분기를 더했을 때 성능과 자원이 어떻게 변하는지 확인한다.
  - **DWT 단계:** 1단계와 2단계에서 품질과 효율의 변화를 비교한다.
  - **X-ray 평가:** 전체 평균 지표 외에 결함 영역의 경계·대비와 미세 결함 보존을 확인하고, 가능하다면 후속 결함 검출 성능도 측정한다.
- **확인이 필요한 점:**
  - EWT가 사용한 웨이블릿 필터 종류와 공식 코드 공개 여부
  - 저널판(*Neural Networks*, 2024)에서 실험이나 수치가 바뀌었는지
  - 잔차를 웨이블릿 계수 공간에서 더하는 방식(EWT)과 특징 공간에서 더하는 방식의 차이가 결과에 미치는 영향

## 8. 참고 자료

- Li et al., [EWT: Efficient Wavelet-Transformer for Single Image Denoising](https://arxiv.org/abs/2304.06274), arXiv v1, 2023.
- Liang et al., SwinIR: Image Restoration Using Swin Transformer, ICCV 2021.
- Liu et al., Multi-level Wavelet-CNN for Image Restoration (MWCNN), CVPR Workshops 2018.
- Liu et al., Swin Transformer: Hierarchical Vision Transformer using Shifted Windows, 2021.
