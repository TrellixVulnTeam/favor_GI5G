#Favor's CMake build file. Right now we build a library with the src folder and an executable with the test folder
cmake_minimum_required(VERSION 2.8)

project(favor)

add_subdirectory(src)
add_subdirectory(test)