# MambaIRv2 & Edge AI

Jetson Orin Nano 이식 및 GPU Server–Jetson FP32/FP16 비교 실험 자료.

## 포함 자료

- `MambaIRv2 & Edge AI.pptx`: 세미나 발표자료
- `comparison_assets/`: 동일 test image `cylinder_AL_Cu_lr_27108.tif`의 입력, 정답, GPU Server FP32, Jetson FP32, Jetson FP16 결과

## 실험 요약

- Dataset: XAVIS Cylinder / AL_Cu
- Test split: v2 (legacy v5), 600 images
- Device: Jetson Orin Nano
- Precision: FP32 and FP16 CUDA autocast
- FP16 latency improvement on Jetson: 15.65%
- FP16 speedup: 1.186x
- GPU Server–Jetson mean PSNR difference: 0.00222 dB
- GPU Server–Jetson mean SSIM difference: -0.0000051

