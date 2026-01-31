cmake_minimum_required(VERSION 3.28)

project(icp VERSION 2025.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_BUILD_TYPE Debug)

find_package(glm           CONFIG REQUIRED)
find_package(GLEW                 REQUIRED)
find_package(glfw3         CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(assimp REQUIRED)

if(WIN32)
    find_package(OpenCV CONFIG REQUIRED PATHS "${OPENCV_DIR}/x64/vc16/lib")
elseif(UNIX)
    find_package(OpenCV CONFIG REQUIRED)
endif()

file(GLOB_RECURSE PROJECT_SOURCES 
    "${PROJECT_SOURCE_DIR}/source/*.cpp"
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    GLEW::GLEW
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
    ${OpenCV_LIBS}
    imgui::imgui
    tinyfiledialogs::tinyfiledialogs
    assimp::assimp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

set(RUN_MODE "GLAPP_SHOOTER" CACHE STRING "Which program entry to build")
set_property(CACHE RUN_MODE PROPERTY STRINGS GLAPP_SHOOTER GLAPP_VIEWER TRACKAPP THREADTRACKAPP)

if (RUN_MODE STREQUAL "GLAPP_SHOOTER")
    target_compile_definitions(${PROJECT_NAME} PRIVATE RUN_GLAPP)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SCENE_SHOOTER)
elseif (RUN_MODE STREQUAL "GLAPP_VIEWER")
    target_compile_definitions(${PROJECT_NAME} PRIVATE RUN_GLAPP)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SCENE_VIEWER)
elseif (RUN_MODE STREQUAL "TRACKAPP")
    target_compile_definitions(${PROJECT_NAME} PRIVATE RUN_TRACKAPP)
elseif (RUN_MODE STREQUAL "THREADTRACKAPP")
    target_compile_definitions(${PROJECT_NAME} PRIVATE RUN_THREADTRACKAPP)
elseif (RUN_MODE STREQUAL "VIDEOENCODER")
    target_compile_definitions(${PROJECT_NAME} PRIVATE RUN_VIDEOENCODER)
else()
    message(FATAL_ERROR "Invalid RUN_MODE: ${RUN_MODE}")
endif()
