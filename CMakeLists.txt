cmake_minimum_required(VERSION 3.22)
project(sjtu_algorithm_final_project)
set(CMAKE_CXX_STANDARD 17)

# Include sources
include_directories(hFiles)
file(GLOB SOURCES cppFiles/*.cpp)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})
