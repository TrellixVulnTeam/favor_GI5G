#ifndef favor_messagequery_include
#define favor_messagequery_include

#include "favor.h"
#include "logger.h"
#include "address.h"
#include "accountmanager.h"

//TODO: make specific instantiations of the class for specific operations


namespace favor {
    template <typename T>
    class MessageQuery {
    protected:
        //TODO: some of these can probably be private but I'll have to go back and look at which
        enum BoundParam {ADDRESS_START, FROMDATE, UNTILDATE};
        int bindingIndices[3];
        int currentBindingIndex = 1;
        time_t fromDate;
        time_t untilDate;

        shared_ptr<vector<Address>> addresses;
        bool getAll = false;

        unsigned short currentKeyIndex = 0;
        unsigned short keyIndices[17] = {0}; //So we can just use the flags as indices
        Key keys;

        string sql;
        sqlite3_stmt* stmt;
        sqlite3* const db;

        bool empty = false;

        T result;

        void appendWhereClause();
        void bindWhereClause();
        //TODO: this doesn't quite work for a multi-table query because while it would owrk if we could somehow magically call both twice,
        //the second one has to be


        string keyString();

    public:
        //enum Operations {SELECT_KEYS, SUM, AVG, SELECT_KEYS_MULTITABLE};

        MessageQuery(const AccountManager* account, Key k, time_t fDate, time_t uDate, sqlite3 *database);

        void setGetAll();

        bool isEmpty();

        T getResult();

        virtual void finalize() = 0;
    };

    template <typename T>
    class CollationQuery : public MessageQuery<int> {
    public:
        enum CollationType {SUM, AVERAGE, COUNT};
    private:
        CollationType operation;
        const string tableName;
    public:
        //TODO: count should count body so we ignore null messages
        void finalize() override;

        CollationQuery(const AccountManager* account, Key k, time_t fDate, time_t uDate, sqlite3 *database, CollationType op, bool sent);
        };


}


#endif