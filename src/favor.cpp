#include "favor.h"
#include "worker.h"
#include "reader.h"
#include "logger.h"

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
  
  //These methods will not behave well with numbers whose digit count exceeds the DIGIT_MAX
  #define INT_DIGIT_MAX 10 
  #define LONG_DIGIT_MAX 20
  string int_string(int val){
    char result[INT_DIGIT_MAX] = {0};
    sprintf(result, "%d", val);
    return string(result);
  }
  string long_string(long val)
  {
    char result[LONG_DIGIT_MAX] = {0};
    sprintf(result, "%ld", val);
    return string(result);
  }


  void sqlite3_validate(int result, sqlite3 *db ){
    switch(result){
      case SQLITE_OK: break;
      case SQLITE_ROW: break;
      case SQLITE_DONE: break;
      default:
	logger::error("SQLite error #"+long_string(result)+", db says \""+sqlite3_errmsg(db)+"\"");
	//TODO: throw exception
    }
  }

}