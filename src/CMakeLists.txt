#Build the core Favor library

find_package(Threads)

set(compilation_files favor.cpp processor.cpp reader.cpp worker.cpp accountmanager.cpp message.cpp contact.cpp logger.cpp) #Core files
LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/lib/sqlite/sqlite3.c) #Sqlite
add_definitions(-DSQLITE_THREADSAFE=2)
LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/lib/pugixml/pugixml.cpp)




SET(CMAKE_CXX_FLAGS "-std=c++11")
set(compilation_libraries ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
#${CMAKE_DL_LIBS} is our "-ldl"
#${CMAKE_THREAD_LIBS_INIT} is our "-pthread"


OPTION(FAVOR_DEBUG "Favor's debug and test settings" ON)
IF(FAVOR_DEBUG)
  ################################
  # GTest
  ################################
  add_definitions(-DDEBUG)
  ADD_SUBDIRECTORY (lib/gtest)
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

  # Link test executable against gtest & gtest_main
  LIST(APPEND compilation_libraries gtest gtest_main)
ENDIF()

# Include Email
OPTION(FAVOR_EMAIL_MANAGER "Include the MessageManager for Email" ON)
IF(FAVOR_EMAIL_MANAGER)
  LIST(APPEND compilation_files ${favor_SOURCE_DIR}/src/managers/emailmanager.cpp)
  add_definitions(-DFAVOR_EMAIL_MANAGER)
  
  #VMIME
  find_library(VMIME_LOCATION libvmime.so PATHS ${favor_SOURCE_DIR}/src/lib/vmime/lib NO_DEFAULT_PATH) #TODO: if this fails (VMIME_LOCATION-NOTFOUND), check for a vmime installation? Just drop NO_DEFAULT_PATH
  add_library(libvmime SHARED IMPORTED)
  set_target_properties(libvmime PROPERTIES IMPORTED_LOCATION ${VMIME_LOCATION})
  LIST(APPEND compilation_libraries libvmime)
  
  #TIDY (TODO: we may use this for everyone, and it does compile on Android, but for now it's email only)
  find_library(TIDY_LOCATION libtidy.a PATHS ${favor_SOURCE_DIR}/src/lib/tidy-html5/ NO_DEFAULT_PATH)
  add_library(libtidy STATIC IMPORTED)
  set_target_properties(libtidy PROPERTIES IMPORTED_LOCATION ${TIDY_LOCATION})
  list(APPEND compilation_libraries libtidy)
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
target_include_directories(favor PUBLIC managers)
target_include_directories(favor PUBLIC .) #So managers can get at core files
target_include_directories(favor PUBLIC lib)
target_link_libraries (favor ${compilation_libraries})