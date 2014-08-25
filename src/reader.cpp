#include "reader.h"

namespace favor{
  namespace reader{
    namespace{ //Private members
      sqlite3 *db;
    }
    void initialize(){
      int result = sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READONLY, "");
      if (result!=SQLITE_OK){
	//TODO: throw an exception or something
      }
    }
    void cleanup(){
      int result = sqlite3_close(db);
      if (result!=SQLITE_OK){
	//Log failure, probably means it's busy because we haven't finalized all of our statements
      }
    }
  }
}