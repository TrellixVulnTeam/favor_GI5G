#ifndef favor_core_include
#define favor_core_include

#include <string>
#include "lib/sqlite/sqlite3.h"

namespace favor{
  const std::string DB_NAME="favor.db";
  void initialize();
  void cleanup();
}

#endif