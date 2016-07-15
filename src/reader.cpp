/*
Copyright (C) 2015  Joshua Tanner (mindful.jt@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "reader.h"

namespace favor {
    namespace reader {
        namespace { //Private members
            sqlite3 *db;

            std::mutex accountMutex;
            thread_local int accountsHolderCount = 0;
            list<AccountManager*>* _accounts; //Have to hold pointers for polymorphism, list is pointer for uniformity with other data

            std::mutex contactsMutex;
            thread_local int contactsHolderCount = 0;
            list<Contact>* _contacts;
            bool contactsValid = false;

            enum QueryBindables {ADDRESSES_START, FROM_DATE, UNTIL_DATE};
            typedef std::unordered_map<int, int> Indices;


            /*
                This is so we can write to these inside the reader, but nowhere else
             */
            template <typename T>
            class WriteableDataLock : public DataLock<T> {
            public:
                T* operator->() const {
                    if (!this->valid()) throw threadingException("Cannot reference invalid data lock");
                    return this->data;
                }

                T& operator*() const {
                    if (!this->valid()) throw threadingException("Cannot reference invalid data lock");
                    return *(this->data);
                }

                WriteableDataLock(std::mutex* mut, int* hcount, T* protected_data) : DataLock<T>(mut, hcount, protected_data){}
                WriteableDataLock(const favor::DataLock<T> &other) : DataLock<T>(other) {}

            };

            WriteableDataLock<list<AccountManager*>> writeableAccountList() {
                //The DataLock constructor will block if we can't get a lock.
                return WriteableDataLock<list<AccountManager*>>(&accountMutex, &accountsHolderCount, _accounts);
            }


            WriteableDataLock<list<Contact>> writeableContactList(){
                if (!contactsValid) refreshContactList();
                return WriteableDataLock<list<Contact>>(&contactsMutex, &contactsHolderCount, _contacts);
            }
        }

        void initialize() {
            sqlv(sqlite3_open_v2(DB_PATH_FULL, &db, SQLITE_OPEN_READONLY, NULL));
            _accounts = new std::list<AccountManager*>;
            _contacts = new std::list<Contact>;
        }

        void cleanup() {
            sqlv(sqlite3_close(db));
            writeableAccountList()->clear();
            writeableContactList()->clear();
        }

        DataLock<list<AccountManager*>> accountList() {
            //The DataLock constructor will block if we can't get a lock.
            return DataLock<list<AccountManager*>>(&accountMutex, &accountsHolderCount, _accounts);
        }


        DataLock<list<Contact>> contactList(){
            if (!contactsValid) refreshContactList();
            return DataLock<list<Contact>>(&contactsMutex, &contactsHolderCount, _contacts);
        }

        void removeAccount(AccountManager* account){
            writeableAccountList()->remove(account);
            delete account; //If the reader doesn't have it, no one should
        }

        void addAccount(AccountManager* account){
            writeableAccountList()->push_back(account);
        }

        //Something has changed, and the contacts list needs refreshing
        void invalidateContactList(){
            DataLock<list<Contact>>(&contactsMutex, &contactsHolderCount, _contacts); //Just for locking purposes
            contactsValid = false;
        }


        void refreshAll() {
            refreshAccountList();
            refreshContactList();
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
                keyIndices[KEY_BODY] = currentKeyIndex++;
            }
            if (keys & KEY_ID){
                keystring += "id,";
                keyIndices[KEY_ID] = currentKeyIndex; //This doesn't increment, but only because it's the very last. MUST increment indices except the last one checked
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
                    bindings[ADDRESSES_START] = currentBinding;
                    for (int i = 0; i < addresses->size(); ++i){
                        selection += "address=?";
                        currentBinding++;
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
                    //logger::info("Binding address in slot "+as_string(bindings[ADDRESSES_START]+i)+" as "+(*addresses)[i].addr);
                    sqlv(sqlite3_bind_text(stmt, bindings[ADDRESSES_START]+i, (*addresses)[i].addr.c_str(), (*addresses)[i].addr.length(), SQLITE_STATIC));
                }
            }
            if (bindings.count(FROM_DATE)){
                //logger::info("Binding fromDate in slot "+as_string(bindings[FROM_DATE])+" as "+as_string(fromDate));
                sqlv(sqlite3_bind_int64(stmt, bindings[FROM_DATE], fromDate));
            }
            if (bindings.count(UNTIL_DATE)){
                //logger::info("Binding untilDate in slot "+as_string(bindings[UNTIL_DATE])+" as "+as_string(untilDate));
                sqlv(sqlite3_bind_int64(stmt, bindings[UNTIL_DATE], untilDate));
            }
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

        enum ComputeCommand {SUM, COUNT, AVERAGE, MAX};
        const char* const ComputeCommandName[4] = {"SUM", "COUNT", "AVG", "MAX"};

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
                case SUM:
                    result = sqlite3_column_int64(stmt, 0);
                    break;
                case COUNT:
                    result = sqlite3_column_int64(stmt, 0);
                    break;
                case AVERAGE:
                    result = sqlite3_column_double(stmt, 0);
                    break;
                case MAX:
                    result = sqlite3_column_int64(stmt, 0);
                    break;
                default:
                    throw queryException("Unsupported SQLite compute command issued");
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

        shared_ptr<vector<Message>> queryConversation(const AccountManager* account, const Contact& c, Key keys, time_t fromDate, time_t untilDate){
            keys = keys | KEY_DATE; //This has to be in here whether it's requested or not for sorting reasons
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

            //The offset adjustment for ADDRESSES_START will be 0 when it's just one address (which is fine)
            //The important thing is that in cases of more than one address we adjust properly to account for the
            //fact that multiple addresses take up multiple bindings.
            int selectionOffset = selectionIndices.size();
            if (selectionIndices.count(ADDRESSES_START)) selectionOffset += (addresses->size() -1 );
            for (auto it = selectionIndices.begin(); it != selectionIndices.end(); ++it){
                it->second += selectionOffset; //Slide every index over by the number of indices so we can bind the second set
            }
            bindSelection(addresses, fromDate, untilDate, stmt, selectionIndices);

            shared_ptr<vector<Message>> ret = std::make_shared<vector<Message>>();
            int result;

            while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
                //keyIndices.size() is the number of keys already used, but they start at 0, so that value will be our unused (sent) index
                ret->push_back(buildMessage(stmt, keyIndices, (bool) sqlite3_column_int(stmt, keyIndices.size()), account->type));
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
        long count(const AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<long>(COUNT, &(c.getAddresses()), account->getTableName(sent), KEY_DATE, fromDate, untilDate);
        }
        long max(const AccountManager* account, const Contact& c, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<long>(MAX, &(c.getAddresses()), account->getTableName(sent), key, fromDate, untilDate);
        }

        long sumAll(const AccountManager* account, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<long>(SUM, NULL, account->getTableName(sent), key, fromDate, untilDate);
        }

        double averageAll(const AccountManager* account, Key key, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<double>(AVERAGE, NULL, account->getTableName(sent), key, fromDate, untilDate);
        }

        long countAll(const AccountManager* account, time_t fromDate, time_t untilDate, bool sent){
            return sqlComputeCommand<long>(SUM, NULL, account->getTableName(sent), KEY_DATE, fromDate, untilDate);
        }

        /*
           REFRESH AND OTHER GETTERS
         */


        void refreshAccountList() {
            sqlite3_stmt *stmt;
            const char sql[] = "SELECT * FROM " ACCOUNT_TABLE ";"; //Important this is an array and not a const char* so that sizeof() works properly
            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
            int result;
            auto accounts = writeableAccountList();
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

        void refreshContactList(){
            sqlite3_stmt* stmt;
            string sql = "SELECT * FROM " CONTACT_TABLE ";";
            int result;
            std::unordered_map<int, string> contactHolder;
            std::unordered_map<int, vector<Address>> contactAddresses;
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

            while ((result = sqlite3_step(stmt)) == SQLITE_ROW){
                int id = sqlite3_column_int(stmt, 0);
                //using the [] operator is no good
                contactHolder.emplace(id, reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
            }
            sqlv(result);
            sqlv(sqlite3_finalize(stmt));
            //Get this manually because the method to get it from outside the worker could trigger infinite recursion if it's invalid
            WriteableDataLock<list<Contact>> contacts = WriteableDataLock<list<Contact>>(&contactsMutex, &contactsHolderCount, _contacts);
            contacts->clear();

            shared_ptr<list<Address>> addrs = addresses(FLAG_ALL, true);
            for (auto it = addrs->begin(); it != addrs->end(); it++){
                contactAddresses[it->contactId].push_back(*it);
            }

            //Add canonical contacts in hash table to list
            for (auto it = contactHolder.begin(); it != contactHolder.end(); it++){
                //Using [] here will give us an empty vector if there isn't a record in contactAddresses, but this is intentional
                //because there will be no records for contacts without any addresses currently assigned in contactAddresses
                contacts->push_back(Contact(it->first, it->second, contactAddresses[it->first]));
            }
            contactsValid = true;
        }

        shared_ptr<list<Address>> addresses(const MessageType &t, bool contactRelevantOnly){
            sqlite3_stmt* stmt;
            string sql;
            if (contactRelevantOnly) sql = "SELECT * FROM " ADDRESS_TABLE(t) " WHERE contact_id IS NOT NULL;";
            else sql = "SELECT * FROM " ADDRESS_TABLE(t) ";";
            int result;
            shared_ptr<list<Address>> ret = std::make_shared<list<Address>>();
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            while ((result = sqlite3_step(stmt)) == SQLITE_ROW){
                ret->push_back(Address(
                        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                        sqlite3_column_int(stmt, 1),
                        sqlite3_column_type(stmt, 2) == SQLITE_NULL ? Address::NO_CONTACT_ID : sqlite3_column_int(stmt, 2),
                        t));
                //-1 if there's no corresponding contact, otherwise that contact's ID

            }
            sqlv(result);
            sqlv(sqlite3_finalize(stmt));
            return ret;
        }
        
        shared_ptr<list<Address>> addresses(const MessageTypeFlag &ts, bool contactRelevantOnly){
            if (ts == FLAG_EMPTY){
                logger::warning("addresses called with empty MessageTypeFlag");
                return std::make_shared<list<Address>>();
            }
            sqlite3_stmt* stmt;
            string sql;
            for (short i = 0; i < NUMBER_OF_TYPES; ++i){
                if (MessageTypeFlags[i] & ts){
                    if (i > 0) sql += " UNION ";
                    if (contactRelevantOnly) sql += "SELECT *,? as type FROM " ADDRESS_TABLE(i)" WHERE contact_id IS NOT NULL";
                    else sql += "SELECT *,? as type FROM " ADDRESS_TABLE(i);
                }
            }
            sql += ";";

            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

            int bindingIndex = 1;
            for (short i = 0; i < NUMBER_OF_TYPES; ++i){
                if (MessageTypeFlags[i] & ts) sqlv(sqlite3_bind_int(stmt, bindingIndex++, i));
            }

            int result;
            shared_ptr<list<Address>> ret = std::make_shared<list<Address>>();
            while ((result = sqlite3_step(stmt)) == SQLITE_ROW){
                ret->push_back(Address(
                        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                        sqlite3_column_int(stmt, 1),
                        sqlite3_column_type(stmt, 2) == SQLITE_NULL ? Address::NO_CONTACT_ID : sqlite3_column_int(stmt, 2),
                        (MessageType) sqlite3_column_int(stmt, 3)));
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