#ifndef favor_worker_include
#define favor_worker_include

#include "favor.h"

namespace favor{
  namespace worker{
    //Basic
    void initialize();
    void cleanup();
    //Database
    void buildDatabase();
    void truncateDatabase();
    void indexDatabase();
    void deindexDatabase();
    //Writing methods
    void sqlite3_exec_noread(string sql);
    void add_account(string name, MessageType type, string detailsJson);
  }
}

#endif