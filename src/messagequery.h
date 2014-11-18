#ifndef favor_messagequery_include
#define favor_messagequery_include

#include "favor.h"
#include "logger.h"

//TODO: make specific instantiations of the class for specific operations


namespace favor {
    class Query {
    protected:
        //TODO: some of these can probably be private but I'll have to go back and look at which
        enum BindingIndex {ADDRESS_START, FROMDATE, UNTILDATE};
        BindingIndex indices[3];
        int currentBindingIndex = 0;
        time_t fromDate;
        time_t untilDate;
        const AccountManager* account;
        shared_ptr<vector<Address>> addresses;
        bool getAll = false;

        Key keys;

        bool queryReady = false;
        bool resultReady = false;

        string sql;
        sqlite3_stmt* stmt;

        bool empty = false;


    public:
        //Looks weird, but makes for really convenient use with the "sent" parameter in most methods
        enum Table {FROM = false, SENT = true, BOTH = 3};
        //enum Operations {SELECT_KEYS, SUM, AVG, SELECT_KEYS_MULTITABLE};

        Query(const AccountManager* account, const string& tName, Key k, time_t fDate, time_t uDate, Table table){

            fromDate = fDate;
            untilDate = uDate;
            if (fromDate > untilDate){
                logger::error("Could not construct valid query with fromDate "+as_string(fromDate)+" as it is > untilDate "+as_string(untilDate));
                throw queryException("Cannot run query with fromDate > untilDate");
            }

            //TODO: change the reader method so it spits out a vector because we're using it here now so reading speed matters slightly, and then get it and addresses= to it
            //TODO: probably need a getter to check if this is empty since if the actual SQL has to be run outside, we can't skip it based on this variable
            empty = (addresses->size()==0);

            keys = k;
        }

        virtual void prepareSql() = 0;
        //TODO: let this be used as an sqlite_statement as well with a conversion operator. thinking about this though, the method should construct the sql string but definitely not run
        //itself because that gets too complicated too fast. we'd need a DB pointer and it just gets ugly
    };


}

#endif