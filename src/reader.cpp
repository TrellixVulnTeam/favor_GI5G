#include "reader.h"

namespace favor{
  namespace reader{
    namespace{ //Private members
      sqlite3 *db;
    }
    void initialize(){
      sqlv(sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READONLY, NULL));
    }
    void cleanup(){
      sqlv(sqlite3_close(db));
    }
  }
}