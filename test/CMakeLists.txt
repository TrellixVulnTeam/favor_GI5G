link_directories (${favor_BINARY_DIR}/src) 
SET(CMAKE_CXX_FLAGS "-std=c++11 -O0 -g")

#There are specific testing facilities for cmake, but we don't necessarily need them right this minute: http://www.vtk.org/Wiki/CMake/Testing_With_CTest
add_definitions(-DDEBUG) #This needs to be here for tests to compile properly in Favor's CMAKE, god knows why
set(CMAKE_BUILD_TYPE Debug)
add_executable (ftest main.cpp)
target_include_directories(favor PUBLIC ../src/lib/gtest/include)
target_link_libraries(ftest gtest gtest_main)

# Link the executable to the Hello library. 
target_link_libraries (ftest favor)