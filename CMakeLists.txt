# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15...4.0)

project(
  Project
  VERSION 1.0
  LANGUAGES CXX)

# Used modern C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg toolchain (use CVPKG_ROOT for global) 
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

find_package(OpenCV CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED) # built-in FindOpenGL, uses opengl32.lib

# Sources
file(GLOB SOURCES "${Project_SOURCE_DIR}/source/*.cpp")

# Executable target
add_executable(app main.cpp ${SOURCES})

# Include dirs (often unnecessary, but OpenCV is special)
target_include_directories(app PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(app PRIVATE OpenGL::GL)

# Link libraries
target_link_libraries(app
  PRIVATE
    ${OpenCV_LIBS}     # from OpenCVConfig.cmake
    GLEW::GLEW
    glfw
) 