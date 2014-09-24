#ifndef favor_core_include
#define favor_core_include

#include <string>
#include <vector>
#include <list>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <memory>
#include <iostream>
#include <utility>

#include "lib/sqlite/sqlite3.h"
#include "lib/rapidjson/document.h"
#include "lib/rapidjson/error/en.h"
#include "definitions.h"
#include "exceptions.h"
#include "types.h"

namespace favor{
  class message;
  //Basic
  void initialize();
  void cleanup();
  //Utility
  void sqlite3_validate(int result, sqlite3 *db);
  string as_string(int i);
  string as_string(long l);
  string as_string(message m);
}

#endif