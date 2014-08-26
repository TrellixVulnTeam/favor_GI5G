#Build the core Favor library
find_package(Threads)
add_library(favor SHARED ${favor_SOURCE_DIR}/src/lib/sqlite/sqlite3.c favor.cpp processor.cpp reader.cpp worker.cpp messagemanager.cpp)
#add_library(favor SHARED ${favor_SOURCE_DIR}/src/lib/sqlite/sqlite3.c favor.cpp processor.cpp reader.cpp worker.cpp messagemanager.cpp)

target_link_libraries (favor ${CMAKE_THREAD_LIBS_INIT}) #this is our "-pthread"
target_link_libraries(favor ${CMAKE_DL_LIBS}) #this is our "-ldl"