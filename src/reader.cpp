#include "reader.h"

namespace favor {
    namespace reader {
        namespace { //Private members
            sqlite3 *db;
            list<shared_ptr<AccountManager>> accounts;
        }

        void initialize() {
            sqlv(sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READONLY, NULL));
        }

        void cleanup() {
            sqlv(sqlite3_close(db));
        }

        list<shared_ptr<AccountManager>> accountList() {
            //TODO: take reader lock
            return accounts;
            //TODO: release reader lock
        }

        void refreshAll() {
            refreshAccountList();
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


    }
}