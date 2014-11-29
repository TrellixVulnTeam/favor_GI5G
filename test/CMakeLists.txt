link_directories (${favor_BINARY_DIR}/src) 
SET(CMAKE_CXX_FLAGS "-std=c++11")

#There are specific testing facilities for cmake, but we don't necessarily need them right this minute: http://www.vtk.org/Wiki/CMake/Testing_With_CTest
add_executable (test main.cpp)


# Add executable called "helloDemo" that is built from the source files 
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.
################################
# GTest
################################
ADD_SUBDIRECTORY (gtest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Link test executable against gtest & gtest_main
target_link_libraries(test gtest gtest_main)

target_include_directories(test PUBLIC gtest/include)

# Link the executable to the Hello library. 
target_link_libraries (test favor) 