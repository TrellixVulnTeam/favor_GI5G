#include "worker.h"

namespace favor{
  namespace worker{
    namespace{  //Private members
      sqlite3 *db;
    }
    void initialize(){
      int result;
      result = sqlite3_open(DB_NAME.c_str(), &db);
      //TODO: if result is bad, do something here
    }
  }
  
}