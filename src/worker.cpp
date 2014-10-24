#include "worker.h"
#include "reader.h"
#include "logger.h"

namespace favor {
    namespace worker {
        namespace {
            sqlite3 *db;
            bool transacting = false;

            long createContact(const string& displayName, MessageType type){
                string sql = "INSERT INTO " CONTACT_TABLE(type) "(display_name) VALUES(?)";
                sqlite3_stmt* stmt;
                sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
                sqlv(sqlite3_bind_text(stmt, 1, displayName.c_str(), displayName.length(), SQLITE_STATIC));
                sqlv(sqlite3_step(stmt));
                long contactId = sqlite3_last_insert_rowid(db);
                sqlv(sqlite3_finalize(stmt));
                return contactId;
            }
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

        void beginTransaction() {
            if (transacting) throw sqliteException("Cannot begin transaction while transacting");
            exec("BEGIN IMMEDIATE TRANSACTION;");
            transacting = true;
        }

        void commitTransaction() {
            if (!transacting) throw sqliteException("Cannot end transaction while not transacting");
            exec("COMMIT TRANSACTION;");
            transacting = false;
        }

        void rollbackTransaction() {
            if (!transacting) throw sqliteException("Cannot roll back transaction while not transacting");
            exec("ROLLBACK TRANSACTION;");
            transacting = false;
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
            list<AccountManager*>* l = reader::accountList();
            for (list<AccountManager*>::iterator it = l->begin(); it != l->end(); ++it) {
                (*it)->truncateTables();
            }
        }

        //TODO: test next 2 methods for creating contacts
        void createContactWithAddress(const string &address, MessageType type, const string &displayName){
            long contactId = createContact(displayName, type);

            string sql = "INSERT INTO " ADDRESS_TABLE(type) " VALUES (?,?,?);";
            sqlite3_stmt* stmt;
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            sqlv(sqlite3_bind_text(stmt, 1, address.c_str(), address.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int(stmt, 2, 0));
            sqlv(sqlite3_bind_int64(stmt, 3, contactId));
        }

        void createContactFromAddress(const Address& addr, const string& displayName){
            long contactId = createContact(displayName, addr.type);

            string sql = "UPDATE " ADDRESS_TABLE(addr.type) " SET contact_id=? WHERE address=?";
            sqlite3_stmt* stmt;
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            sqlv(sqlite3_bind_int64(stmt, 1, contactId));
            sqlv(sqlite3_bind_text(stmt, 2, addr.addr.c_str(), addr.addr.length(), SQLITE_STATIC));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_finalize(stmt));
        }

        void indexDatabase() {
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("CREATE INDEX IF NOT EXISTS " ADDRESS_INDEX(i) " ON " ADDRESS_INDEX(i) ADDRESS_INDEX_SCHEMA ";");
            }
            list<AccountManager*>* l = reader::accountList();
            for (list<AccountManager*>::iterator it = l->begin(); it != l->end(); ++it) {
                (*it)->indexTables();
            }
        }

        void deindexDatabase() {
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("DROP INDEX IF EXISTS " ADDRESS_INDEX(i) ";");
            }
            list<AccountManager*>* l = reader::accountList();
            for (list<AccountManager*>::iterator it = l->begin(); it != l->end(); ++it) {
                (*it)->deindexTables();
            }
        }

        /* --------------------------------------------------------------------------------
        Account manager methods from here down. Declared in this file because they require
        direct db access
         --------------------------------------------------------------------------------*/

        void AccountManager::saveHeldMessages() {
            //id INTEGER, address TEXT NOT NULL, date INTEGER NOT NULL, charcount INTEGER NOT NULL, media INTEGER NOT NULL
            string sentSql = "INSERT INTO " SENT_TABLE_NAME " VALUES(?,?,?,?,?)";
            string receivedSql = "INSERT INTO " RECEIVED_TABLE_NAME " VALUES(?,?,?,?,?)";
            sqlite3_stmt* sentStmt;
            sqlite3_stmt* receivedStmt;
            sqlv(sqlite3_prepare(db, sentSql.c_str(), sentSql.length(), &sentStmt, NULL));
            sqlv(sqlite3_prepare(db, receivedSql.c_str(), receivedSql.length(), &receivedStmt, NULL));

            beginTransaction();
            for (int i = 0; i < heldMessages->size(); ++i) {
                if ((*heldMessages)[i].sent) saveMessage((*heldMessages)[i], sentStmt);
                else saveMessage((*heldMessages)[i], receivedStmt);
            }
            sqlv(sqlite3_finalize(sentStmt));
            sqlv(sqlite3_finalize(receivedStmt));
            commitTransaction();
            heldMessages->clear();
        }

        void AccountManager::saveMessage(const Message& m, sqlite3_stmt* stmt) {
            sqlv(sqlite3_bind_int64(stmt, 1, m.id));
            sqlv(sqlite3_bind_text(stmt, 2, m.address.c_str(), m.address.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int64(stmt, 3, m.date));
            sqlv(sqlite3_bind_int64(stmt, 4, m.charCount));
            sqlv(sqlite3_bind_int(stmt, 5, m.media));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_reset(stmt));
        }

        void AccountManager::saveAddress(const Address &a, sqlite3_stmt* stmt) {
            sqlv(sqlite3_bind_text(stmt, 1, a.addr.c_str(), a.addr.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int(stmt, 2, a.count));
            if (a.contactId > -1) sqlv(sqlite3_bind_int(stmt, 3, a.contactId));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_reset(stmt));
            sqlv(sqlite3_clear_bindings(stmt));
            //I think we can just rebind without clearing bindings, but in this case we need at least #3 (id) to be NULL
            //so we can leave it unbound when it's -1

        }

        void AccountManager::saveHeldAddresses() {
            shared_ptr<list<Address>> addresses = reader::addresses(type);

            for (auto it = countedAddresses.begin(); it != countedAddresses.end(); it++) {
                addresses->push_back(Address (it->first, it->second, -1, type)); //Because currently unbound to any contact
            }

            addresses->sort(); //TODO: this actually needs to be sorted backward, also make sure sorting works with contact's overloaded operators

            auto itr = addresses->begin();
            int i;
            for (i = 0; i < MAX_ADDRESSES && itr != addresses->end(); ++i) ++itr;
            if (i == MAX_ADDRESSES) addresses->erase(itr, addresses->end()); //Erase anything above our max if there are enough elements to need to

            //TODO: eventually we can do some guessing about current contacts here with levenshtein distance on names

            countedAddresses.clear();
            addressNames.clear();

            beginTransaction();
            exec("DELETE FROM " ADDRESS_TABLE(type) ";");
            string sql = "INSERT INTO " ADDRESS_TABLE(type) " VALUES(?,?,?);";
            sqlite3_stmt* stmt;
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            for (auto it = addresses->begin(); it != addresses->end(); ++it){
                saveAddress(*it, stmt);
            }
            sqlv(sqlite3_finalize(stmt)); //Finalizing it here is just cleanup
            commitTransaction();
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