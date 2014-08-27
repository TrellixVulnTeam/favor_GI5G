#include "reader.h"

namespace favor{
  namespace reader{
    namespace{ //Private members
      sqlite3 *db;
    }
    void initialize(){
      sqlite3_validate(sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READONLY, "")); //TODO: this is broken
    }
    void cleanup(){
      sqlite3_validate(sqlite3_close(db));
    }
  }
}