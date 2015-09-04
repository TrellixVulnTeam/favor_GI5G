/*Copyright (C) 2015  Joshua Tanner (mindful.jt@gmail.com)

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



#include "skypemanager.h"

namespace favor{

    SkypeManager::SkypeManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_SKYPE, detailsJson) {
        consultJson(true);
    }


    void SkypeManager::updateJson() {
        setJsonLong(lastFetchTime);
        rapidjson::Value addrsVal;
        addrsVal.SetArray();
        for (auto it = managedAddresses.begin(); it != managedAddresses.end(); ++it){
            addrsVal.PushBack(rapidjson::Value(it->c_str(), json.GetAllocator()).Move(), json.GetAllocator());
        }
        json[addrListName] = addrsVal;
    }


    const char* SkypeManager::addrListName  = "managedAddresses";
    #define SKYPE_MSG_TABLE_NAME "Messages"
    #define SKYPE_ACCOUNTS_TABLE_NAME "Accounts"
    #define SKYPE_PARTICIPANTS_TABLE_NAME "Participants"
    #define SKYPE_TRANSFERS_TABLE_NAME "Tranfers"


    #define SKYPE_PARTICIPANTS_COLUMN_ACCNAME "identity"
    #define SKYPE_PARTICIPANTS_COLUMN_CONVO "convo_id"

    #define SKYPE_ACCOUNTS_COLUMN_ACCNAME "skypename"

    #define SKYPE_MSG_COLUMN_BODY "body_xml"
    #define SKYPE_MSG_COLUMN_AUTHOR "author"
    #define SKYPE_MSG_COLUMN_AUTHOR_DISPLAYNAME "from_dispname" //TODO: do we need this? we can get the info from other tables, and might use those for contacts anyway
    #define SKYPE_MSG_COLUMN_DATE "timestamp"
    #define SKYPE_MSG_COLUMN_CONVO "convo_id"
    #define SKYPE_MSG_COLUMN_RMID "remote_id"

    #define SKYPE_TRANSFERS_COLUMN_AUTHOR "partner_id"
    #define SKYPE_TRANSFERS_COLUMN_CONVO "convo_id"
    #define SKYPE_TRANSFERS_COLUMN_STATUS "status"
    #define SKYPE_TRANSFERS_STATUS_SUCCESS "8" //This just has to be inferred from looking at the database

    void SkypeManager::verifyDatabaseContents() {

        std::unordered_map<string, bool> columnCheck;
        #define SKYPE_CHECK_COLUMN(table, col, colname) if (!columnCheck.count(col)) throw badUserDataException("Skype database missing " table " table " colname " column (" col ");")

        sqlite3 *db;
        sqlite3_stmt* stmt;
        int result;
        sqlv(sqlite3_open_v2(skypeDatabaseLocation.c_str(), &db, SQLITE_OPEN_READONLY, NULL));
        //Look for tables, and maybe verify presence of our desired attributes?

        //Messages table ------------------------
        string sql("PRAGMA table_info(" SKYPE_MSG_TABLE_NAME ");");
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            columnCheck[reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))] = true;
        }
        sqlv(result);
        sqlite3_finalize(stmt);
        if (columnCheck.size() == 0) throw badUserDataException("Skype database missing messages table (" SKYPE_MSG_TABLE_NAME")");
        else {
            SKYPE_CHECK_COLUMN("messages", SKYPE_MSG_COLUMN_BODY, "body");
            SKYPE_CHECK_COLUMN("messages", SKYPE_MSG_COLUMN_AUTHOR, "author");
            SKYPE_CHECK_COLUMN("messages", SKYPE_MSG_COLUMN_AUTHOR_DISPLAYNAME, "author display name");
            SKYPE_CHECK_COLUMN("messages", SKYPE_MSG_COLUMN_DATE, "date");
            SKYPE_CHECK_COLUMN("messages", SKYPE_MSG_COLUMN_CONVO, "conversation id");
            columnCheck.clear();
        }

        //Accounts table ------------------------
        sql = ("PRAGMA table_info(" SKYPE_ACCOUNTS_TABLE_NAME ");");
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            columnCheck[reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))] = true;
        }
        sqlv(result);
        sqlite3_finalize(stmt);
        if (columnCheck.size() == 0) throw badUserDataException("Skype database missing accounts table (" SKYPE_ACCOUNTS_TABLE_NAME")");
        else {
            SKYPE_CHECK_COLUMN("accounts", SKYPE_ACCOUNTS_COLUMN_ACCNAME, "account name");
            columnCheck.clear();
        }

        //Conversation participants table ------------------------
        sql = ("PRAGMA table_info(" SKYPE_PARTICIPANTS_TABLE_NAME ");");
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            columnCheck[reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))] = true;
        }
        sqlv(result);
        sqlite3_finalize(stmt);
        if (columnCheck.size() == 0) throw badUserDataException("Skype database missing accounts table (" SKYPE_PARTICIPANTS_TABLE_NAME")");
        else {
            SKYPE_CHECK_COLUMN("conversation participants", SKYPE_PARTICIPANTS_COLUMN_ACCNAME, "account name");
            SKYPE_CHECK_COLUMN("conversation participants", SKYPE_PARTICIPANTS_COLUMN_CONVO, "conversation id");
            columnCheck.clear();
        }

        //Transfers table
        sql = ("PRAGMA table_info(" SKYPE_TRANSFERS_TABLE_NAME ");");
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            columnCheck[reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))] = true;
        }
        sqlv(result);
        sqlite3_finalize(stmt);
        if (columnCheck.size() == 0) throw badUserDataException("Skype database missing accounts table (" SKYPE_PARTICIPANTS_TABLE_NAME")");
        else {
            SKYPE_CHECK_COLUMN("file transfers", SKYPE_TRANSFERS_COLUMN_AUTHOR, "author");
            SKYPE_CHECK_COLUMN("file transfers", SKYPE_TRANSFERS_COLUMN_CONVO, "conversation id");
            SKYPE_CHECK_COLUMN("file transfers", SKYPE_TRANSFERS_COLUMN_STATUS, "status");
            columnCheck.clear();
        }




        //Verify that we actually have the right account name
        sql = ("SELECT " SKYPE_ACCOUNTS_COLUMN_ACCNAME " FROM " SKYPE_ACCOUNTS_TABLE_NAME";");
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

        bool found = false;
        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            if (sqlite3_build_string(sqlite3_column_text(stmt, 0)) == accountName) found = true;
        }
        sqlv(result);
        if (!found) throw badUserDataException("Skype manager account name not found in database");

        sqlv(sqlite3_finalize(stmt));
        sqlv(sqlite3_close(db));
    }

    void SkypeManager::consultJson(bool initial) {
        if (initial) {
            if (json.HasMember("skypeDatabaseLocation")){
                skypeDatabaseLocation = json["skypeDatabaseLocation"].GetString();
                verifyDatabaseContents();
            }
            else throw badUserDataException("Skype manager missing database location");
        }

        if (json.HasMember(addrListName)){
            rapidjson::Value& addrsVal = json[addrListName];
            if (!addrsVal.IsArray()) throw badUserDataException("Managed addresses list improperly formatted in "+accountName +" json");
            else {
                for (auto it = addrsVal.Begin(); it!= addrsVal.End(); ++it){
                    //TODO: do skype addresses need to be validated somehow?
                    managedAddresses.insert(it->GetString());
                }
            }
        }
        else {
            rapidjson::Value addrsVal;
            addrsVal.SetArray();
            json.AddMember(rapidjson::Value(addrListName, json.GetAllocator()).Move(), addrsVal, json.GetAllocator());
        }

        getJsonLong(lastFetchTime, 0);
    }


    void SkypeManager::fetchAddresses() {
        //TODO: we can use Skype's pretty names to guess suggested display names the same way we do with the email manager

    }

    void SkypeManager::fetchMessages() {

        //TODO: minimal HTML (name suggests just XML?) shows up here too; we're going to need stripping



        sqlite3 *db;
        sqlite3_stmt* stmt;
        DLOG(skypeDatabaseLocation);
        sqlv(sqlite3_open_v2(skypeDatabaseLocation.c_str(), &db, SQLITE_OPEN_READONLY, NULL));
        int result;
        string sql("SELECT " SKYPE_PARTICIPANTS_COLUMN_ACCNAME "," SKYPE_PARTICIPANTS_COLUMN_CONVO " FROM " SKYPE_PARTICIPANTS_TABLE_NAME ";");
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
        std::unordered_map<int, vector<string>> conversationIDMap;
        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            //Build a map of convo id to name so we can use it for messages later. Every convo includes at least two participants:
            //the account owner, plus the person (people) they are talking to.
            string participant = sqlite3_build_string(sqlite3_column_text(stmt, 0));
            if (participant != accountName) conversationIDMap[sqlite3_column_int(stmt, 1)].push_back(participant);
        }
        sqlv(result);
        sqlv(sqlite3_finalize(stmt));





        //Normal messages
        //TODO: include our lastFetchTime here as a limiter, and eventually go back and get newly added accounts
        sql = "SELECT " SKYPE_MSG_COLUMN_RMID "," SKYPE_MSG_COLUMN_AUTHOR "," SKYPE_MSG_COLUMN_DATE "," SKYPE_MSG_COLUMN_BODY "," SKYPE_MSG_COLUMN_CONVO
                " FROM " SKYPE_MSG_TABLE_NAME " ORDER BY " SKYPE_MSG_COLUMN_DATE " " DB_SORT_ORDER";";
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            long rmid = sqlite3_column_int64(stmt, 0);
            long timestamp =  sqlite3_column_int64(stmt, 2); //TODO: is this fine? proper unix timestamp?
            //TODO: and if it is, compare it to lastfetch and select the larger
            string body = sqlite3_build_string(sqlite3_column_text(stmt, 3));
            long convId = sqlite3_column_int64(stmt,4);

            string author = sqlite3_build_string(sqlite3_column_text(stmt, 1));
            bool sent = author == accountName;
            if (sent){
                for (int i = 0; i < conversationIDMap[convId].size(); ++i){
                    holdMessage(sent, rmid, timestamp, conversationIDMap[convId][i], true, body);
                }
            } else {
                string address = author;
                holdMessage(sent, rmid, timestamp, address, false, body);
            }

        }
        sqlv(result); //make sure we broke out with good results
        sqlv(sqlite3_finalize(stmt));

        //File transfers (media-only messages, as far as Favor is concerned)
        sql = "SELECT " SKYPE_TRANSFERS_COLUMN_AUTHOR "," SKYPE_TRANSFERS_COLUMN_CONVO "," SKYPE_TRANSFERS_COLUMN_STATUS " FROM " SKYPE_TRANSFERS_TABLE_NAME
                " WHERE " SKYPE_TRANSFERS_COLUMN_STATUS "=" SKYPE_TRANSFERS_STATUS_SUCCESS ";";
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {

            //TODO:
        }
        sqlv(result); //make sure we broke out with good results
        sqlv(sqlite3_finalize(stmt));


        sqlv(sqlite3_close(db));


    }


}