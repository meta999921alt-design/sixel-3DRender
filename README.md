# gdi-raytracer

Win32 GDI 창에 실시간으로 레이트레이싱 + 물리 시뮬레이션을 그리는 C++17 프로젝트입니다.

## 물리 데모

경사로(램프) 위에서 공이 중력으로 떨어져 미끄러져 내려온 뒤, 바닥에서 마찰로 멈추는
기본적인 물리엔진 데모입니다. 중력 가속, 평면 충돌(반발계수+마찰), 질량을 고려한
강체 충돌까지 구현되어 있습니다.

- **WASD**: 카메라 이동
- **마우스 우클릭 드래그**: 시선 회전
- **R**: 공을 스폰 위치로 리셋
- **ESC** 또는 창 닫기: 종료

창 제목 표시줄에 실시간 FPS와 render/present 소요시간이 표시됩니다.

## 요구 사항

- Windows (Win32 GDI를 직접 쓰므로 Windows 전용입니다)
- C++17 지원 컴파일러 (MSVC 또는 MinGW g++)
- CMake 3.10 이상 (선택 — 없어도 g++로 직접 빌드 가능)

## 빌드

### CMake

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### MinGW로 직접 빌드 (콘솔 없는 배포용 exe)

```bash
windres resources/version.rc -O coff -o version.o
g++ -std=c++17 -O2 -mwindows -static -static-libgcc -static-libstdc++ -Iinclude src/*.cpp version.o -o raytracer.exe -lgdi32 -luser32
```

`-mwindows`를 빼면 진단 메시지를 볼 수 있는 콘솔 창이 같이 뜨는 개발용 빌드가 됩니다.

## 구조

```
include/      헤더 (타입 선언)
src/          구현 (.cpp)
resources/    exe 버전 정보 리소스 (오탐 방지용 메타데이터)
```

| 파일 | 역할 |
|---|---|
| vector3.h, color.h, ray.h, material.h | 기본 값 타입 |
| hittable.h, sphere.h, plane.h, quad.h | 교차 판정 가능한 물체들 (구체/무한평면/유한사각면) |
| scene.h/cpp | 레이트레이싱 셰이딩 (그림자/반사/프레넬) + 씬 조립 |
| **physics.h/cpp** | **범용 물리엔진** — 중력, 질량 고려 강체 충돌, 평면 충돌(반발+마찰) |
| camera.h/cpp | 1인칭 카메라 (이동/회전, 프레임당 1회 sin/cos 캐싱) |
| window.h/cpp | Win32 GDI 창, 메시지 루프, 키보드/마우스 입력 |
| thread_pool.h/cpp | 영구 스레드풀 |
| app.h/cpp | 메인 루프, 씬 구성(경사로+공) |

## 물리엔진 설계

`PhysicsEngine`은 다음 역할을 합니다:

1. **중력**: 매 스텝 `gravity * dt`를 속도에 더함 (반암시적 오일러 적분)
2. **정적 평면 충돌**: 반발계수(bounce)와 마찰(접선 속도 감쇠)을 분리해서 처리 —
   바닥은 마찰이 커서 공이 멈추고, 경사면은 마찰이 낮아 중력의 접선 성분이 이겨서
   미끄러져 내려감
3. **강체-강체 충돌**: 질량을 실제로 반영한 완전탄성/비탄성 충돌 임펄스 공식
   (`RigidBody::invMass()`가 0이면 고정체로 취급하도록 설계되어 있어, 나중에
   고정 장애물을 강체로 추가하기도 쉬움)

`reset()`을 호출하면 addBody 시점의 초기 상태로 되돌아갑니다 (데모에서 R키로 연결됨).
