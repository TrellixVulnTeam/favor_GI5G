#include "worker.h"
#include "reader.h"

namespace favor {
    namespace worker {
        namespace {  //Private members
            sqlite3 *db;
        }

        void initialize() {
            sqlv(sqlite3_open(DB_NAME, &db));
            exec("PRAGMA foreign_keys = ON;");
        }

        void cleanup() {
            sqlv(sqlite3_close(db));
        }

        void exec(string sql) {
            sqlv(sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL));
        }

        void addAccount(string name, MessageType type, string detailsJson) {
            //TODO: validate the JSON here first, or perhaps have this be slave to another method that performs JSON validation (latter seems better, something owned by accountManager)
            sqlite3_stmt *stmt;
            const char sql[] = "INSERT INTO " ACCOUNT_TABLE " VALUES(?,?,?);"; //Important this is an array and not a const char* so that sizeof() works properly
            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
            sqlv(sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), SQLITE_STATIC)); //Memory managed by containing string, so we tell SQLite it's static
            sqlv(sqlite3_bind_int(stmt, 2, type));
            sqlv(sqlite3_bind_text(stmt, 3, detailsJson.c_str(), detailsJson.length(), SQLITE_STATIC));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_finalize(stmt));
            AccountManager::buildTablesStatic(name, type);
        }

        void removeAccount(string name, MessageType type) {
            sqlite3_stmt *stmt;
            //TODO: hardcoded column names. :(
            const char sql[] = "DELETE FROM " ACCOUNT_TABLE " WHERE name=? AND type=?;";
            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
            sqlv(sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int(stmt, 2, type));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_finalize(stmt));
            AccountManager::destroyTablesStatic(name, type);
        }

        void updateAccountDetails(string name, MessageType type, string detailsJson) {
            sqlite3_stmt *stmt;
            //TODO: more hardcoded column names :(
            const char sql[] = "UPDATE " ACCOUNT_TABLE " SET details_json=? WHERE name=? AND type=?;";
            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL))
            sqlv(sqlite3_bind_text(stmt, 1, detailsJson.c_str(), detailsJson.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_text(stmt, 2, name.c_str(), name.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int(stmt, 3, type));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_finalize(stmt));
        }


        void buildDatabase() {
            exec("CREATE TABLE IF NOT EXISTS " ACCOUNT_TABLE ACCOUNT_TABLE_SCHEMA ";");
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("CREATE TABLE IF NOT EXISTS " CONTACT_TABLE(i) CONTACT_TABLE_SCHEMA ";");
                exec("CREATE TABLE IF NOT EXISTS " ADDRESS_TABLE(i) ADDRESS_TABLE_SCHEMA(i) ";");
            }
            //We don't build per account here because there won't be any accounts right after we just built the database
        }

        //TODO: test next 3 methods (truncateTables, deindexTables, indexTables)
        void truncateDatabase() {
            /* http://www.sqlite.org/lang_delete.html
            * When the WHERE is omitted from a DELETE statement and the table being deleted has no triggers,
            * SQLite uses an optimization to erase the entire table content without having to visit each row of the table individually.
            */
            exec("DELETE FROM " ACCOUNT_TABLE);
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("DELETE FROM " CONTACT_TABLE(i) ";");
                exec("DELETE FROM " ADDRESS_TABLE(i) ";");
            }
            list<shared_ptr<AccountManager>> l = reader::accountList();
            for (list<shared_ptr<AccountManager>>::iterator it = l.begin(); it != l.end(); ++it) {
                (*it)->truncateTables();
            }
        }

        void indexDatabase() {
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("CREATE INDEX IF NOT EXISTS " ADDRESS_INDEX(i) " ON " ADDRESS_INDEX(i) ADDRESS_INDEX_SCHEMA ";");
            }
            list<shared_ptr<AccountManager>> l = reader::accountList();
            for (list<shared_ptr<AccountManager>>::iterator it = l.begin(); it != l.end(); ++it) {
                (*it)->indexTables();
            }
        }

        void deindexDatabase() {
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("DROP INDEX IF EXISTS " ADDRESS_INDEX(i) ";");
            }
            list<shared_ptr<AccountManager>> l = reader::accountList();
            for (list<shared_ptr<AccountManager>>::iterator it = l.begin(); it != l.end(); ++it) {
                (*it)->deindexTables();
            }
        }


    }

}