#ifndef favor_core_include
#define favor_core_include

#include <string>
#include <ctime>
#include <cstring>
#include <iostream> //TODO: necessary?
#include "lib/sqlite/sqlite3.h"
#include "definitions.h"

using namespace std;

namespace favor{
  //Basic
  void initialize();
  void cleanup();
  //Utility
  void sqlite3_validate(int result);
}

#endif