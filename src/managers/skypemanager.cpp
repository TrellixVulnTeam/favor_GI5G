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
#include <sstream>

namespace favor{

    SkypeManager::SkypeManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_SKYPE, detailsJson) {
        consultJson(true);
    }


    void SkypeManager::updateJson() {
        setJsonLong(lastMessageTime);
        setJsonLong(lastTransferTime);
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
    #define SKYPE_TRANSFERS_TABLE_NAME "Transfers"

    #define SKYPE_PHONE_REGEXP "'^\\+\\d+$'"


    #define SKYPE_PARTICIPANTS_COLUMN_ACCNAME "identity"
    #define SKYPE_PARTICIPANTS_COLUMN_CONVO "convo_id"

    #define SKYPE_ACCOUNTS_COLUMN_ACCNAME "skypename"

    #define SKYPE_MSG_COLUMN_BODY "body_xml"
    #define SKYPE_MSG_COLUMN_AUTHOR "author"
    #define SKYPE_MSG_COLUMN_AUTHOR_DISPLAYNAME "from_dispname" //TODO: do we need this? we can get the info from other tables, and might use those for contacts anyway
    #define SKYPE_MSG_COLUMN_DATE "timestamp"
    #define SKYPE_MSG_COLUMN_CONVO "convo_id"
    #define SKYPE_MSG_COLUMN_RMID "remote_id"

    #define SKYPE_TRANSFERS_COLUMN_AUTHOR "partner_handle"
    #define SKYPE_TRANSFERS_COLUMN_CONVO "convo_id"
    #define SKYPE_TRANSFERS_COLUMN_STATUS "status"
    #define SKYPE_TRANSFERS_STATUS_SUCCESS "8" //This just has to be inferred from looking at the database
    #define SKYPE_TRANSFERS_COLUMN_DATE "starttime"
    #define SKYPE_TRANSFERS_COLUMN_PKID "pk_id" //Wish I knew what "PK" stood for, but this appears to be unique

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
        if (columnCheck.size() == 0) throw badUserDataException("Skype database missing conversation participants table (" SKYPE_PARTICIPANTS_TABLE_NAME")");
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
        if (columnCheck.size() == 0) throw badUserDataException("Skype database missing transfers table (" SKYPE_TRANSFERS_TABLE_NAME")");
        else {
            SKYPE_CHECK_COLUMN("file transfers", SKYPE_TRANSFERS_COLUMN_AUTHOR, "author");
            SKYPE_CHECK_COLUMN("file transfers", SKYPE_TRANSFERS_COLUMN_CONVO, "conversation id");
            SKYPE_CHECK_COLUMN("file transfers", SKYPE_TRANSFERS_COLUMN_STATUS, "status");
            SKYPE_CHECK_COLUMN("file transfers", SKYPE_TRANSFERS_COLUMN_DATE, "start time");
            SKYPE_CHECK_COLUMN("file transfers", SKYPE_TRANSFERS_COLUMN_PKID, "pk id");
            columnCheck.clear();
        }




        //Verify that we actually have the right account name
        sql = ("SELECT " SKYPE_ACCOUNTS_COLUMN_ACCNAME " FROM " SKYPE_ACCOUNTS_TABLE_NAME";");
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));

        bool found = false;
        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            if (sqlite3_get_string(stmt, 0) == accountName) found = true;
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

        getJsonLong(lastMessageTime, 0);
        getJsonLong(lastTransferTime, 0);
    }

    string SkypeManager::buildSelection(const vector<Address> &addresses, const std::set<string>& badAddresses, const std::unordered_map<string, vector<long>>& participantIds,
                                        const string convoIDColumn, const string timeColumn, bool catchUp = false) {
        string selection = "WHERE (";
        int totalCount = 0;
        for (int i = 0; i < addresses.size(); ++i){
            if (!badAddresses.count(addresses[i].addr)){
                for (int j = 0; j < participantIds.at(addresses[i].addr).size(); ++j) ++totalCount;
            }
        }
        for (int i = 0; i < totalCount; ++i){
                selection += convoIDColumn +"=?";
                if (i != totalCount -1 ) selection += " OR ";
                else selection += ")";
        }
        if (!catchUp) selection += " AND "+timeColumn+">?";
        else selection += " AND "+timeColumn+"<=?";
        return selection;

    }

    void SkypeManager::bindSelection(sqlite3_stmt *stmt, const vector<Address>& addresses, const std::set<string>& badAddresses,
                                     const std::unordered_map<string, vector<long>>& participantIds, long time) {
        //The bindings here aren't validated which isn't ideal, but we'd have to also take the DB as an argument for that
        int usedAddressTotal = 1; //We start from 1 because bindings start from 1
        for (int i=0; i < addresses.size(); ++i){
            if (!badAddresses.count(addresses[i].addr)){
                const vector<long>& ids = participantIds.at(addresses[i].addr);
                for (int j = 0; j < ids.size(); ++j){
                    DLOG("Bind "+as_string(usedAddressTotal) +" to convo id "+as_string(ids[j]));
                    sqlite3_bind_int64(stmt, usedAddressTotal, ids[j]);
                    ++usedAddressTotal;
                }
            }
        }
        DLOG("Bind "+as_string(usedAddressTotal)+" to timestamp "+as_string(time));
        sqlite3_bind_int64(stmt, usedAddressTotal, time);
    }


    void SkypeManager::fetchAddresses() {
        //TODO: we can use Skype's pretty names to guess suggested display names the same way we do with the email manager

        //Go through the participants table, pick up every participant who isn't a phone number and their convo IDs (for now; not sure we want to deal with numbers)
        //go to the messages table and run a count looking for all messages that correspond to that participant's ID(s)
        //we now have a number of messages corresponding to each participant and can organize by that

        //TODO: finally, to get the suggested pretty name, run a query in the messages table by (a) convo ID for each participant, excluding messages where
        //the "author" is our account. Then, we can use the display name on the message to get a pretty name for that contact


        std::unordered_map<string, vector<long>> participantToIDMap;

        sqlite3 *db;
        sqlite3_stmt* stmt;
        int result;
        sqlv(sqlite3_open_v2(skypeDatabaseLocation.c_str(), &db, SQLITE_OPEN_READONLY, NULL));
        sqlite3_bind_regexp_function(db);


        //Get our participants
        string sql("SELECT " SKYPE_PARTICIPANTS_COLUMN_ACCNAME "," SKYPE_PARTICIPANTS_COLUMN_CONVO " FROM " SKYPE_PARTICIPANTS_TABLE_NAME
        " WHERE (" SKYPE_PARTICIPANTS_COLUMN_ACCNAME "!=?) AND (NOT " SKYPE_PARTICIPANTS_COLUMN_ACCNAME " REGEXP " SKYPE_PHONE_REGEXP ");" );

        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
        sqlv(sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.length(), SQLITE_STATIC));



        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            string participant = sqlite3_get_string(stmt, 0);
            long convoID = sqlite3_column_int64(stmt, 1);

            DLOG(participant + " : "+ as_string(convoID));

            participantToIDMap[participant].push_back(convoID);

        }
        sqlv(result); //make sure we broke out with good results
        sqlv(sqlite3_finalize(stmt));



        //Count messages from each participant
        for (auto it = participantToIDMap.begin(); it != participantToIDMap.end(); ++it){
            string sqlSubstmt("SELECT COUNT(*) FROM " SKYPE_MSG_TABLE_NAME " WHERE " SKYPE_MSG_COLUMN_AUTHOR "=? AND (");

            vector<long>& participantIDs = it->second;
            for (int i = 0; i < participantIDs.size(); ++i){
                sqlSubstmt += SKYPE_MSG_COLUMN_CONVO "=?";
                if (i < participantIDs.size() -1 ) sqlSubstmt += " OR ";
                else sqlSubstmt += ");";
            }


            DLOG(sqlSubstmt)
            sqlv(sqlite3_prepare_v2(db, sqlSubstmt.c_str(), sqlSubstmt.length(), &stmt, NULL));

            sqlv(sqlite3_bind_text(stmt, 1, accountName.c_str(), accountName.length(), SQLITE_STATIC));

            for (int i = 0; i < participantIDs.size(); ++i){
                //sqlite bindings start at 1, so add 1 for binding value and 1 because we've already bound a parameter for author column
                sqlite3_bind_int64(stmt, i+2, participantIDs[i]);
            }

            sqlv(sqlite3_step(stmt));
            long sum = sqlite3_column_int64(stmt, 0);
            setAddressCount(it->first, sum);
            DLOG("sent msg sum for "+it->first+" : "+as_string(sum));
            sqlv(sqlite3_finalize(stmt));
        }


        //Get participant suggested names
        sql = "SELECT " SKYPE_MSG_COLUMN_AUTHOR_DISPLAYNAME "," SKYPE_MSG_COLUMN_CONVO "," SKYPE_MSG_COLUMN_AUTHOR " FROM "
                SKYPE_MSG_TABLE_NAME " WHERE " SKYPE_MSG_COLUMN_CONVO "=? AND " SKYPE_MSG_COLUMN_AUTHOR "!=? LIMIT 1";
        for (auto it = participantToIDMap.begin(); it != participantToIDMap.end(); ++it){
            sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
            sqlv(sqlite3_bind_int64(stmt, 1, it->second[0])); //This only uses the first convo_id, but any of them should work, and there will always be at least 1
            sqlv(sqlite3_bind_text(stmt, 2, accountName.c_str(), accountName.length(), SQLITE_STATIC));
            int result = sqlite3_step(stmt);
            if (result == SQLITE_ROW){
                string displayName = sqlite3_get_string(stmt, 0);
                string accountName = it->first;
                DLOG("Author: "+accountName +" Display Name: "+displayName);
            } else if (result == SQLITE_DONE){
                DLOG("No name found for "+it->first);
            } else sqlv(result);

            sqlv(sqlite3_finalize(stmt));
        }


        sqlv(sqlite3_close(db));


    }

    string SkypeManager::processBody(string body) {
        //Even if we're removing XML, ironically, the whole thing has to be in a <body> element to be valid
        std::stringstream ss("<body>"+body+"</body>");
        pugi::xml_document doc;
        pugi::xml_parse_result res = doc.load(ss);
        if (res) return stripXML(doc);
        else throw badMessageDataException("Unable to process Skype message body XML");
    }


    void SkypeManager::fetchFromMessagesTable(sqlite3 *db,
                                              const shared_ptr<vector<Address>> addresses,
                                              const std::set<string>& badAddressIDs,
                                              const std::unordered_map<long, vector<string>>& conversationIDToParticipantMap,
                                              const std::unordered_map<string, vector<long>>& participantToIDMap,
                                              bool catchUp = false){
        sqlite3_stmt* stmt;
        int result;

        if (catchUp && lastMessageTime == 0) return; //No need to run specific catch-ups for new addresses on first fetch


        string sql("SELECT " SKYPE_MSG_COLUMN_RMID "," SKYPE_MSG_COLUMN_AUTHOR "," SKYPE_MSG_COLUMN_DATE "," SKYPE_MSG_COLUMN_BODY "," SKYPE_MSG_COLUMN_CONVO
                " FROM " SKYPE_MSG_TABLE_NAME " ");
        sql += buildSelection((*addresses), badAddressIDs, participantToIDMap, SKYPE_MSG_COLUMN_CONVO, SKYPE_MSG_COLUMN_DATE, catchUp);
        sql += " ORDER BY " SKYPE_MSG_COLUMN_DATE " " DB_SORT_ORDER;
        DLOG(sql);
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
        bindSelection(stmt, (*addresses), badAddressIDs, participantToIDMap, lastMessageTime);

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            long rmid = sqlite3_column_int64(stmt, 0);
            long timestamp =  sqlite3_column_int64(stmt, 2);
            if (timestamp > lastMessageTime) lastMessageTime = timestamp;
            string body = sqlite3_get_string(stmt, 3);
            long convId = sqlite3_column_int64(stmt,4);

            bool failure = false;
            try {
                body = processBody(body);
            } catch (badMessageDataException& e){
                logger::warning("Failed to parse XML in body of Skype message with ID "+as_string(rmid));
                failure = true;
            }

            string author = sqlite3_get_string(stmt, 1);
            bool sent = author == accountName;
            if (sent){
                for (int i = 0; i < conversationIDToParticipantMap.at(convId).size(); ++i){
                    if (failure) holdMessageFailure(sent, rmid, conversationIDToParticipantMap.at(convId)[i]);
                    else holdMessage(sent, rmid, timestamp, conversationIDToParticipantMap.at(convId)[i], false, body);
                }
            } else {
                if (failure) holdMessageFailure(sent, rmid, author);
                else holdMessage(sent, rmid, timestamp, author, false, body);
            }

        }
        sqlv(result); //make sure we broke out with good results
        sqlv(sqlite3_finalize(stmt));
    }

    void SkypeManager::fetchFromTransfersTable(sqlite3 *db,
                                               const shared_ptr<vector<Address>> addresses,
                                               const std::set<string>& badAddressIDs,
                                               const std::unordered_map<long, vector<string>>& conversationIDToParticipantMap,
                                               const std::unordered_map<string, vector<long>>& participantToIDMap,
                                               bool catchUp = false ){
        sqlite3_stmt* stmt;
        int result;

        if (catchUp && lastTransferTime == 0) return; //No need to run specific catch-ups for new addresses on first fetch

        //File transfers (media-only messages, as far as Favor is concerned)
        string sql("SELECT " SKYPE_TRANSFERS_COLUMN_PKID "," SKYPE_TRANSFERS_COLUMN_AUTHOR ","  SKYPE_TRANSFERS_COLUMN_DATE "," SKYPE_TRANSFERS_COLUMN_CONVO ","
                SKYPE_TRANSFERS_COLUMN_STATUS " FROM " SKYPE_TRANSFERS_TABLE_NAME " ");
        sql += buildSelection((*addresses), badAddressIDs, participantToIDMap, SKYPE_TRANSFERS_COLUMN_CONVO, SKYPE_TRANSFERS_COLUMN_DATE, catchUp);
        sql += " AND " SKYPE_TRANSFERS_COLUMN_STATUS "=" SKYPE_TRANSFERS_STATUS_SUCCESS ";";
        DLOG(sql);
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
        bindSelection(stmt, (*addresses), badAddressIDs, participantToIDMap, lastTransferTime);

        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            long id = -1 * sqlite3_column_int64(stmt, 0); //Multiply by -1 to prevent collisions with normal Skype message IDs
            long timestamp = sqlite3_column_int64(stmt, 2);
            if (timestamp > lastTransferTime) lastTransferTime = timestamp;
            long convId = sqlite3_column_int64(stmt,3);

            string author = sqlite3_get_string(stmt, 1);
            bool sent = author == accountName;
            if (sent) {
                for (int i = 0; i < conversationIDToParticipantMap.at(convId).size(); ++i){
                    holdMessage(sent, id, timestamp, conversationIDToParticipantMap.at(convId)[i], true, "");
                }
            } else {
                holdMessage(sent, id, timestamp, author, true, "");
            }

        }
        sqlv(result); //make sure we broke out with good results
        sqlv(sqlite3_finalize(stmt));

    }


    void SkypeManager::fetchMessages() {


        shared_ptr<vector<Address>> addresses = contactAddresses();
        shared_ptr<vector<Address>> newAddresses  = std::make_shared<vector<Address>>();



        sqlite3 *db;
        sqlite3_stmt* stmt;
        int result;
        sqlv(sqlite3_open_v2(skypeDatabaseLocation.c_str(), &db, SQLITE_OPEN_READONLY, NULL));
        string sql("SELECT " SKYPE_PARTICIPANTS_COLUMN_ACCNAME "," SKYPE_PARTICIPANTS_COLUMN_CONVO " FROM " SKYPE_PARTICIPANTS_TABLE_NAME ";");
        sqlv(sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL));
        std::unordered_map<long, vector<string>> conversationIDToParticipantMap; //For determining who sent things
        std::unordered_map<string, vector<long>> participantToIDMap; //For building selections to only get messages we want


        while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
            //Build a map of convo id to name so we can use it for messages later. Every convo includes at least two participants:
            //the account owner, plus the person (people) they are talking to.
            string participant = sqlite3_get_string(stmt, 0);
            long participantID = sqlite3_column_int64(stmt, 1);
            if (participant != accountName){
                conversationIDToParticipantMap[participantID].push_back(participant);
                participantToIDMap[participant].push_back(participantID);
            }
        }
        sqlv(result);
        sqlv(sqlite3_finalize(stmt));

        std::set<string> badAddressIDs;
        if (addresses->size() != participantToIDMap.size()){
            for (int i = 0; i < addresses->size(); ++i){
                if (!participantToIDMap.count(addresses->at(i).addr)){
                    string addr = addresses->at(i).addr;
                    logger::warning("Ignoring skype address "+addr+" because a corresponding participant conversation ID cannot be found.");
                    badAddressIDs.insert(addr);
                }
            }
        }

        if (addresses->size() - badAddressIDs.size() == 0){
            logger::info("Account "+accountName+" fetchMessages returned because no usable addresses found");
            return;
        }

        for (auto it = addresses->begin(); it != addresses->end(); ++it){
            if (!badAddressIDs.count(it->addr) && !managedAddresses.count(it->addr)){
                logger::info("New Skype address "+it->addr+" detected");
                newAddresses->push_back(*it);
            }
        }
        if (newAddresses->size()){
            //If there are new addresses, we run catchup fetches before the new normal fetch
            fetchFromTransfersTable(db, newAddresses, badAddressIDs, conversationIDToParticipantMap, participantToIDMap, true);
            fetchFromMessagesTable(db, newAddresses, badAddressIDs, conversationIDToParticipantMap, participantToIDMap, true);

        }

        fetchFromMessagesTable(db, addresses, badAddressIDs, conversationIDToParticipantMap, participantToIDMap);

        fetchFromTransfersTable(db, addresses, badAddressIDs, conversationIDToParticipantMap, participantToIDMap);

        //Many of these inserts will be redundant, but it's just our way of updating the fetch data
        for (auto it = addresses->begin(); it != addresses->end(); ++it) managedAddresses.insert(it->addr);

        sqlv(sqlite3_close(db));


    }


}