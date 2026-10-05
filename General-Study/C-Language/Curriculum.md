# C 학습 커리큘럼


> **목표:** C 문법 암기보다 **Memory / Pointer / Compiler / Embedded / Linux**를 연결해 이해한다. 최종적으로 Arduino UNO의 MCU-level 제어와 Jetson Orin Nano의 Linux/ROS 2 환경을 연결할 수 있는 수준을 목표로 한다.

## 학습 환경

| 항목 | 사용 환경 | 역할 |
| --- | --- | --- |
| Editor | VS Code | C 코드 작성 및 Remote SSH |
| Linux 실습 | Jetson Orin Nano | GCC, GDB, Makefile, Linux System Programming |
| MCU 실습 | Arduino UNO / ATmega328P | GPIO, Register, Timer, Interrupt, UART, ADC, PWM |
| 보조 장비 | Jetson Nano | 필요 시 Linux 실험용 |

### 기본 Toolchain

- **Compiler:** GCC (`gcc`)

- **Debugger:** GDB (`gdb`)

- **Build:** Make / Makefile

- **Binary 분석:** `objdump`, `readelf`, `nm`

- 처음에는 VS Code의 자동 Build 기능에 의존하지 않고 **Terminal에서 직접 Compile → Execute → Debug** 한다.

```bash
gcc -Wall -Wextra -Wpedantic -std=c17 main.c -o main
./main

```
---

## 전체 학습 흐름

```text
C Syntax
   ↓
Array / String
   ↓
Pointer / Memory
   ↓
Struct / Dynamic Memory
   ↓
Bit Operation / volatile
   ↓
Embedded C (Arduino UNO)
   ↓
GCC / GDB / Makefile / Linux I/O
   ↓
UNO ↔ Jetson Serial Communication
   ↓
Embedded + Linux + ROS 2 Integration

```
## 10주 Curriculum

| 주차 | 핵심 주제 | 주요 내용 | 장비 |
| --- | --- | --- | --- |
| 1주차 | C Fundamentals | 자료형, 연산자, 조건문, 반복문, 함수 | Jetson Orin Nano |
| 2주차 | Array & String | 배열, 문자열, 메모리 배치, `sizeof` | Jetson Orin Nano |
| 3주차 | Pointer | 주소, 역참조, Pointer Arithmetic, Array-Pointer 관계, Pointer to Pointer | Jetson Orin Nano |
| 4주차 | Memory & Struct | Stack/Heap, `malloc/free`, Struct, Union, Enum, Typedef, Alignment/Padding | Jetson Orin Nano |
| 5주차 | Low-level C | Bitwise Operation, Masking, `const`, `volatile`, Register 개념 | Jetson + UNO |
| 6주차 | Embedded I/O | GPIO, Register 직접 제어, ADC, PWM | Arduino UNO |
| 7주차 | Interrupt & Communication | Timer, External Interrupt, ISR, UART | Arduino UNO |
| 8주차 | Toolchain & Linux C | GCC, GDB, Makefile, ELF, File I/O, System Call | Jetson Orin Nano |
| 9주차 | Serial Integration | UART/USB Serial, Packet 설계, Parser, `/dev/ttyACM*` | UNO ↔ Jetson |
| 10주차 | Final Integration | Sensor → MCU → UART → Jetson → C/C++ → ROS 2 | UNO + Orin Nano |

---

## Week 1 — C Fundamentals

- [x] 개발 환경 및 `gcc` 확인

- [x] Hello World 직접 Compile / Execute

- [x] Integer / Floating-point 자료형

- [x] `stdint.h`: `uint8_t`, `uint16_t`, `uint32_t`

- [x] signed / unsigned

- [x] 연산자와 Type Conversion

- [x] `if`, `switch`

- [x] `for`, `while`

- [x] 함수 선언 / 정의 / 호출

- [x] Header file의 기본 역할
**실습 목표:** 간단한 CLI Calculator를 함수 단위로 구현한다.

## Week 2 — Array & String

- [ ] 1D / 2D Array

- [ ] Array Memory Layout

- [ ] Character Array와 C String

- [ ] Null Terminator (`\0`)

- [ ] `strlen`, `strcmp`, `strcpy` 계열 함수의 동작 이해

- [ ] `sizeof`와 배열 크기

- [ ] 배열의 함수 전달

- [ ] Buffer Boundary 개념
**실습 목표:** 입력 문자열 분석기 및 간단한 데이터 배열 처리 프로그램을 구현한다.

## Week 3 — Pointer 집중

> 이 과정의 핵심 주차. Pointer를 문법으로 외우지 않고 **Address + Type + Memory Access** 관점으로 이해한다.

- [ ] Address (`&`) / Dereference (`*`)

- [ ] Pointer type

- [ ] Pointer Arithmetic

- [ ] `$a[i] \equiv *(a+i)$` 관계 이해

- [ ] Array ↔ Pointer 관계

- [ ] Pointer to Pointer (`T **`)

- [ ] `void *`

- [ ] `const int *`, `int * const`, `const int * const`

- [ ] NULL Pointer

- [ ] Dangling / Wild Pointer

- [ ] GDB로 주소와 실제 메모리 확인
**실습 목표:** Pointer 기반 Array Traversal 및 GDB Memory Inspection.

## Week 4 — Memory, Struct & Dynamic Allocation

- [ ] Process Memory Layout 개념: Text / Data / BSS / Heap / Stack

- [ ] Automatic / Static Storage Duration

- [ ] `malloc`, `calloc`, `realloc`, `free`

- [ ] Memory Leak / Use-after-free / Double-free

- [ ] `struct`

- [ ] `union`

- [ ] `enum`

- [ ] `typedef`

- [ ] Structure Pointer와 `->`

- [ ] Alignment / Padding

- [ ] `sizeof(struct)` 직접 검증
**실습 목표:** Dynamic Array 또는 Linked List를 직접 구현한다.

## Week 5 — Low-level C

- [ ] Binary / Hexadecimal 표현

- [ ] `&`, `|`, `^`, `~`

- [ ] `<<`, `>>`

- [ ] Bit Masking / Set / Clear / Toggle

- [ ] Register 개념

- [ ] Memory-mapped I/O 개념

- [ ] `const`

- [ ] `volatile`

- [ ] Compiler Optimization과 `volatile`의 관계

- [ ] Endianness

```c
reg |=  (1U << bit);   // set
reg &= ~(1U << bit);   // clear
reg ^=  (1U << bit);   // toggle

```
**실습 목표:** 8/16/32-bit 가상 Register를 만들어 Bit Manipulation 함수를 구현한다.

## Week 6 — Arduino UNO: GPIO / ADC / PWM

- [ ] ATmega328P 기본 Architecture 확인

- [ ] Arduino API와 Register-level 제어 차이

- [ ] `DDRx`, `PORTx`, `PINx`

- [ ] LED Blink

- [ ] Button Digital Input

- [ ] Register 직접 GPIO 제어

- [ ] ADC 기본 동작

- [ ] PWM 기본 동작

- [ ] Datasheet의 Register Map 읽기

```c
DDRB  |= (1U << DDB5);
PORTB |= (1U << PORTB5);

```
**실습 목표:** Arduino API 없이 Register 기반 LED / Button 제어.

## Week 7 — Timer / Interrupt / UART

- [ ] Polling vs Interrupt

- [ ] ISR (Interrupt Service Routine)

- [ ] External Interrupt

- [ ] Timer / Counter

- [ ] Prescaler

- [ ] Timer Overflow / Compare Match

- [ ] PWM와 Timer 관계

- [ ] UART Frame 기본 구조

- [ ] Baud Rate

- [ ] UART TX / RX

- [ ] ISR 공유 변수와 `volatile`
**실습 목표:** Timer Interrupt 기반 주기 동작 + UART Sensor Data 출력.

## Week 8 — GCC / GDB / Makefile / Linux C

### Build 과정

```text
Source (.c)
   ↓ Preprocessor
Preprocessed Source
   ↓ Compiler
Assembly (.s)
   ↓ Assembler
Object (.o)
   ↓ Linker
Executable / ELF

```
- [ ] `gcc -E`

- [ ] `gcc -S`

- [ ] `gcc -c`

- [ ] Linking

- [ ] Header / Source 분리

- [ ] Multiple Translation Units

- [ ] Makefile

- [ ] GDB Breakpoint / Step / Watch

- [ ] `objdump`

- [ ] `readelf`

- [ ] `nm`

- [ ] Linux File I/O: `open`, `read`, `write`, `close`

- [ ] File Descriptor
**실습 목표:** 여러 `.c/.h` 파일로 구성된 프로그램을 Makefile로 Build하고 GDB로 Debugging한다.

## Week 9 — Arduino UNO ↔ Jetson

```text
Arduino UNO
 ATmega328P
     │
     │ UART / USB Serial
     ▼
Jetson Orin Nano
     │
     └── Linux C Application

```
- [ ] `/dev/ttyACM*` / `/dev/ttyUSB*`

- [ ] Linux Serial Port 설정

- [ ] `termios`

- [ ] UART TX/RX

- [ ] ASCII Protocol vs Binary Protocol

- [ ] Packet Header / Length / Payload / Checksum

- [ ] State-machine Parser

- [ ] Error Handling
**실습 목표:** UNO에서 ADC 값을 읽어 Jetson C 프로그램으로 전송하고 Parsing한다.

## Week 10 — Final Project

> **최종 Pipeline:** Sensor → Arduino UNO → UART → Jetson Orin Nano → Linux Application → ROS 2

- [ ] Sensor/ADC 데이터 취득

- [ ] MCU-side Packet 생성

- [ ] UART 전송

- [ ] Jetson Serial Receiver 작성

- [ ] Packet Parsing / Validation

- [ ] 데이터 Logging

- [ ] C/C++ 경계 이해

- [ ] ROS 2 Node로 확장

- [ ] Topic Publish

- [ ] 전체 시스템 Debugging
---

## Mini Projects

1. **CLI Calculator** — 함수 / 조건문 / 입력 처리

2. **Dynamic Array** — Pointer / `malloc` / `realloc` / `free`

3. **Linked List or Queue** — Struct / Pointer / Dynamic Memory

4. **Binary Packet Parser** — Endianness / Bit Operation / Struct

5. **Register-level GPIO Controller** — ATmega328P

6. **UART Sensor Node** — Arduino UNO

7. **Linux Serial Receiver** — Jetson Orin Nano

8. **UNO ↔ Jetson Integrated Sensor Pipeline** — Final Project

## 학습 원칙

- **30% Theory / 70% Coding & Debugging**을 기준으로 한다.

- Pointer와 Memory는 코드 실행 결과만 보지 말고 **GDB로 주소와 메모리를 직접 확인**한다.

- Embedded 실습은 Arduino API에만 의존하지 않고 **ATmega328P Datasheet → Register → C code** 순으로 확인한다.

- Compiler가 발생시키는 Warning을 무시하지 않는다.

- 동작하는 코드에서 끝내지 않고 가능하면 **C source → Assembly → Machine-level behavior**를 연결한다.

- 알고리즘 문제 풀이 자체보다 Embedded/Linux 환경에서 실제로 사용할 수 있는 코드를 우선한다.

## 권장 Repository 구조

```text
c_study/
├── 01_basic/
├── 02_array_string/
├── 03_pointer/
├── 04_memory_struct/
├── 05_low_level_c/
├── 06_arduino_gpio_adc_pwm/
├── 07_timer_interrupt_uart/
├── 08_linux_toolchain/
├── 09_serial_integration/
└── 10_final_project/

```

> 각 주차는 **개념 → 짧은 예제 → 직접 구현 → Debugging → Mini Project** 순으로 진행한다. 진도보다 Pointer/Memory/Register가 실제로 어떻게 동작하는지 이해하는 것을 우선한다.


---

원본: [Notion — C 학습 커리큘럼](https://app.notion.com/p/3e29b0b79c14801d8c3fc3eeca7bfc2f)

GitHub 정리일: 2026-10-05. 이 문서는 Notion 노트의 수동 스냅샷이며 자동 동기화되지 않습니다.
