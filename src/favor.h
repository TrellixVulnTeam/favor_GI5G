#ifndef favor_core_include
#define favor_core_include

#include <string>
#include "lib/sqlite/sqlite3.h"
#include "definitions.h"
using namespace std;

namespace favor{
  void initialize();
  void cleanup();
}

#endif