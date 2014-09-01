#ifndef favor_core_include
#define favor_core_include

#include <string>
#include <vector>
#include <list>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <cassert>

#include <iostream> //TODO: necessary?

#include "lib/sqlite/sqlite3.h"
#include "definitions.h"
#include "exceptions.h"

using namespace std;

namespace favor{
  //Basic
  void initialize();
  void cleanup();
  //Utility
  string int_string(int);
  string long_string(long);
  void sqlite3_validate(int result, sqlite3 *db);
}

#endif