cmake_minimum_required(VERSION 3.22)

# Create executable
add_executable(single-core main.cpp def.hpp def.cpp)

add_executable(generator generator.cpp ResourceSchedulerOriginal.cpp)