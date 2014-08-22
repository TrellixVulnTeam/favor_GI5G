#Build the core Favor library
add_library(favor SHARED favor.cpp processor.cpp reader.cpp worker.cpp)
#add_library(favor STATIC favor.cpp processor.cpp reader.cpp worker.cpp)