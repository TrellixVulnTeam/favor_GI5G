#ifndef favor_managers_include
#define favor_managers_include

#include "../favor.h"
#include "../accountmanager.h"

namespace favor{
  //Forward declarations of manager subclasses so we can use their constructors
  class EmailManager : public AccountManager {public: EmailManager(string accNm, string detailsJson);};
  //class LineManager : public AccountManager {public: LineManager(string accNm, string detailsJson);}; //etc
  
}

#endif