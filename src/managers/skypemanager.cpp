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

//TODO: minimal HTML (name suggests just XML?) shows up here too; we're going to need stripping
//TODO: we can use Skype's pretty names to guess suggested display names the same way we do with the email manager

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
    #define MSG_TABLE_NAME "Messages"
    #define MSG_COLUMN_BODY "body_xml"
    #define MSG_COLUMN_AUTHOR "author"
    #define MSG_COLUMN_AUTHOR_DISPLAYNAME "from_dispname"
    #define MSG_COLUMN_DATE "timestamp"


    void SkypeManager::verifyDatabase() {
        //TODO: this should throw an exception if we can't both open the database and find the right table(s) we need
        sqlite3 *db;
        int r = sqlite3_open_v2(skypeDatabaseLocation.c_str(), &db, SQLITE_OPEN_READONLY, NULL);
        DLOG(as_string(r));
        sqlv(sqlite3_close(db));
    }

    void SkypeManager::consultJson(bool initial) {
        if (initial) {
            if (json.HasMember("skypeDatabaseLocation"))
                skypeDatabaseLocation = json["skypeDatabaseLocation"].GetString();
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
        //TODO

    }

    void SkypeManager::fetchMessages() {

        //TODO: figure out a way to verify we have the right database file; the wrong one simply gives us "Table does not exist" which is tremendously uninformative
        sqlite3 *db;
        sqlite3_stmt* stmt;
        DLOG(skypeDatabaseLocation);
        sqlv(sqlite3_open_v2(skypeDatabaseLocation.c_str(), &db, SQLITE_OPEN_READONLY, NULL));
        //TODO: include our lastFetchTime here as a limiter, and eventually go back and get newly added accounts
        string sql("SELECT " MSG_COLUMN_AUTHOR "," MSG_COLUMN_DATE "," MSG_COLUMN_BODY " FROM " MSG_TABLE_NAME " ORDER BY " MSG_COLUMN_DATE " " DB_SORT_ORDER";");
        DLOG(sql);
        DLOG(skypeDatabaseLocation);
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
        int result;
        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            string body = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));

        }
        sqlv(sqlite3_finalize(stmt));
        sqlv(sqlite3_close(db));


    }


}