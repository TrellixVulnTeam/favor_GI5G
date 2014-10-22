#ifndef favor_worker_include
#define favor_worker_include

#include "favor.h"

namespace favor {
    namespace worker {
        //Basic
        void initialize();

        void cleanup();

        //Database
        void buildDatabase();

        void truncateDatabase();

        void indexDatabase();

        void deindexDatabase();


        void beginTransaction();

        void commitTransaction();

        void rollbackTransaction();

        //Writing methods
        void exec(string sql);

    }
}

#endif