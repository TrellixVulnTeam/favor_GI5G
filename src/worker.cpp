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



#include "worker.h"
#include "reader.h"
#include "logger.h"

namespace favor {
    namespace worker {
        namespace {
            sqlite3 *db;
            bool transacting = false;

        }

        void initialize() {
            sqlv(sqlite3_open(DB_PATH_FULL, &db));
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
            exec("CREATE TABLE IF NOT EXISTS " CONTACT_TABLE CONTACT_TABLE_SCHEMA ";");
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("CREATE TABLE IF NOT EXISTS " ADDRESS_TABLE(i) ADDRESS_TABLE_SCHEMA ";");
            }
            //We don't build per account here because there won't be any accounts right after we just built the database
        }

        void dropDatabase() {
            //TODO: This method screws up our DB connections, and needs some work before it's safe ot use normally.
            logger::info(string("Removing ")+ DB_PATH_FULL);
            int res = remove(DB_PATH_FULL);
            if (res) logger::warning("Could not successfully delete database");
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
            exec("DELETE FROM " CONTACT_TABLE ";");
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("DELETE FROM " ADDRESS_TABLE(i) ";");
            }
            auto l = reader::accountList();
            for (auto it = l->begin(); it != l->end(); ++it) {
                (*it)->truncateTables();
                (*it)->destroy();
            }
        }

        long createContact(const string& displayName, MessageType type){
            string sql = "INSERT INTO " CONTACT_TABLE "(display_name) VALUES(?)";
            sqlite3_stmt* stmt;
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            sqlv(sqlite3_bind_text(stmt, 1, displayName.c_str(), displayName.length(), SQLITE_STATIC));
            sqlv(sqlite3_step(stmt));
            long contactId = sqlite3_last_insert_rowid(db);
            sqlv(sqlite3_finalize(stmt));
            reader::invalidateContactList();
            return contactId;
        }

        void createContactWithAddress(const string &address, MessageType type, const string &displayName){
            long contactId = createContact(displayName, type);

            string sql = "INSERT OR REPLACE INTO " ADDRESS_TABLE(type) " VALUES (?,?,?);";
            sqlite3_stmt* stmt;
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            sqlv(sqlite3_bind_text(stmt, 1, address.c_str(), address.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int(stmt, 2, 0));
            sqlv(sqlite3_bind_int64(stmt, 3, contactId));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_finalize(stmt));
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
            if (sqlite3_changes(db) == 0) logger::warning("Contact "+displayName+" created from address "+as_string(addr)+" which SQLite does not report finding.");
        }

        void saveAddress(const Address &a, sqlite3_stmt* stmt) {
            sqlv(sqlite3_bind_text(stmt, 1, a.addr.c_str(), a.addr.length(), SQLITE_STATIC));
            sqlv(sqlite3_bind_int(stmt, 2, a.count));
            if (a.contactId > -1) sqlv(sqlite3_bind_int(stmt, 3, a.contactId));
            sqlv(sqlite3_step(stmt));
            sqlv(sqlite3_reset(stmt));
            sqlv(sqlite3_clear_bindings(stmt));
            //I think in general we can just rebind without clearing bindings, but in this case we need at least #3 (id) to be NULL
            //so that we can leave it unbound when it's -1

        }

        void rewriteAddressTable(const list<Address>& newAddresses, const MessageType& type){
            beginTransaction();
            exec("DELETE FROM " ADDRESS_TABLE(type) ";");
            string sql = "INSERT INTO " ADDRESS_TABLE(type) " VALUES(?,?,?);";
            sqlite3_stmt* stmt;
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            for (auto it = newAddresses.begin(); it != newAddresses.end(); ++it){
                saveAddress(*it, stmt);
            }
            sqlv(sqlite3_finalize(stmt)); //Finalizing it here is just cleanup
            commitTransaction();
        }

        //Not that this should ever matter, but this will remove any addresses that exist in the database from countedAddresses
        void recomputeAddressTable(std::unordered_map<std::string, int>& countedAddresses, std::unordered_map<std::string, std::string>& addressNames, MessageType type){
            //TODO: we don't know what we're doing with names yet
            shared_ptr<list<Address>> addrList = reader::addresses(type);

            list<Address> addrOutList = list<Address>();

            for (auto it = addrList->begin(); it != addrList->end(); ++it){
                //Insert, get state and remove from countedaddrs if exists
                int count = it->count;
                if (countedAddresses.count(it->addr)){
                    //This is newer information, so we update the count and then remove what would be duplicate data later on
                    count = countedAddresses.at(it->addr);
                    countedAddresses.erase(it->addr);
                }
                addrOutList.push_back(Address(it->addr, count, it->contactId, type));
            }
            for (auto it = countedAddresses.begin(); it != countedAddresses.end(); it++) {
                addrOutList.push_back(Address(it->first, it->second, -1, type));
            }

            addrOutList.sort(compareAddress);

            auto itr = addrOutList.begin();
            int i;
            for (i = 0; i < MAX_ADDRESSES && itr != addrOutList.end(); ++i) ++itr;
            if (i == MAX_ADDRESSES) addrOutList.erase(itr, addrOutList.end()); //Erase anything above our max if there are enough elements to need to

            //TODO: eventually we can do some guessing about current contacts here with levenshtein distance on names

            rewriteAddressTable(addrOutList, type);
        }

        void indexDatabase() {
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("CREATE INDEX IF NOT EXISTS " ADDRESS_INDEX(i) " ON " ADDRESS_INDEX(i) ADDRESS_INDEX_SCHEMA ";");
            }
            auto l = reader::accountList();
            for (auto it = l->begin(); it != l->end(); ++it) {
                (*it)->indexTables();
            }
        }

        void deindexDatabase() {
            for (int i = 0; i < NUMBER_OF_TYPES; ++i) {
                exec("DROP INDEX IF EXISTS " ADDRESS_INDEX(i) ";");
            }
            auto l = reader::accountList();
            for (auto it = l->begin(); it != l->end(); ++it) {
                (*it)->deindexTables();
            }
        }



        /* --------------------------------------------------------------------------------
        Account manager methods from here down. Declared in this file because they require,
        or can be run more efficientl with, direct db access
         --------------------------------------------------------------------------------*/

        long AccountManager::saveHeldMessages() {
            //id INTEGER, address TEXT NOT NULL, date INTEGER NOT NULL, charcount INTEGER NOT NULL, media INTEGER NOT NULL, body TEXT
            long successes = 0;
            string sentSql = "INSERT INTO " SENT_TABLE_NAME " VALUES(?,?,?,?,?,?);";
            string receivedSql = "INSERT INTO " RECEIVED_TABLE_NAME " VALUES(?,?,?,?,?,?);";
            sqlite3_stmt* sentStmt;
            sqlite3_stmt* receivedStmt;
            sqlv(sqlite3_prepare_v2(db, sentSql.c_str(), sentSql.length(), &sentStmt, NULL));
            sqlv(sqlite3_prepare_v2(db, receivedSql.c_str(), receivedSql.length(), &receivedStmt, NULL));

            beginTransaction();
            for (int i = 0; i < heldMessages.size(); ++i) {
                if (heldMessages[i].sent){
                 if (saveMessage(heldMessages[i], sentStmt)) ++successes;
                }
                else{
                   if (saveMessage(heldMessages[i], receivedStmt)) ++successes;
                }
            }

            sqlv(sqlite3_finalize(sentStmt));
            sqlv(sqlite3_finalize(receivedStmt));
            commitTransaction();
            heldMessages.clear();
            return successes;
        }

        bool AccountManager::saveMessage(const Message& m, sqlite3_stmt* stmt) {
            //logger::info("Save messages "+as_string(m));
            sqlv(sqlite3_bind_int64(stmt, 1, m.id));
            sqlv(sqlite3_bind_text(stmt, 2, m.address.c_str(), m.address.length(), SQLITE_STATIC));
            if (!m.failure()){
                sqlv(sqlite3_bind_int64(stmt, 3, m.date));
                sqlv(sqlite3_bind_int64(stmt, 4, m.charCount));
                sqlv(sqlite3_bind_int(stmt, 5, m.media()));
                if (SAVE_BODY) sqlv(sqlite3_bind_text(stmt, 6, m.body().c_str(), m.body().length(), SQLITE_STATIC));
            }
            bool inserted = true;

            int result = sqlite3_step(stmt);
            if (result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE){
                int extErrCode = sqlite3_extended_errcode(db);
                if (extErrCode == SQLITE_CONSTRAINT_PRIMARYKEY){
                    inserted = false;
                    sqlite3_reset(stmt); //This is something we can handle, so we don't want to validate the reset because it'll return the same error as above
                } else sqlv(sqlite3_reset(stmt)); //This reset should return the same code as the previous result anyway
            } else {
                sqlv(sqlite3_reset(stmt));
            }
            return inserted;
        }


        void AccountManager::saveJson() {
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