#include "reader.h"

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

            enum QueryBindables {ADDRESSES_START, FROM_DATE, UNTIL_DATE};
            typedef std::unordered_map<int, int> Indices;
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


        /*
        QUERY HELPER METHODS
         */

        //Returns the computed string, the indices to be used for getting the appropriate values later. SQLite indices always from 0
        std::pair<string, Indices> computeKeys(Key keys){
            string keystring;
            Indices keyIndices;
            int currentKeyIndex = 0;
            if (keys & KEY_ADDRESS){
                keystring += "address,";
                keyIndices[KEY_ADDRESS] = currentKeyIndex++;
            }
            if (keys & KEY_DATE){
                keystring += "date,";
                keyIndices[KEY_DATE] = currentKeyIndex++;
            }
            if (keys & KEY_CHARCOUNT){
                keystring += "charcount,";
                keyIndices[KEY_CHARCOUNT] = currentKeyIndex++;
            }
            if (keys & KEY_MEDIA){
                keystring += "media,";
                keyIndices[KEY_MEDIA] = currentKeyIndex++;
            }
            if (keys & KEY_BODY){
                keystring += "body,";
                keyIndices[KEY_BODY] = currentKeyIndex;
            }
            if (keys & KEY_ID){
                keystring += "id,";
                keyIndices[KEY_ID] = currentKeyIndex;
            }
            keystring = keystring.substr(0, keystring.length()-1); //Strip last comma
            return std::pair<string, Indices>(keystring, keyIndices);
        }

        //Returns the computed string, the indices to be used for binding corresponding terms later. SQLite bindings always from 1
        std::pair<string, Indices> computeSelection(const vector<Address>* addresses, time_t fromDate, time_t untilDate, int bindingOffset = 1){
            string selection = "WHERE ";
            bool fresh = true;
            int currentBinding = bindingOffset;
            Indices bindings;
            if (addresses != NULL){
                fresh = false;
                if (addresses->size() == 0 ){
                    selection += "1=2"; //Always false, returns nothing
                } else {
                    selection += "(";
                    bindings[ADDRESSES_START] = currentBinding++;
                    for (int i = 0; i < addresses->size(); ++i){
                        selection += "address=?";
                        if (i != addresses->size() -1 ) selection += " OR ";
                    }
                    selection += ")";
                }
            }
            if (fromDate != -1){
                if (!fresh) selection += " AND ";
                fresh = false;
                selection += "date >=?";
                bindings[FROM_DATE] = currentBinding++;
            }
            if (untilDate != -1){
                if (!fresh) selection += " AND ";
                fresh = false;
                selection += "date <=?";
                bindings[UNTIL_DATE] = currentBinding++;
            }
            if (fresh) return std::pair<string, Indices>("", bindings);
            else return std::pair<string, Indices>(selection, bindings);
        }

        void bindSelection(const vector<Address>* addresses, time_t fromDate, time_t untilDate, sqlite3_stmt* stmt, Indices bindings){
            if (bindings.count(ADDRESSES_START)){
                for (int i = 0; i < addresses->size(); ++i){
                    sqlv(sqlite3_bind_text(stmt, bindings[ADDRESSES_START]+i, (*addresses)[i].addr.c_str(), (*addresses)[i].addr.length(), SQLITE_STATIC));
                }
            }
            if (bindings.count(FROM_DATE)) sqlv(sqlite3_bind_int64(stmt, bindings[FROM_DATE], fromDate));
            if (bindings.count(UNTIL_DATE)) sqlv(sqlite3_bind_int64(stmt, bindings[UNTIL_DATE], untilDate));
        }

        Message buildMessage(sqlite3_stmt* stmt, Indices keyIndices, bool sent, MessageType type){
            if (keyIndices.count(KEY_BODY)) {
                return Message(type,
                        sent,
                        (keyIndices.count(KEY_ID) ? sqlite3_column_int64(stmt, keyIndices[KEY_ID]) : Message::UNKNOWN_NUMERIC_VALUE),
                        (keyIndices.count(KEY_DATE) ? sqlite3_column_int64(stmt, keyIndices[KEY_DATE]) : Message::UNKNOWN_NUMERIC_VALUE),
                        (keyIndices.count(KEY_ADDRESS) ? reinterpret_cast<const char *>(sqlite3_column_text(stmt, keyIndices[KEY_ADDRESS])) : Message::UNKNOWN_ADDRESS_VALUE),
                        (keyIndices.count(KEY_MEDIA) ? (bool) sqlite3_column_int(stmt, keyIndices[KEY_MEDIA]) : Message::UNKNOWN_NUMERIC_VALUE),
                        (keyIndices.count(KEY_CHARCOUNT) ? sqlite3_column_int64(stmt, keyIndices[KEY_CHARCOUNT]) : Message::UNKNOWN_NUMERIC_VALUE),
                        reinterpret_cast<const char *>(sqlite3_column_text(stmt, keyIndices[KEY_BODY]))
                );
            }
            else {
                return Message(type,
                        sent,
                        (keyIndices.count(KEY_ID) ? sqlite3_column_int64(stmt, keyIndices[KEY_ID]) : Message::UNKNOWN_NUMERIC_VALUE),
                        (keyIndices.count(KEY_DATE) ? sqlite3_column_int64(stmt, keyIndices[KEY_DATE]) : Message::UNKNOWN_NUMERIC_VALUE),
                        (keyIndices.count(KEY_ADDRESS) ? reinterpret_cast<const char *>(sqlite3_column_text(stmt, keyIndices[KEY_ADDRESS])) : Message::UNKNOWN_ADDRESS_VALUE),
                        (keyIndices.count(KEY_MEDIA) ? (bool) sqlite3_column_int(stmt, keyIndices[KEY_MEDIA]) : Message::UNKNOWN_NUMERIC_VALUE),
                        (keyIndices.count(KEY_CHARCOUNT) ? sqlite3_column_int64(stmt, keyIndices[KEY_CHARCOUNT]) : Message::UNKNOWN_NUMERIC_VALUE)
                );
            }

        }

        /*
          ACTUAL QUERY METHODS
         */

        enum ComputeCommand {SUM, COUNT, AVERAGE};
        const char* const ComputeCommandName[3] = {"SUM", "COUNT", "AVG"};

        template <typename T>
        T sqlComputeCommand(ComputeCommand cmd, const vector<Address>* addresses, const string& tableName, Key key, time_t fromDate, time_t untilDate){
            sqlite3_stmt* stmt;
            string sql = "SELECT "+string(ComputeCommandName[cmd])+"(";

            std::pair<string, Indices> keyResult = computeKeys(key);
            sql += keyResult.first;
            Indices keyIndices = keyResult.second;
            if (keyIndices.size() > 1){
                logger::error("Attempted to run SQLite collation command "+string(ComputeCommandName[cmd])+" with multi-column flags"+as_string((int)key));
                throw queryException("SQLite collation commands should only be run on a single column");
            }

            sql += ") FROM "+tableName+" ";

            std::pair<string, Indices> selectionResult = computeSelection(addresses, fromDate, untilDate);
            sql += selectionResult.first;
            Indices selectionIndices = selectionResult.second;

            sql += ";";

            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            bindSelection(addresses, fromDate, untilDate, stmt, selectionIndices);
            sqlv(sqlite3_step(stmt));
            T result;
            switch(cmd){
                case SUM: result = sqlite3_column_int64(stmt, 0);
                case COUNT: result = sqlite3_column_int64(stmt, 0);
                case AVERAGE: result = sqlite3_column_double(stmt, 0);
            }
            sqlv(sqlite3_finalize(stmt));
            return result;
        }

        shared_ptr<vector<Message>> query(const vector<Address>* addresses, const string& tableName, bool sent, MessageType type, Key keys, time_t fromDate, time_t untilDate){
            sqlite3_stmt* stmt;
            string sql = "SELECT ";

            std::pair<string, Indices> keyResult = computeKeys(keys);
            sql += keyResult.first;
            Indices keyIndices = keyResult.second;

            sql += " FROM "+tableName+" ";

            std::pair<string, Indices> selectionResult = computeSelection(addresses, fromDate, untilDate);
            sql += selectionResult.first;
            Indices selectionIndices = selectionResult.second;

            sql += " ORDER BY date " DB_SORT_ORDER ";";

            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            bindSelection(addresses, fromDate, untilDate, stmt, selectionIndices);
            shared_ptr<vector<Message>> ret = std::make_shared<vector<Message>>();
            int result;
            while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
                ret->push_back(buildMessage(stmt, keyIndices, sent, type));
            }
            sqlv(result); //Make sure we broke out of the loop with good results
            sqlv(sqlite3_finalize(stmt));
            return ret;
        }

        //TODO: TEST THIS METHOD
        shared_ptr<vector<Message>> queryConversation(const AccountManager* account, const Contact& c, Key keys, time_t fromDate, time_t untilDate){
            const vector<Address>* addresses = &(c.getAddresses());
            const string sentTableName(account->getTableName(true));
            const string receivedTableName(account->getTableName(false));

            sqlite3_stmt *stmt;
            string sql = "SELECT ";
            std::pair<string, Indices> keyResult = computeKeys(keys);
            std::pair<string, Indices> selectionResult = computeSelection(addresses, fromDate, untilDate);

            sql += keyResult.first+",1 as sent";
            sql += " FROM "+sentTableName+" "+selectionResult.first+" UNION SELECT ";

            sql += keyResult.first+",0 as sent";
            sql += " FROM "+receivedTableName+" "+selectionResult.first;

            sql += " ORDER BY date " DB_SORT_ORDER ";";

            Indices selectionIndices = selectionResult.second;

            Indices keyIndices = keyResult.second;

            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            bindSelection(addresses, fromDate, untilDate, stmt, selectionIndices); //Bind the first set
            for (auto it = selectionIndices.begin(); it != selectionIndices.end(); ++it){
                it->second += selectionIndices.size(); //Slide every index over by the number of indices so we can bind the second set
            }
            bindSelection(addresses, fromDate, untilDate, stmt, selectionIndices);

            shared_ptr<vector<Message>> ret = std::make_shared<vector<Message>>();
            int result;

            while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
                ret->push_back(buildMessage(stmt, keyIndices, (bool) sqlite3_column_int(stmt, keyIndices.size()+1), account->type));
            }
            sqlv(result); //Make sure we broke out of the loop with good results
            sqlv(sqlite3_finalize(stmt));
            return ret;
        }

        shared_ptr<vector<Message>> queryAll(const AccountManager* account, const Key keys, time_t fromDate, time_t untilDate, bool sent){
            return query(NULL, account->getTableName(sent), sent, account->type, keys, fromDate, untilDate);
        }
        shared_ptr<vector<Message>> queryContact(const AccountManager* account, const Contact& c, Key keys, time_t fromDate, time_t untilDate, bool sent){
            return query(&(c.getAddresses()), account->getTableName(sent), sent, account->type, keys, fromDate, untilDate);
        }

        long sum(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<long>(SUM, &(c.getAddresses()), account->getTableName(sent), key, fromDate, untilDate);
        }
        double average(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<double>(AVERAGE, &(c.getAddresses()), account->getTableName(sent), key, fromDate, untilDate);
        }

        long sumAll(const AccountManager* account, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<long>(SUM, NULL, account->getTableName(sent), key, fromDate, untilDate);
        }

        double averageAll(const AccountManager* account, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<double>(AVERAGE, NULL, account->getTableName(sent), key, fromDate, untilDate);
        }

        /*
           REFRESH AND OTHER GETTERS
         */


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