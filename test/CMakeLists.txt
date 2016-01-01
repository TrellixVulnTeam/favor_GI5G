link_directories (${favor_BINARY_DIR}/src) 
SET(CMAKE_CXX_FLAGS "-std=c++11")

#There are specific testing facilities for cmake, but we don't necessarily need them right this minute: http://www.vtk.org/Wiki/CMake/Testing_With_CTest
add_definitions(-DDEBUG) #This needs to be here for tests to compile properly in Favor's CMAKE, god knows why
set(CMAKE_BUILD_TYPE Debug)
add_executable (tests main.cpp)
target_include_directories(tests PUBLIC ../src/lib/gtest/googletest/include)
target_include_directories(tests PUBLIC ../src/lib/gtest/googlemock/include)
target_link_libraries(tests gtest gtest_main)
target_link_libraries(tests gmock gmock_main)


# Link the executable to the favor testing library
target_link_libraries (tests favor_test)