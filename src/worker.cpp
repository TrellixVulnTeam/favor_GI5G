#include "worker.h"
#include "reader.h"
#include "logger.h"

namespace favor {
    namespace worker {
        namespace {
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
            list<AccountManager*> l = reader::accountList();
            for (list<AccountManager*>::iterator it = l.begin(); it != l.end(); ++it) {
                (*it)->truncateTables();
            }
        }

        void indexDatabase() {
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("CREATE INDEX IF NOT EXISTS " ADDRESS_INDEX(i) " ON " ADDRESS_INDEX(i) ADDRESS_INDEX_SCHEMA ";");
            }
            list<AccountManager*> l = reader::accountList();
            for (list<AccountManager*>::iterator it = l.begin(); it != l.end(); ++it) {
                (*it)->indexTables();
            }
        }

        void deindexDatabase() {
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("DROP INDEX IF EXISTS " ADDRESS_INDEX(i) ";");
            }
            list<AccountManager*> l = reader::accountList();
            for (list<AccountManager*>::iterator it = l.begin(); it != l.end(); ++it) {
                (*it)->deindexTables();
            }
        }

        /* --------------------------------------------------------------------------------
        Account manager methods from here down. Declared in this file because they require
        direct db access
         --------------------------------------------------------------------------------*/

        void AccountManager::saveMessage(const message* m){
            //id INTEGER, address TEXT NOT NULL, date INTEGER NOT NULL, charcount INTEGER NOT NULL, media INTEGER NOT NULL
            string sql = "INSERT INTO " + (m->sent ? SENT_TABLE_NAME : RECEIVED_TABLE_NAME) + " VALUES(?,?,?,?,?)";
            sqlite3_stmt* stmt;
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            sqlv(sqlite3_bind_int64(stmt, 1, m->id));
            sqlv(sqlite3_bind_text(stmt, 2, m->address.c_str(), m->address.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int64(stmt, 3, m->date));
            sqlv(sqlite3_bind_int64(stmt, 4, m->charCount));
            sqlv(sqlite3_bind_int(stmt, 5, m->media));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_finalize(stmt));
        }

        void AccountManager::saveHeldContacts(){
            list<contact> contactResultList;
            for (std::unordered_map<string, int>::const_iterator it = countedContacts.begin(); it != countedContacts.end(); it++) {
                contactResultList.push_back(contact(it->first, contactNames[it->first], it->second));
            }

            contactResultList.sort(); //TODO: make sure sorting works with contact's overloaded operators
            //TODO: figure out what contacts we don't need and save the ones we do. will definitely require hitting the DB

            for (list<contact>::const_iterator it = contactResultList.begin(); it != contactResultList.end(); it++) {
                logger::info("Addr: "+it->address+" SuggName: "+it->suggestedName+" Count: "+as_string(it->count));
            }
        }

        void AccountManager::saveFetchData() {
            string detailsJson = as_string(json);
            sqlite3_stmt *stmt;
            const char sql[] = "UPDATE " ACCOUNT_TABLE " SET details_json=? WHERE name=? AND type=?;";
            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
            sqlv(sqlite3_bind_text(stmt, 1, detailsJson.c_str(), detailsJson.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_text(stmt, 2, accountName.c_str(), accountName.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int(stmt, 3, type));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_finalize(stmt));
        }

        AccountManager* AccountManager::addAccount(string name, MessageType type, string detailsJson) {
            AccountManager* account = buildManager(name, type, detailsJson);
            //Creating the manager first will validate the JSON for us before we go to save it
            sqlite3_stmt *stmt;
            const char sql[] = "INSERT INTO " ACCOUNT_TABLE " VALUES(?,?,?);"; //Important this is an array and not a const char* so that sizeof() works properly
            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
            sqlv(sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), SQLITE_STATIC)); //Memory managed by containing string, so we tell SQLite it's static
            sqlv(sqlite3_bind_int(stmt, 2, type));
            sqlv(sqlite3_bind_text(stmt, 3, detailsJson.c_str(), detailsJson.length(), SQLITE_STATIC));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_finalize(stmt));
            account->buildTables();
            reader::addAccount(account);
            return account;
        }

        void AccountManager::destroy() {
            sqlite3_stmt *stmt;
            const char sql[] = "DELETE FROM " ACCOUNT_TABLE " WHERE name=? AND type=?;";
            sqlv(sqlite3_prepare_v2(db, sql, sizeof(sql), &stmt, NULL));
            sqlv(sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int(stmt, 2, type));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_finalize(stmt));
            destroyTables();
            reader::removeAccount(this); //Removing it from the reader list also deletes the object
        }




    }

}