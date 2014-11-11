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

        void dropDatabase();

        void truncateDatabase();

        void indexDatabase();

        void deindexDatabase();


        void beginTransaction();

        void commitTransaction();

        void rollbackTransaction();

        //Writing methods
        void exec(string sql);

        void createContactWithAddress(const string &address, MessageType type, const string &displayName);

        void createContactFromAddress(const Address& addr, const string& displayName);

        void saveAddress(const Address& a, sqlite3_stmt* stmt);

        void rewriteAddressTable(const list<Address>& newAddresses, const MessageType& type);
    }
}

#endif