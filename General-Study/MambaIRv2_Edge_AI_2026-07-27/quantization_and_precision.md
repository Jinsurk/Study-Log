# MambaIRv2 Edge AI: 정밀도·양자화 실험 정리

## 1. 실험 조건

| 항목 | 설정 |
|---|---|
| Dataset | XAVIS Cylinder / AL_Cu |
| Test split | v2 (legacy v5), 600 images |
| Sampling seed | 42 |
| Model | DWT–MambaIRv2-small |
| Input resolution | 256 × 959 |
| Batch size | 1 |
| Device | Jetson Orin Nano |
| Software | L4T R36.5, CUDA 12.6, PyTorch 2.5.0a0 |
| Checkpoint SHA256 | `f351f70a1ef479fdacf260ce28bd3c918a88429fae1d426dab4e594317dee8bd` |

GPU Server와 Jetson은 동일한 600개 filename, 동일한 순서, 동일 checkpoint를 사용했다. LR/HR 누락과 중복 filename은 없었다.

## 2. FP32 기준

Jetson FP32 결과:

- Mean PSNR: **39.259579 dB**
- Mean SSIM: **0.982422719**
- Mean latency: **1419.7694 ms/image**
- Total inference time: **851.8617 sec**
- Average GPU utilization: **87.34%**
- Average power: **15.24 W**
- Maximum GPU temperature: **61.03°C**
- Output shape: **256 × 959 × 1**
- NaN/Inf: 없음
- Inference errors: 0

FP32는 품질·속도·자원 비교를 위한 baseline으로 사용했다.

## 3. FP16 적용 방식

FP16은 전체 모델을 무조건 `model.half()`로 변환하지 않고 **CUDA autocast** 방식으로 적용했다.

- FP16 적용 대상: autocast가 안전하다고 판단하는 연산
- FP32 유지 대상: DWT/IDWT 및 PSNR·SSIM 계산
- 모델 구조와 checkpoint: 변경하지 않음
- test split과 filename 순서: FP32와 동일

전체 모델을 FP16으로 변환하면 Selective Scan CUDA 연산, normalization, residual 누적, DWT/IDWT에서 정밀도 손실이나 NaN/Inf가 발생할 가능성이 있다. CUDA autocast는 연산별로 적절한 정밀도를 선택하므로 속도 향상과 수치 안정성을 함께 확보할 수 있다.

DWT/IDWT는 복원 영상의 수치에 직접 영향을 주므로 FP32로 유지했고, PSNR·SSIM은 작은 오차를 측정하는 지표이므로 공정한 비교를 위해 FP32로 계산했다.

## 4. FP32 vs FP16 결과

| Metric | FP32 | FP16 | Difference |
|---|---:|---:|---:|
| Mean PSNR | 39.259579 dB | 39.259806 dB | +0.000227 dB |
| Mean SSIM | 0.982422719 | 0.982421993 | −0.000000727 |
| Mean latency | 1419.7694 ms/image | 1197.6434 ms/image | −15.6452% |
| Total inference time | 851.8617 sec | 718.5860 sec | −133.2757 sec |
| Speedup | 1.000× | 1.1855× | +18.55% |

FP16은 평균 복원 품질을 사실상 유지하면서 Jetson latency를 줄였다. NaN/Inf와 inference error는 없었다.

## 5. FP32 vs FP16 자원 사용량

| Metric | FP32 | FP16 | Difference |
|---|---:|---:|---:|
| Average GPU utilization | 87.34% | 87.62% | +0.28%p |
| Maximum GPU utilization | 99% | 100% | +1%p |
| Average power | 15.24 W | 15.43 W | +0.19 W |
| Maximum power | 16.98 W | 17.97 W | +0.99 W |
| Average GPU temperature | 58.75°C | 57.27°C | −1.48°C |
| Maximum GPU temperature | 61.03°C | 59.97°C | −1.06°C |

추가 모니터링 정보:

- CPU temperature: 55.63°C average / 58.63°C maximum
- RAM usage: 5718 MB average / 6076 MB maximum
- Monitoring samples: 783
- FP16 monitoring interval: 약 787초

FP16은 FP32와 유사한 GPU utilization을 보였고 평균 전력은 소폭 증가했다. 반면 GPU 온도는 더 낮게 측정됐다. `tegrastats`에는 throttle 상태 자체가 기록되지 않았으므로 thermal throttling이 없다고 단정할 수는 없지만, 관측 구간에서 throttling 징후는 확인되지 않았다.

## 6. GPU Server–Jetson 수치적 일관성

동일한 v2/legacy v5 600장 test split을 사용해 GPU Server FP32와 Jetson FP32를 비교했다.

| Metric | GPU Server FP32 | Jetson FP32 | Difference |
|---|---:|---:|---:|
| Mean PSNR | 39.257357857 dB | 39.259579180 dB | +0.002221323 dB |
| Mean SSIM | 0.982427816 | 0.982422719 | −0.000005097 |
| Mean absolute PSNR difference | — | — | 0.013017463 dB |
| Maximum absolute PSNR difference | — | — | 0.392864041 dB |
| Mean absolute SSIM difference | — | — | 0.000022216 |
| Maximum absolute SSIM difference | — | — | 0.000181899 |

평균 PSNR과 SSIM은 거의 동일했지만 이미지별 출력이 완전히 동일한 것은 아니다.

## 7. INT8 적용 현황

이번 실험에서는 INT8 latency, 품질, 전력 결과를 정량 측정하지 않았다. 따라서 INT8을 현재 검증된 배포 방식으로 제시하지 않는다.

INT8 적용 전에 확인해야 할 항목:

1. Selective Scan CUDA 연산의 INT8 입력·가중치·누산 지원 여부
2. DWT/IDWT 및 Mamba Block의 양자화 민감도
3. XAVIS Cylinder / AL_Cu용 calibration set 구성
4. Layer-wise sensitivity 분석
5. PyTorch eager, TensorRT, custom CUDA kernel 중 backend 선택
6. 양자화 후 PSNR/SSIM 및 장시간 안정성 재검증

향후에는 민감도가 낮은 연산부터 부분 INT8 양자화를 적용하고, Selective Scan 호환성을 확인한 뒤 전체 모델 적용 여부를 판단한다.

## 8. 결론

- FP16 CUDA autocast는 FP32 대비 평균 품질을 사실상 유지했다.
- Jetson FP16 latency는 FP32보다 15.65% 낮았고 1.1855× speedup을 기록했다.
- FP16 평균 전력은 0.19 W 증가했지만 GPU 온도는 더 낮았다.
- 동일한 600장 test split에서 GPU Server와 Jetson FP32의 평균 PSNR/SSIM은 수치적으로 일관됐다.
- 현재 검증된 배포 경로는 FP16이며, INT8은 calibration·연산 호환성·품질 검증 이후의 다음 단계다.

관련 발표자료: [`MambaIRv2 & Edge AI.pptx`](./MambaIRv2%20%26%20Edge%20AI.pptx)
