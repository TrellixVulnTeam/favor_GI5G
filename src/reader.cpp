#include "reader.h"

namespace favor {
    namespace reader {
        namespace { //Private members
            sqlite3 *db;

            std::mutex accountMutex;
            list<AccountManager*>* accounts; //Have to hold pointers for polymorphism, list is pointer for uniformity with other data

            std::mutex contactsMutices[NUMBER_OF_TYPES];
            list<Contact>* contacts[NUMBER_OF_TYPES];
        }

        void initialize() {
            sqlv(sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READONLY, NULL));
            accounts = new std::list<AccountManager*>;
            for (int i = 0; i < NUMBER_OF_TYPES; ++i){
                contacts[i] = new std::list<Contact>;
            }
        }

        void cleanup() {
            sqlv(sqlite3_close(db));
        }

        list<AccountManager*>* accountList() {
            //TODO: take reader lock, in case someone's writing
            return accounts;
            //TODO: release reader lock
        }

//        DataLock<list<AccountManager*>> accountList() {
//            //TODO: take reader lock, in case someone's writing
//            return DataLock<list<AccountManager*>>(accountMutex, accounts);
//            //TODO: release reader lock
//        }

        list<Contact>* contactList(const MessageType& t){
            //TODO: reader lock
            return contacts[t];
            //TODO: release reader lock
        }

        void removeAccount(AccountManager* account){
            //TODO: writer lock
            accounts->remove(account);
            delete account; //If the reader doesn't have it, no one should
            //TODO: release writer lock
        }

        void addAccount(AccountManager* account){
            //TODO: writer lock
            accounts->push_back(account);
            //TODO: release wrtier lock
        }

        void refreshAll() {
            refreshAccountList();
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                refreshContactList((MessageType)i);
            }
        }

        void refreshAccountList() {
            sqlite3_stmt *stmt;
            const char sql[] = "SELECT * FROM " ACCOUNT_TABLE ";"; //Important this is an array and not a const char* so that sizeof() works properly
            //TODO: take writer lock
            for (auto it = accounts->begin(); it != accounts->end(); it++) delete *it;
            accounts->clear();

            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
            int result;

            while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
                accounts->push_back(AccountManager::buildManager(
                        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                        static_cast<MessageType>(sqlite3_column_int(stmt, 1)),
                        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
            }
            sqlv(result); //Make sure we broke out of the loop with good results
            //TODO: release writer lock
        }

        void refreshContactList(const MessageType &t){
            sqlite3_stmt* stmt;
            string sql = "SELECT * FROM " CONTACT_TABLE(t) ";";
            int result;
            shared_ptr<list<Address>> addrs = addresses(t);
            std::unordered_map<int, Contact> contactHolder;
            //TODO: writer lock
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

            while ((result = sqlite3_step(stmt)) == SQLITE_ROW){
                int id = sqlite3_column_int(stmt, 0);
                //contactHolder[id] = Contact(id, reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)), t); is no good
                contactHolder.emplace(id, Contact(id, reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)), t));
                //Canonical contacts added to hash table
            }
            sqlv(result);

            contacts[t]->clear();

            for (auto it = addrs->begin(); it != addrs->end(); it++){
                //TODO: this could conceivably, possible, throw an exception for being out of bounds, though it shouldn't
                //given the check we're making
                if (it->belongsToContact()) contactHolder.at(it->contactId).addAddress(*it);
            }

            //Add canonical contacts in hash table to list
            for (auto it = contactHolder.begin(); it != contactHolder.end(); it++){
                contacts[t]->push_back(it->second);
            }


            //TODO: drop writer lock
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
            return ret;
        }


    }
}