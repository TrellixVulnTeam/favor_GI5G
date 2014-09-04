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
  void sqlite3_validate(int result, sqlite3 *db);
  string as_string(int i);
  string as_string(long l);
}

#endif