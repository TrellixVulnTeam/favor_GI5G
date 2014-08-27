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
  
  void sqlite3_validate(int result){
    cout << result << endl;
    switch(result){
      case SQLITE_ROW: //TODO: this is a mess right now with stdout usage, but the important thing is that not every !=SQLITE_OK code is an error
	cout << "ROW" << endl;
        break;
      case SQLITE_DONE:
	cout << "DONE" << endl;
	break;
      default:
	logger::error(sqlite3_errstr(result));
    }
  }

}