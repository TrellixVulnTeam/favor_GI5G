#include "reader.h"

namespace favor {
    namespace reader {
        namespace { //Private members
            sqlite3 *db;
            list<AccountManager*> accounts;
        }

        void initialize() {
            sqlv(sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READONLY, NULL));
        }

        void cleanup() {
            sqlv(sqlite3_close(db));
        }

        list<AccountManager*> accountList() {
            //TODO: take reader lock, in case someone's writing
            return accounts;
            //TODO: release reader lock
        }

        void removeAccount(AccountManager* account){
            //TODO: writer lock
            accounts.remove(account);
            delete account; //If the reader doesn't have it, no one should
            //TODO: release writer lock
        }

        void addAccount(AccountManager* account){
            //TODO: writer lock
            accounts.push_back(account);
            //TODO: release wrtier lock
        }

        void refreshAll() {
            refreshAccountList();
            //TODO: refresh metrics
        }

        void refreshAccountList() //TODO: test this code
        {
            sqlite3_stmt *stmt;
            const char sql[] = "SELECT * FROM " ACCOUNT_TABLE ";"; //Important this is an array and not a const char* so that sizeof() works properly
            //TODO: take writer lock

            accounts.erase(accounts.begin(), accounts.end());

            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
            int result;
            while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
                accounts.push_back(AccountManager::buildManager(
                        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)),
                        static_cast<MessageType>(sqlite3_column_int(stmt, 1)),
                        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2))));
            }
            sqlv(result); //Make sure we broke out of the loop with good results
            //TODO: release writer lock
        }

        list<favor::contact> contacts(const MessageType& t){
            sqlite3_stmt *stmt;
            string sql = "SELECT * FROM " CONTACT_TABLE(t) ";";
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            int result;
            list<contacts> ret;
            while ((result = sqlite3_step(stmt)) == SQLITE_ROW){
                //TODO: read contacts table, etc. Before that though, we have to update the contacts table schema
                //ret.push_back(contact())
            }


        }


    }
}