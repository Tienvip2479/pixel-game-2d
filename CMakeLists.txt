cmake_minimum_required(VERSION 3.16)
project(pixel-game-2d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML package
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

# Add executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Link SFML libraries
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system sfml-audio)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Platform-specific settings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
