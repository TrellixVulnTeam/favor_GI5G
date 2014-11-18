#include "reader.h"
#include "logger.h"

namespace favor {
    namespace reader {
        namespace { //Private members
            sqlite3 *db;

            std::mutex accountMutex;
            thread_local int accountsHolderCount = 0;
            list<AccountManager*>* _accounts; //Have to hold pointers for polymorphism, list is pointer for uniformity with other data

            std::mutex contactsMutex[NUMBER_OF_TYPES];
            thread_local int contactsHolderCount[NUMBER_OF_TYPES] = {0};
            list<Contact>* _contacts[NUMBER_OF_TYPES];
            bool valid[NUMBER_OF_TYPES] = {false};
        }

        void initialize() {
            sqlv(sqlite3_open_v2(DB_PATH_FULL, &db, SQLITE_OPEN_READONLY, NULL));
            _accounts = new std::list<AccountManager*>;
            for (int i = 0; i < NUMBER_OF_TYPES; ++i){
                _contacts[i] = new std::list<Contact>;
            }
        }

        void cleanup() {
            sqlv(sqlite3_close(db));
        }

        DataLock<list<AccountManager*>> accountList() {
            //The DataLock constructor will block if we can't get a lock.
            return DataLock<list<AccountManager*>>(&accountMutex, &accountsHolderCount, _accounts);
        }


        DataLock<list<Contact>> contactList(const MessageType& t){
            if (!valid[t]) refreshContactList(t);
            return DataLock<list<Contact>>(&contactsMutex[t], &contactsHolderCount[t], _contacts[t]);
        }

        void removeAccount(AccountManager* account){
            accountList()->remove(account);
            delete account; //If the reader doesn't have it, no one should
        }

        void addAccount(AccountManager* account){
            accountList()->push_back(account);
        }

        //Something has changed, and the contacts list needs refreshing
        void invalidateContactList(MessageType t){
            DataLock<list<Contact>>(&contactsMutex[t], &contactsHolderCount[t], _contacts[t]); //Just for locking purposes
            valid[t] = false;
        }


        void refreshAll() {
            refreshAccountList();
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                refreshContactList((MessageType)i);
            }
        }

        string keysToString(Key key, bool maxOne){

        }

        string verifyAndComputeWhereClause(const vector<Address>* addresses, time_t fromDate, time_t untilDate){
            //TODO: this makes way less sense as a method when we have to bind the actual parameters later... should we just stringify things in here?
            //should we make this the last method in the chain and have it actually bind stuff? this all got substantially more complicated very quickly...
            const char* SELECTION_START = "WHERE ";
            string selection = SELECTION_START;
            if (addresses != NULL) {
                //Nothing to do here
            }
            if (fromDate != -1){
                if (selection != SELECTION_START) selection += " AND ";
                selection += "date >= ?";
            }
            if (untilDate != -1){
                if (selection != SELECTION_START) selection += " AND ";
                selection += "date <= ?";
            }
        }

        template <typename T>
        T sqlComputeCommand(const string& cmd, const vector<Address>* addresses, const string& tableName, Key key, time_t fromDate, time_t untilDate){
            string where = verifyAndComputeWhereClause(addresses, fromDate, untilDate);
        }

        shared_ptr<vector<Message>> query(const vector<Address>* addresses, const string& tableName, Key keys, time_t fromDate, time_t untilDate){
            string where = verifyAndComputeWhereClause(addresses, fromDate, untilDate);
        }

        shared_ptr<vector<Message>> queryConversation(const AccountManager* account, const Contact& c, Key keys, time_t fromDate, time_t untilDate, bool sent){
            const vector<Address>* addresses = &(c.getAddresses());
            string where = verifyAndComputeWhereClause(addresses, fromDate, untilDate);
        }

        shared_ptr<vector<Message>> queryAll(const AccountManager* account, const Key keys, time_t fromDate, time_t untilDate, bool sent){
            return query(NULL, account->getTableName(sent), keys, fromDate, untilDate);
        }
        shared_ptr<vector<Message>> queryContact(const AccountManager* account, const Contact& c, Key keys, time_t fromDate, time_t untilDate, bool sent){
            return query(&(c.getAddresses()), account->getTableName(sent), keys, fromDate, untilDate);
        }

        long sum(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<long>("SUM", &(c.getAddresses()), account->getTableName(sent), key, fromDate, untilDate);
        }
        double average(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<double>("AVG", &(c.getAddresses()), account->getTableName(sent), key, fromDate, untilDate);
        }

        double averageAll(const AccountManager* account, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<double>("AVG", NULL, account->getTableName(sent), key, fromDate, untilDate);
        }

        long sumAll(const AccountManager* account, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<long>("SUM", NULL, account->getTableName(sent), key, fromDate, untilDate);
        }


        void refreshAccountList() {
            sqlite3_stmt *stmt;
            const char sql[] = "SELECT * FROM " ACCOUNT_TABLE ";"; //Important this is an array and not a const char* so that sizeof() works properly
            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
            int result;
            auto accounts = accountList();
            for (auto it = accounts->begin(); it != accounts->end(); it++) delete *it;
            accounts->clear();

            while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
                accounts->push_back(AccountManager::buildManager(
                        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                        static_cast<MessageType>(sqlite3_column_int(stmt, 1)),
                        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
            }
            sqlv(result); //Make sure we broke out of the loop with good results
            sqlv(sqlite3_finalize(stmt));
        }

        void refreshContactList(const MessageType &t){
            sqlite3_stmt* stmt;
            string sql = "SELECT * FROM " CONTACT_TABLE(t) ";";
            int result;
            shared_ptr<list<Address>> addrs = addresses(t);
            std::unordered_map<int, Contact> contactHolder;
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

            while ((result = sqlite3_step(stmt)) == SQLITE_ROW){
                int id = sqlite3_column_int(stmt, 0);
                //contactHolder[id] = Contact(id, reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)), t); is no good
                contactHolder.emplace(id, Contact(id, reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)), t));
                //Canonical contacts added to hash table
            }
            sqlv(result);

            //Get this manually because the method to get it from outside the worker could trigger infinite recursion if it's invalid
            DataLock<list<Contact>> contacts = DataLock<list<Contact>>(&contactsMutex[t], &contactsHolderCount[t], _contacts[t]);
            contacts->clear();

            for (auto it = addrs->begin(); it != addrs->end(); it++){
                //TODO: this could conceivably, possible, throw an exception for being out of bounds, though it shouldn't
                //given the check we're making. we can catch it and rethrow it as bad data if it ever happens somehow though
                if (it->belongsToContact()) contactHolder.at(it->contactId).addAddress(*it);
            }

            //Add canonical contacts in hash table to list
            for (auto it = contactHolder.begin(); it != contactHolder.end(); it++){
                contacts->push_back(it->second);
            }
            valid[t] = true;
        }

        shared_ptr<list<Address>> addresses(const MessageType &t){
            sqlite3_stmt* stmt;
            string sql = "SELECT * FROM " ADDRESS_TABLE(t) ";";
            int result;
            shared_ptr<list<Address>> ret = std::make_shared<list<Address>>();
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            while ((result = sqlite3_step(stmt)) == SQLITE_ROW){
                ret->push_back(Address(
                        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                        sqlite3_column_int(stmt, 1),
                        sqlite3_column_type(stmt, 2) == SQLITE_NULL ? -1 : sqlite3_column_int(stmt, 2),
                        t));
                        //-1 if there's no corresponding contact, otherwise that contact's ID

            }
            sqlv(result);
            sqlv(sqlite3_finalize(stmt));
            return ret;
        }

        bool addressExists(const string& addr, const MessageType &t){
            sqlite3_stmt* stmt;
            string sql = "SELECT EXISTS(SELECT address FROM " ADDRESS_TABLE(t) " WHERE address=?);";
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            sqlv(sqlite3_bind_text(stmt, 1, addr.c_str(), addr.length(), SQLITE_STATIC));
            sqlv(sqlite3_step(stmt));
            bool exists = static_cast<bool>(sqlite3_column_int(stmt, 0));
            sqlv(sqlite3_finalize(stmt));
            return exists;
        }


    }
}