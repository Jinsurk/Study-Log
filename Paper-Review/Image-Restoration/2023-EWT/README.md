# EWT: Efficient Wavelet-Transformer for Single Image Denoising

## 논문 정보

- **저자:** Juncheng Li, Bodong Cheng, Ying Chen, Guangwei Gao, Tieyong Zeng
- **공개:** 2023년 4월, arXiv v1
- **논문:** [arXiv:2304.06274](https://arxiv.org/abs/2304.06274)
- **키워드:** Image Denoising, DWT, Wavelet, Transformer, CNN
- **리뷰 작성일:** 2026-11-03
- **검토 범위:** 위 arXiv v1

## 1. 한 줄 요약

DWT로 입력을 네 주파수 대역으로 분해하고 CNN과 윈도우 기반 Transformer를 결합하여 영상 복원 성능과 연산 효율의 균형을 추구한다.

## 2. 연구 배경과 문제 정의

고해상도 영상에 Transformer를 적용하면 실행 시간과 GPU 메모리 부담이 크다. 저자들은 공간 해상도를 줄이면서 주파수 대역을 활용해 영상 복원에 필요한 정보를 다루는 denoising 모델을 제안한다.

## 3. 핵심 아이디어 및 기여

1. **DWT/IWT:** 입력에 DWT를 적용하고 출력에서 IWT로 원래 해상도를 복원한다.
2. **DFEB:** CNN 분기와 Transformer 분기를 병렬로 사용하고 출력을 결합해 특징을 융합한다.
3. **MFAM:** 여러 DFEB의 특징을 집계하고 잔차 연결을 사용한다.
4. **웨이블릿 영역 잔차 학습:** 입력 웨이블릿 계수에 네트워크 출력을 더한 뒤 IWT를 적용한다.

## 4. 모델 구조와 동작 과정

```text
잡음 영상 → DWT → 4개 대역 결합 → 합성곱 → MFAM
                                              ├─ CNN 분기
                                              └─ Transformer 분기
          → 합성곱 → 입력 웨이블릿 계수와 잔차 합산 → IWT → 복원 영상
```

DWT는 영상을 저주파 LL과 세 방향의 고주파 성분 LH, HL, HH로 분해한다. 각 대역의 가로·세로 크기는 원본의 절반이며, 네 대역을 채널 방향으로 결합한다. DWT 자체는 적절한 조건에서 가역적이지만, 이것이 학습된 전체 네트워크의 완벽한 정보 보존을 보장하지는 않는다.

DFEB의 CNN 분기는 국소 특징을 추출하고, Transformer 분기는 Swin의 윈도우 및 시프트 윈도우 attention을 사용한다. 전체 영상에 한 번에 전역 attention을 적용하는 구조는 아니다. MFAM은 여러 DFEB의 출력을 집계한다.

최종 모델은 1단계 DWT, 4개 DFEB, DFEB당 6개 Transformer 블록, 임베딩 차원 180, 윈도우 크기 8을 사용한다. 손실 함수는 L1이며 파라미터는 약 11.8M이다. (논문 3–5쪽)

## 5. 실험 설정 및 주요 결과

- DIV2K 학습 영상 800장을 사용하고, 합성 Gaussian 잡음과 실제 잡음 데이터에서 평가한다.
- 회색조 영상은 σ = 15, 25, 50, 컬러 영상은 σ = 10, 30, 50을 사용한다.
- Kodak24, CBSD68, Urban100 및 SIDD 등을 평가에 사용한다.

### SwinIR 비교

| 모델 | 파라미터 | PSNR | 실행 시간 | GPU 메모리 |
|---|---:|---:|---:|---:|
| SwinIR* | 5.17M | 31.79 dB | 53.29초 | 18,432 MiB |
| DWT + SwinIR* | 5.20M | 31.57 dB | 12.52초 | 8,276 MiB |
| EWT* | 5.18M | 31.73 dB | 9.14초 | 6,347 MiB |

Kodak24, 컬러, Gaussian σ = 30, 논문 Table V. 별표 모델은 비교를 위해 크기를 조정하고 같은 설정으로 재학습한 모델이다. EWT*는 최종 11.8M 모델이 아니다. EWT*는 SwinIR* 대비 PSNR이 0.06 dB 낮고, 표의 실행 시간은 약 83%, GPU 메모리는 약 66% 적다. 논문은 GPU 수치를 학습 시 사용량으로 설명한다. 다른 장비나 추론 조건에서도 같은 비율을 보장하지 않는다.

### 구성 요소 실험

- **DWT 단계:** 1/2/3단계에서 PSNR은 각각 31.78/31.62/27.94 dB이며, 단계가 증가하면 시간과 메모리는 줄지만 성능도 저하된다. (Table VI)
- **분기 조합:** 2 DFEB 실험에서 CNN+CNN, Transformer+Transformer, CNN+Transformer는 각각 31.12, 31.66, 31.72 dB를 기록했다. CNN+CNN이 가장 빠르고 메모리 사용량이 적었다. (Table VII)
- **실제 잡음:** SIDD에서 39.52 dB를 기록했으며 표의 CycleISP와 같은 수치다. (Table IV)

## 6. 강점과 한계

### 강점

- 해상도 축소와 웨이블릿 대역 활용을 결합해 Transformer의 자원 부담을 줄이려는 설계가 명확하다.
- PSNR과 함께 파라미터, 실행 시간, GPU 메모리, FLOPs를 비교한다.
- DWT 단계와 분기 조합, DFEB 개수에 대한 ablation을 제공한다.

### 한계 및 해석 시 주의

- 효율 수치는 특정 모델 크기와 실험 조건에 해당하며, 추론 환경에서 별도 측정이 필요하다.
- DWT+SwinIR과 EWT는 구조도 다르므로 결과 차이를 한 요소의 효과로만 단정할 수 없다.
- 변환의 가역성이 네트워크 전체의 완벽한 복원이나 미세 구조 보존을 보장하지 않는다.
- X-ray 영상에서의 성능은 논문에서 검증하지 않았다.

## 7. 내 연구에 적용할 점

> 아래는 논문 결과가 아닌 연구 적용 아이디어다.

- **CAT vs. DWT+CAT:** 같은 데이터와 평가 조건에서 DWT 적용 전후 복원 품질 및 시간·메모리를 비교한다.
- **구조 비교:** 가능하다면 비슷한 모델 규모와 학습 조건에서 DWT+SwinIR 또는 EWT 계열과 DWT+CAT을 비교해 CAT의 기여를 살펴본다.
- **DWT 단계:** 1단계와 2단계에서 품질과 효율의 변화를 비교한다.
- **X-ray 평가:** 전체 평균 지표 외에 결함 영역의 경계·대비와 미세 결함 보존을 확인하고, 가능하다면 후속 결함 검출 성능도 측정한다.

## 8. 참고 자료

- Li et al., [EWT: Efficient Wavelet-Transformer for Single Image Denoising](https://arxiv.org/abs/2304.06274), arXiv v1, 2023.
