#Build the core Favor library

find_package(Threads) #So we can use threads later

set(compilation_files favor.cpp processor.cpp reader.cpp worker.cpp accountmanager.cpp logger.cpp) #Core files
LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/lib/sqlite/sqlite3.c) #Sqlite

set(compilation_libraries ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
#${CMAKE_DL_LIBS} is our "-ldl"
#${CMAKE_THREAD_LIBS_INIT} is our "-pthread"

#Note that options don't seem to play well with KDevelop, in that they're on even if they're off... or something
#In any case, it's a non-issue because we should be compiling everything when developing anyway.

# Include Email
OPTION(FAVOR_EMAIL_MANAGER "Include the MessageManager for Email" ON)
IF(FAVOR_EMAIL_MANAGER)
  LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/managers/emailmanager.cpp)
  add_definitions(-DFAVOR_EMAIL_MANAGER)
  
  find_library(VMIME_LOCATION libvmime.a) #TODO: if this fails (VMIME_LOCATION-NOTFOUND), check for a user submitted path
  add_library(libvmime STATIC IMPORTED)
  set_property(TARGET libvmime PROPERTY IMPORTED_LOCATION VMIME_LOCATION)
  LIST(APPEND compilation_libraries ${libvmime})
ENDIF()

# Include Line
OPTION(FAVOR_LINE_MANAGER "Include the MessageManager for Line messaging client" ON)


add_library(favor SHARED ${compilation_files})

SET(CMAKE_CXX_FLAGS "-std=c++11")
target_link_libraries (favor ${compilation_libraries})