cmake_minimum_required(VERSION 3.10)
project(gdi_raytracer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT WIN32)
    message(FATAL_ERROR "이 데모는 Win32 GDI를 사용하므로 Windows에서만 빌드됩니다.")
endif()

if (MSVC)
    add_compile_options(/O2)
else()
    add_compile_options(-O2 -Wall)
endif()

add_executable(raytracer WIN32
    src/scene.cpp
    src/physics.cpp
    src/camera.cpp
    src/thread_pool.cpp
    src/window.cpp
    src/app.cpp
    src/main.cpp
    resources/version.rc
)

target_include_directories(raytracer PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(raytracer PRIVATE Threads::Threads gdi32 user32)
