#include "logger.h"

namespace favor{
  namespace logger{
    namespace {
      inline void log(string s){
	//TODO: this should write to an actual log file
	time_t t = time(NULL);
	cout << t << ":" << s << endl;
      }
    }
    void error(string s)
    {
	log("error:"+s);
    }
    //Other types of errors should check verbosity level, I think
    
  }
}