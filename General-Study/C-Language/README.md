# C Language

C17 기반으로 문법 → 배열·문자열 → 포인터·메모리 → 임베디드·Linux를 연결해 학습한다.

- [10주 커리큘럼](Curriculum.md) — Notion 원본의 계획 및 체크리스트
- [1주차: Fundamentals](01-Fundamentals/README.md)
- [2주차: Array & String](02-Array-String/README.md)
- [미니 프로젝트: Jetson CPU 온도](Mini-Projects/Jetson-CPU-Temperature/README.md)

## 현재 진도

2026-10-05 정리 기준, 아래 진도는 학습 대화에서 확인한 내용이다. 커리큘럼 원본의 체크리스트와 별도로 기록한다.

- 1주차: 기본 자료형·연산·조건문·반복문·함수·헤더 학습 및 실습 완료.
- 2주차: 배열 합계·크기·주소 간격, 문자열과 널 종료 문자 학습. 문자열 비교 이후의 실행 완료는 미확인.
- Jetson CPU 온도 읽기: 사용자 실행 완료 확인. 현재 저장소에 실제 소스는 아직 없다.

현재 로컬에서 확인된 소스는 `hello.c`다. 나머지 Jetson/Windows 실습 파일은 실제 파일을 확보한 뒤 추가한다.

## 실행 원칙

Linux/Jetson에서 GCC가 설치된 경우:

```bash
gcc -Wall -Wextra -Wpedantic -std=c17 01-Fundamentals/hello.c -o hello
./hello
```

실행 파일은 커밋하지 않는다. 필요하면 `bin/` 디렉터리에 빌드한다. Windows의 실행 방식은 설치된 컴파일러와 터미널 환경에 따라 구분한다.

## C++ 센서 실습

- [RPLIDAR A3M1 데이터 취득](../Hardware/Sensors/RPLIDAR-A3M1/README.md) — C17 과정과 별도의 C++ 실습
