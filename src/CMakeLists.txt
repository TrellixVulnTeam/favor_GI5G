#Build the core Favor library

find_package(Threads REQUIRED)

set(compilation_files favor.cpp processor.cpp reader.cpp worker.cpp accountmanager.cpp message.cpp contact.cpp logger.cpp) #Core files
LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/lib/sqlite/sqlite3.c) #Sqlite
add_definitions(-DSQLITE_THREADSAFE=2)
LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/lib/pugixml/pugixml.cpp)

set(compilation_libraries ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS}) #-pthread and #-ldl

OPTION(FAVOR_DEBUG "Favor's debug and test settings" ON)
IF(FAVOR_DEBUG)
  ################################
  # GTest
  ################################
  add_definitions(-DDEBUG)
  ADD_SUBDIRECTORY (lib/gtest)
  set(CMAKE_BUILD_TYPE Debug)
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

  # Link test executable against gtest & gtest_main
  LIST(APPEND compilation_libraries gtest gtest_main)
ENDIF()

SET(CMAKE_CXX_FLAGS "-std=c++11 -O0 -g")


# Include Email
OPTION(FAVOR_EMAIL_MANAGER "Include the MessageManager for Email" ON)
IF(FAVOR_EMAIL_MANAGER)
  LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/managers/emailmanager.cpp)
  add_definitions(-DFAVOR_EMAIL_MANAGER)
  
  #VMIME
  LIST(APPEND compilation_libraries ${favor_SOURCE_DIR}/src/lib/vmime/lib/libvmime.a)

  #OpenSSL, which must come after Vmime:
  #he traditional behavior of linkers is to search for external functions from left to right in the libraries
  #specified on the command line. This means that a library containing the definition of a function should appear
  #after any source files or object files which use it.
  find_package(OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})
  LIST(APPEND compilation_libraries ${OPENSSL_LIBRARIES})

  
  #TIDY
#  find_library(TIDY_LOCATION libtidy.a PATHS ${favor_SOURCE_DIR}/src/lib/tidy-html5/lib NO_DEFAULT_PATH)
#  add_library(libtidy STATIC IMPORTED)
#  set_target_properties(libtidy PROPERTIES IMPORTED_LOCATION ${TIDY_LOCATION})
  list(APPEND compilation_libraries ${favor_SOURCE_DIR}/src/lib/tidy-html5/libtidys.a)
ENDIF()

# Include Line
OPTION(FAVOR_LINE_MANAGER "Include the MessageManager for Line messaging client" ON)
IF(FAVOR_LINE_MANAGER)
  add_definitions(-DFAVOR_LINE_MANAGER)
  LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/managers/linemanager.cpp)
ENDIF()

#Include Skype
OPTION(FAVOR_SKYPE_MANAGER "Include the MessageManager for Skype" ON)
IF(FAVOR_SKYPE_MANAGER)
  add_definitions(-DFAVOR_SKYPE_MANAGER)
  LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/managers/skypemanager.cpp)
ENDIF()


add_library(favor SHARED ${compilation_files})

target_include_directories(favor PUBLIC lib/gtest/include)
target_include_directories(favor PUBLIC lib/vmime/include)
target_include_directories(favor PUBLIC managers)
target_include_directories(favor PUBLIC .) #So managers can get at core files
target_include_directories(favor PUBLIC lib)
target_link_libraries(favor ${compilation_libraries})