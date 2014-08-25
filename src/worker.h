#ifndef favor_worker_include
#define favor_worker_include

#include "favor.h"

namespace favor{
  namespace worker{
    void initialize();
    void cleanup();
    void buildDatabase();
  }
}

#endif