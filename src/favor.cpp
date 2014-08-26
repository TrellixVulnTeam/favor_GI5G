#include "favor.h"
#include "lib/sqlite/sqlite3.h"
#include "worker.h"
#include "reader.h"

using namespace std;

namespace favor{
  const char* MessageTypeName[] = {"email", "androidtext", "line", "skype"};
  void initialize(){
    sqlite3_enable_shared_cache(1);
    //The worker must be initialized first, because it will create the database if there is none
    worker::initialize();
    reader::initialize();
  }
  
  void cleanup(){
   worker::cleanup();
   reader::cleanup();
  }
}