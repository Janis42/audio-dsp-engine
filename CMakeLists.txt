cmake_minimum_required(VERSION 3.20)   # Minimum CMake version required

project(audio-dsp-engine
    VERSION 1.0
    LANGUAGES CXX
)     # Project name

set(CMAKE_CXX_STANDARD 23)             # Use C++23 (change to 20 if you prefer)
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # Enforce that standard
set(CMAKE_CXX_EXTENSIONS OFF)          # Disable compiler-specific extensions

add_subdirectory(src)
add_library(dr_wav STATIC external/dr_wav/dr_wav.cpp) # Add dr_wav library from external directory
target_include_directories(dr_wav PUBLIC external/dr_wav) # Include dr_wav headers

add_executable(audio-dsp-engine src/main.cpp) # Define the executable target
target_include_directories(audio-dsp-engine PRIVATE include) # Include the include directory for header files
target_compile_options(audio-dsp-engine PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
)

target_link_libraries(audio-dsp-engine PRIVATE dr_wav) # Link dr_wav to the app target

enable_testing() # Currently unused