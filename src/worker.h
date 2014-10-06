#ifndef favor_worker_include
#define favor_worker_include

#include "favor.h"

namespace favor{
  namespace worker{
    //Basic
    void initialize();
    void cleanup();
    //Database
    void buildDatabase();
    void truncateDatabase();
    void indexDatabase();
    void deindexDatabase();
    //Writing methods
    void exec(string sql);
    void addAccount(string name, MessageType type, string detailsJson);
    void removeAccount(string name, MessageType type);
    void updateAccountDetails(string name, favor::MessageType type, favor::string detailsJson);
  }
}

#endif