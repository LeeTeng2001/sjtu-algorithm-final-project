cmake_minimum_required(VERSION 3.22)

# Create executable for resource scheduler
add_executable(single-core main.cpp def.hpp def.cpp)

# Create executable for for generator
add_executable(generator-test-cases generator.hpp generator.cpp def.hpp def.cpp)