# Jetson CPU 온도 읽기

목표: Linux sysfs에서 실제 하드웨어 센서 값을 읽고 C의 파일 입출력과 연결한다.

## 확인한 내용

사용자가 Jetson에서 아래 명령을 실행해 센서 이름과 온도를 확인했다.

```bash
for zone in /sys/class/thermal/thermal_zone*; do
    printf '%s: ' "$zone"
    cat "$zone/type" "$zone/temp"
done
```

당시 `thermal_zone0`의 이름은 `cpu-thermal`, 온도 값은 `48500`이었다. 해당 값은 밀리섭씨 단위로 48.5°C에 해당한다. 번호는 장치·환경에 따라 달라질 수 있으므로 `type`을 확인해 CPU 영역을 찾는다.

CV 영역에서는 `No data available`이 나타났다. 이는 이 기록에서 관찰한 읽기 결과이며, 모든 영역에 항상 유효한 온도가 있다고 가정하지 않는다.

CPU 온도를 읽는 C 프로그램은 사용자 실행 완료를 확인했다. 현재 저장소에는 실제 `cpu_temp.c` 원본이 없어 코드·빌드 재검증은 보류한다.

## 소스 확보 후 기록할 것

- 실제 사용한 경로와 센서 이름
- `fopen`·`fscanf`·`fclose` 및 실패 처리
- 단위 변환과 반복 측정 주기
- 빌드 명령, 출력, 최소·최대·평균 계산 결과
