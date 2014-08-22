#include "favor.h"
#include "lib/sqlite/sqlite3.h"
#include "worker.h"

using namespace std;

namespace favor{
  void initialize(){
    sqlite3_enable_shared_cache(1);
    //The worker must be initialized first, because it will create the database if there is none
    worker::initialize();
  }
  
  void cleanup(){
    
  }
}